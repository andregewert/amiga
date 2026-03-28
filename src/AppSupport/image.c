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
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <datatypes/pictureclass.h>
#include <clib/alib_protos.h>

// ASCII characters sorted by density (light to dark)
static const char* ASCII_CHARS = " .:-=+*#%@";
#define NUM_ASCII_CHARS 10

/**
 * Calculates the brightness of an RGB color.
 * Result is in the range [0, 255].
 */
uint32_t getBrightness(uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t)(0.299f * r + 0.587f * g + 0.114f * b);
}

/**
 * Maps a brightness value [0, 255] to an ASCII character.
 */
char mapBrightnessToAscii(uint32_t brightness) {
    uint32_t asciiIdx = (brightness * (NUM_ASCII_CHARS - 1)) / 255;
    return ASCII_CHARS[asciiIdx];
}

/**
 * Maps an RGB color to the closest shell color.
 */
const char* mapRgbToShellColor(uint8_t r, uint8_t g, uint8_t b) {
    // Very simple mapping for demonstration. 
    // Could be improved with a proper distance calculation in color space.
    if (r < 64 && g < 64 && b < 64) return SHELL_FG_BLACK;
    if (r > 192 && g > 192 && b > 192) return SHELL_FG_WHITE;
    
    if (r > g && r > b) return (r > 192) ? SHELL_FG_BRIGHT_RED : SHELL_FG_RED;
    if (g > r && g > b) return (g > 192) ? SHELL_FG_BRIGHT_GREEN : SHELL_FG_GREEN;
    if (b > r && b > g) return (b > 192) ? SHELL_FG_BRIGHT_BLUE : SHELL_FG_BLUE;
    
    if (r > 128 && g > 128) return (r > 192) ? SHELL_FG_BRIGHT_YELLOW : SHELL_FG_YELLOW;
    if (r > 128 && b > 128) return (r > 192) ? SHELL_FG_BRIGHT_MAGENTA : SHELL_FG_MAGENTA;
    if (g > 128 && b > 128) return (r > 192) ? SHELL_FG_BRIGHT_CYAN : SHELL_FG_CYAN;

    return SHELL_FG_WHITE;
}

BOOL imagePrintAscii(const char* filename, uint32_t targetWidth) {
    struct Library* DataTypesBase = OpenLibrary("datatypes.library", 39);
    if (!DataTypesBase) {
        fprintf(stderr, "Could not open datatypes.library\n");
        return FALSE;
    }

    struct Library* GraphicsBase = OpenLibrary("graphics.library", 0);
    if (!GraphicsBase) {
        fprintf(stderr, "Could not open graphics.library\n");
        CloseLibrary(DataTypesBase);
        return FALSE;
    }

    Object* dto = NewDTObject((STRPTR)filename,
        DTA_SourceType, DTST_FILE,
        DTA_GroupID, GID_PICTURE,
        PDTA_Remap, FALSE, // We want raw colors if possible
        TAG_DONE);

    if (!dto) {
        fprintf(stderr, "Could not load image via datatypes: %s\n", filename);
        CloseLibrary(GraphicsBase);
        CloseLibrary(DataTypesBase);
        return FALSE;
    }

    // Perform layout
    DoMethod(dto, DTM_PROCLAYOUT, NULL, 1);

    struct BitMapHeader* bmh = NULL;
    struct BitMap* bm = NULL;

    GetDTAttrs(dto,
        PDTA_BitMapHeader, (uintptr_t)&bmh,
        PDTA_BitMap, (uintptr_t)&bm,
        TAG_DONE);

    if (!bmh || !bm) {
        fprintf(stderr, "Could not get bitmap information from datatype object\n");
        DisposeDTObject(dto);
        CloseLibrary(GraphicsBase);
        CloseLibrary(DataTypesBase);
        return FALSE;
    }

    uint32_t width = bmh->bmh_Width;
    uint32_t height = bmh->bmh_Height;

    if (targetWidth == 0) targetWidth = 80;
    if (targetWidth > width) targetWidth = width;

    // Calculate scaling factors
    float scaleX = (float)width / (float)targetWidth;
    float scaleY = scaleX * 2.0f; // ASCII chars are taller than wide
    uint32_t targetHeight = (uint32_t)((float)height / scaleY);

    // We no longer use ReadPixelArray from cybergraphics, 
    // but use PDTM_READPIXELARRAY from pictureclass.
    // This is more standard for datatypes and doesn't require CyberGraphX.

    for (uint32_t y = 0; y < targetHeight; y++) {
        uint32_t imgY = (uint32_t)(y * scaleY);
        if (imgY >= height) imgY = height - 1;

        for (uint32_t x = 0; x < targetWidth; x++) {
            uint32_t imgX = (uint32_t)(x * scaleX);
            if (imgX >= width) imgX = width - 1;

            uint8_t rgb[3];
            uint8_t r, g, b;
            struct pdtBlitPixelArray pbpa;
            pbpa.MethodID = PDTM_READPIXELARRAY;
            pbpa.pbpa_PixelData = rgb;
            pbpa.pbpa_PixelFormat = PBPAFMT_RGB;
            pbpa.pbpa_PixelArrayMod = 3;
            pbpa.pbpa_Left = imgX;
            pbpa.pbpa_Top = imgY;
            pbpa.pbpa_Width = 1;
            pbpa.pbpa_Height = 1;

            if (DoMethodA(dto, (Msg)&pbpa) == 0) {
                r = g = b = 0; 
            } else {
                r = rgb[0];
                g = rgb[1];
                b = rgb[2];
            }

            uint32_t brightness = getBrightness(r, g, b);
            char asciiChar = mapBrightnessToAscii(brightness);
            const char* shellColor = mapRgbToShellColor(r, g, b);

            shellPrintf(shellColor, "%c", asciiChar);
        }
        printf("\n");
    }

    DisposeDTObject(dto);
    CloseLibrary(GraphicsBase);
    CloseLibrary(DataTypesBase);
    shellResetColor();

    return TRUE;
}
