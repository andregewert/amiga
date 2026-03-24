/*
 * Copyright (c) 2023-2024 André Gewert <agewert@ubergeek.de>
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

#ifndef APPSUPPORT_ENVIRONMENT_H
#define APPSUPPORT_ENVIRONMENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#define APPSUPPORT_DEFAULT_CHARSET "ISO-8859-1"

/**
 * Defines the color mode for shell output.
 */
typedef enum {
    /**
     * Output should not use different colors.
     */
    SHELL_NO_COLORS,

    /**
     * Color usage is not limited.
     */
    SHELL_ANSI_COLORS,

    /**
     * The application should use the four standard workbench
     * colors. These are typically grey, white, black and blue.
     */
    SHELL_FOUR_COLORS

} COLORMODE;

COLORMODE getEnvColorMode();

/**
 * For testing purposes, we can override the color mode.
 * @param mode The color mode to override with (-1 to disable).
 */
void setEnvColorModeOverride(int mode);

STRPTR getEnvCharset();
STRPTR getEnvLanguage();
STRPTR getEnvLanguageName();

#endif
