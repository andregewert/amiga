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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <proto/dos.h>
#include "AppSupport/shell.h"

static const char* getColor(const char* name) {
    if (!name) return SHELL_COLOR_RESET;
    if (strcasecmp(name, "RESET") == 0) return SHELL_COLOR_RESET;
    if (strcasecmp(name, "BOLD") == 0) return SHELL_COLOR_BOLD;
    
    if (strcasecmp(name, "BLACK") == 0) return SHELL_FG_BLACK;
    if (strcasecmp(name, "RED") == 0) return SHELL_FG_RED;
    if (strcasecmp(name, "GREEN") == 0) return SHELL_FG_GREEN;
    if (strcasecmp(name, "YELLOW") == 0) return SHELL_FG_YELLOW;
    if (strcasecmp(name, "BLUE") == 0) return SHELL_FG_BLUE;
    if (strcasecmp(name, "MAGENTA") == 0) return SHELL_FG_MAGENTA;
    if (strcasecmp(name, "CYAN") == 0) return SHELL_FG_CYAN;
    if (strcasecmp(name, "WHITE") == 0) return SHELL_FG_WHITE;

    if (strcasecmp(name, "BRIGHT_BLACK") == 0) return SHELL_FG_BRIGHT_BLACK;
    if (strcasecmp(name, "BRIGHT_RED") == 0) return SHELL_FG_BRIGHT_RED;
    if (strcasecmp(name, "BRIGHT_GREEN") == 0) return SHELL_FG_BRIGHT_GREEN;
    if (strcasecmp(name, "BRIGHT_YELLOW") == 0) return SHELL_FG_BRIGHT_YELLOW;
    if (strcasecmp(name, "BRIGHT_BLUE") == 0) return SHELL_FG_BRIGHT_BLUE;
    if (strcasecmp(name, "BRIGHT_MAGENTA") == 0) return SHELL_FG_BRIGHT_MAGENTA;
    if (strcasecmp(name, "BRIGHT_CYAN") == 0) return SHELL_FG_BRIGHT_CYAN;
    if (strcasecmp(name, "BRIGHT_WHITE") == 0) return SHELL_FG_BRIGHT_WHITE;

    if (strcasecmp(name, "BG_BLACK") == 0) return SHELL_BG_BLACK;
    if (strcasecmp(name, "BG_RED") == 0) return SHELL_BG_RED;
    if (strcasecmp(name, "BG_GREEN") == 0) return SHELL_BG_GREEN;
    if (strcasecmp(name, "BG_YELLOW") == 0) return SHELL_BG_YELLOW;
    if (strcasecmp(name, "BG_BLUE") == 0) return SHELL_BG_BLUE;
    if (strcasecmp(name, "BG_MAGENTA") == 0) return SHELL_BG_MAGENTA;
    if (strcasecmp(name, "BG_CYAN") == 0) return SHELL_BG_CYAN;
    if (strcasecmp(name, "BG_WHITE") == 0) return SHELL_BG_WHITE;

    return SHELL_COLOR_RESET;
}

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"COLOR/A";
    struct {
        char* color;
    } args = {NULL};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), (STRPTR)"shsetcolor");
        return EXIT_FAILURE;
    }

    shellSetColor(getColor(args.color));

    FreeArgs(rdargs);
    return EXIT_SUCCESS;
}
