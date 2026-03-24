/*
 * Copyright (c) 2024 André Gewert <agewert@ubergeek.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "environment.h"
#include <string.h>

/**
 * For testing purposes, we can override the color mode.
 */
static int colorModeOverride = -1;

void setEnvColorModeOverride(int mode) {
    colorModeOverride = mode;
}

COLORMODE getEnvColorMode() {
    if (colorModeOverride != -1) {
        return (COLORMODE)colorModeOverride;
    }

    char dummy[5];
    if (GetVar("shellcolors", (STRPTR)dummy, 5, 0) > 0) {
        if (strcmp(dummy, "4") == 0) {
            return SHELL_FOUR_COLORS;
        }
        if (strcmp(dummy, "amiga") == 0) {
            return SHELL_FOUR_COLORS;
        }
        if (strcmp(dummy, "ansi") == 0) {
            return SHELL_ANSI_COLORS;
        }
        if (strcmp(dummy, "full") == 0) {
            return SHELL_ANSI_COLORS;
        }
    }
    return SHELL_NO_COLORS;
}

STRPTR getEnvCharset() {
    STRPTR buffer = calloc(20, sizeof(char));
    if (buffer != NULL) {
        int ret = GetVar("charset", buffer, 20, 0);
        if (ret <= 0) {
            strcpy(buffer, APPSUPPORT_DEFAULT_CHARSET);
        }
    }
    return buffer;
}

STRPTR getEnvLanguage() {
    STRPTR buffer = calloc(20, sizeof(char));
    if (buffer != NULL) {
        GetVar("language", buffer, 20, 0);
    }
    return buffer;
}

STRPTR getEnvLanguageName() {
    STRPTR buffer = calloc(20, sizeof(char));
    if (buffer != NULL) {
        GetVar("languagename", buffer, 20, 0);
    }
    return buffer;
}
