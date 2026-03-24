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

#include "shell.h"
#include <stdarg.h>
#include <string.h>

/**
 * Maps a given ANSI color code to a simplified set for SHELL_FOUR_COLORS mode.
 * Standard Workbench colors: Grey (Default/Reset), White, Black, Blue.
 * @param color The original ANSI color string.
 * @return The mapped ANSI color string.
 */
static const char* mapFourColors(const char* color) {
    if (color == NULL || strlen(color) == 0) return SHELL_COLOR_RESET;

    // Default/Reset mapping
    if (strcmp(color, SHELL_COLOR_RESET) == 0) return SHELL_COLOR_RESET;

    // Black mapping
    if (strcmp(color, SHELL_FG_BLACK) == 0 || strcmp(color, SHELL_FG_BRIGHT_BLACK) == 0 ||
        strcmp(color, SHELL_BG_BLACK) == 0 || strcmp(color, SHELL_BG_BRIGHT_BLACK) == 0) {
        return (color[2] == '3' || color[2] == '9') ? SHELL_FG_BLACK : SHELL_BG_BLACK;
    }

    // White mapping
    if (strcmp(color, SHELL_FG_WHITE) == 0 || strcmp(color, SHELL_FG_BRIGHT_WHITE) == 0 ||
        strcmp(color, SHELL_BG_WHITE) == 0 || strcmp(color, SHELL_BG_BRIGHT_WHITE) == 0 ||
        strcmp(color, SHELL_FG_YELLOW) == 0 || strcmp(color, SHELL_FG_BRIGHT_YELLOW) == 0 ||
        strcmp(color, SHELL_BG_YELLOW) == 0 || strcmp(color, SHELL_BG_BRIGHT_YELLOW) == 0 ||
        strcmp(color, SHELL_FG_CYAN) == 0 || strcmp(color, SHELL_FG_BRIGHT_CYAN) == 0 ||
        strcmp(color, SHELL_BG_CYAN) == 0 || strcmp(color, SHELL_BG_BRIGHT_CYAN) == 0) {
        return (color[2] == '3' || color[2] == '9') ? SHELL_FG_WHITE : SHELL_BG_WHITE;
    }

    // Blue mapping
    if (strcmp(color, SHELL_FG_BLUE) == 0 || strcmp(color, SHELL_FG_BRIGHT_BLUE) == 0 ||
        strcmp(color, SHELL_BG_BLUE) == 0 || strcmp(color, SHELL_BG_BRIGHT_BLUE) == 0 ||
        strcmp(color, SHELL_FG_RED) == 0 || strcmp(color, SHELL_FG_BRIGHT_RED) == 0 ||
        strcmp(color, SHELL_BG_RED) == 0 || strcmp(color, SHELL_BG_BRIGHT_RED) == 0 ||
        strcmp(color, SHELL_FG_MAGENTA) == 0 || strcmp(color, SHELL_FG_BRIGHT_MAGENTA) == 0 ||
        strcmp(color, SHELL_BG_MAGENTA) == 0 || strcmp(color, SHELL_BG_BRIGHT_MAGENTA) == 0 ||
        strcmp(color, SHELL_FG_GREEN) == 0 || strcmp(color, SHELL_FG_BRIGHT_GREEN) == 0 ||
        strcmp(color, SHELL_BG_GREEN) == 0 || strcmp(color, SHELL_BG_BRIGHT_GREEN) == 0) {
        return (color[2] == '3' || color[2] == '9') ? SHELL_FG_BLUE : SHELL_BG_BLUE;
    }

    return color; // Fallback
}

void shellPrintf(const char* color, const char* format, ...) {
    COLORMODE mode = getEnvColorMode();
    if (mode != SHELL_NO_COLORS) {
        if (mode == SHELL_FOUR_COLORS) {
            printf("%s", mapFourColors(color));
        } else {
            printf("%s", color);
        }
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (mode != SHELL_NO_COLORS) {
        printf("%s", SHELL_COLOR_RESET);
    }
}

void shellSetColor(const char* color) {
    COLORMODE mode = getEnvColorMode();
    if (mode != SHELL_NO_COLORS) {
        if (mode == SHELL_FOUR_COLORS) {
            printf("%s", mapFourColors(color));
        } else {
            printf("%s", color);
        }
    }
}

void shellResetColor() {
    if (getEnvColorMode() != SHELL_NO_COLORS) {
        printf("%s", SHELL_COLOR_RESET);
    }
}
