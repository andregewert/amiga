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

#ifndef APPSUPPORT_SHELL_H
#define APPSUPPORT_SHELL_H

#include <stdio.h>
#include "environment.h"

/**
 * ANSI Escape sequences for shell colors.
 */
#define SHELL_COLOR_RESET   "\033[0m"
#define SHELL_COLOR_BOLD    "\033[1m"

// Foreground colors (Standard)
#define SHELL_FG_BLACK      "\033[30m"
#define SHELL_FG_RED        "\033[31m"
#define SHELL_FG_GREEN      "\033[32m"
#define SHELL_FG_YELLOW     "\033[33m"
#define SHELL_FG_BLUE       "\033[34m"
#define SHELL_FG_MAGENTA    "\033[35m"
#define SHELL_FG_CYAN       "\033[36m"
#define SHELL_FG_WHITE      "\033[37m"

// Foreground colors (Bright)
#define SHELL_FG_BRIGHT_BLACK   "\033[90m"
#define SHELL_FG_BRIGHT_RED     "\033[91m"
#define SHELL_FG_BRIGHT_GREEN   "\033[92m"
#define SHELL_FG_BRIGHT_YELLOW  "\033[93m"
#define SHELL_FG_BRIGHT_BLUE    "\033[94m"
#define SHELL_FG_BRIGHT_MAGENTA "\033[95m"
#define SHELL_FG_BRIGHT_CYAN    "\033[96m"
#define SHELL_FG_BRIGHT_WHITE   "\033[97m"

// Background colors (Standard)
#define SHELL_BG_BLACK      "\033[40m"
#define SHELL_BG_RED        "\033[41m"
#define SHELL_BG_GREEN      "\033[42m"
#define SHELL_BG_YELLOW     "\033[43m"
#define SHELL_BG_BLUE       "\033[44m"
#define SHELL_BG_MAGENTA    "\033[45m"
#define SHELL_BG_CYAN       "\033[46m"
#define SHELL_BG_WHITE      "\033[47m"

// Background colors (Bright)
#define SHELL_BG_BRIGHT_BLACK   "\033[100m"
#define SHELL_BG_BRIGHT_RED     "\033[101m"
#define SHELL_BG_BRIGHT_GREEN   "\033[102m"
#define SHELL_BG_BRIGHT_YELLOW  "\033[103m"
#define SHELL_BG_BRIGHT_BLUE    "\033[104m"
#define SHELL_BG_BRIGHT_MAGENTA "\033[105m"
#define SHELL_BG_BRIGHT_CYAN    "\033[106m"
#define SHELL_BG_BRIGHT_WHITE   "\033[107m"

/**
 * Prints a colored string to stdout if the environment supports it.
 * @param color ANSI color escape sequence.
 * @param format Printf-style format string.
 * @param ... Arguments for the format string.
 */
void shellPrintf(const char* color, const char* format, ...);

/**
 * Sets the current shell color.
 * @param color ANSI color escape sequence.
 */
void shellSetColor(const char* color);

/**
 * Resets the shell color to default.
 */
void shellResetColor();

#endif // APPSUPPORT_SHELL_H
