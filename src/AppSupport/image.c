// Copyright (c) 2026 André Gewert <agewert@ubergeek.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "image.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/guigfx_lib.h>
#include <guigfx/guigfx.h>
#include <render/render.h>

extern struct Library *GuiGFXBase;

// ASCII characters sorted by density (light to dark)
static const char* ASCII_CHARS = " .:-=+*#%@";
#define NUM_ASCII_CHARS 10

/**
 * Calculates the brightness of an RGB color.
 * Result is in the range [0, 255].
 */
static inline uint32_t getBrightness(uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t)(0.299f * r + 0.587f * g + 0.114f * b);
}

/**
 * Maps a brightness value [0, 255] to an ASCII character.
 */
static char mapBrightnessToAscii(uint32_t brightness) {
    uint32_t asciiIdx = (brightness * (NUM_ASCII_CHARS - 1)) / 255;
    return ASCII_CHARS[asciiIdx];
}

/**
 * Maps an RGB color to the closest shell color.
 */
static const char* mapRgbToShellColor(uint8_t r, uint8_t g, uint8_t b) {
    // Very simple mapping for demonstration. 
    // Could be improved with a proper distance calculation in color space.
    if (r < 64 && g < 64 && b < 64) return SHELL_FG_BLACK;
    if (r > 192 && g > 192 && b > 192) return SHELL_FG_WHITE;
    
    if (r > g && r > b) return (r > 192) ? SHELL_FG_BRIGHT_RED : SHELL_FG_RED;
    if (g > r && g > b) return (g > 192) ? SHELL_FG_BRIGHT_GREEN : SHELL_FG_GREEN;
    if (b > r && b > g) return (b > 192) ? SHELL_FG_BRIGHT_BLUE : SHELL_FG_BLUE;
    
    if (r > 128 && g > 128) return (r > 192) ? SHELL_FG_BRIGHT_YELLOW : SHELL_FG_YELLOW;
    if (r > 128 && b > 128) return (r > 192) ? SHELL_FG_BRIGHT_MAGENTA : SHELL_FG_MAGENTA;
    if (g > 128 && b > 128) return (g > 192) ? SHELL_FG_BRIGHT_CYAN : SHELL_FG_CYAN;

    return SHELL_FG_WHITE;
}

static void renderImage(uint32_t* rawData, uint32_t width, uint32_t height, uint32_t targetWidth) {
    if (targetWidth == 0) targetWidth = 80;
    if (targetWidth > width) targetWidth = width;

    // Calculate scaling factors
    float scaleX = (float)width / (float)targetWidth;
    float scaleY = scaleX * 2.0f; // ASCII chars are taller than wide
    uint32_t targetHeight = (uint32_t)((float)height / scaleY);

    for (uint32_t y = 0; y < targetHeight; y++) {
        for (uint32_t x = 0; x < targetWidth; x++) {
            // Sample the image (nearest neighbor)
            uint32_t imgX = (uint32_t)(x * scaleX);
            uint32_t imgY = (uint32_t)(y * scaleY);
            
            if (imgX >= width) imgX = width - 1;
            if (imgY >= height) imgY = height - 1;

            uint32_t pixel = rawData[imgY * width + imgX];
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            uint32_t brightness = getBrightness(r, g, b);
            char asciiChar = mapBrightnessToAscii(brightness);
            const char* shellColor = mapRgbToShellColor(r, g, b);

            shellPrintf(shellColor, "%c", asciiChar);
        }
        printf("\n");
    }
}

BOOL imagePrintAscii(const char* filename, uint32_t targetWidth) {
    struct Library* base = OpenLibrary("guigfx.library", 0);
    if (!base) {
        fprintf(stderr, "Could not open guigfx.library\n");
        return FALSE;
    }
    GuiGFXBase = base;

    APTR picture = LoadPicture((STRPTR)filename, TAG_DONE);
    if (!picture) {
        fprintf(stderr, "Could not load image: %s\n", filename);
        CloseLibrary(GuiGFXBase);
        return FALSE;
    }

    uint32_t width = 0, height = 0, pixelFormat = 0;
    uint32_t* rawData = NULL;

    GetPictureAttrs(picture, 
        PICATTR_Width, &width,
        PICATTR_Height, &height,
        PICATTR_RawData, &rawData,
        PICATTR_PixelFormat, &pixelFormat,
        TAG_DONE);

    BOOL success = FALSE;

    if (rawData) {
        if (pixelFormat == PIXFMT_0RGB_32) {
            renderImage(rawData, width, height, targetWidth);
            success = TRUE;
        } else {
            fprintf(stderr, "Unsupported pixel format: %u\n", pixelFormat);
        }
    } else {
        fprintf(stderr, "Could not get raw data from picture\n");
    }

    DeletePicture(picture);
    CloseLibrary(GuiGFXBase);
    return success;
}
