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
#include <stdbool.h>
#include <proto/dos.h>
#include "../AppSupport/filetools.h"

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"DIRECTORY,PREFIX,D/S";
    struct {
        char* directory;
        char* prefix;
        long directory_flag;
    } args = {NULL, NULL, 0};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);

    const char* directory = "";
    const char* prefix = "tmp_";
    bool create_directory = false;

    if (rdargs) {
        if (args.directory) {
            directory = args.directory;
        }
        if (args.prefix) {
            prefix = args.prefix;
        }
        if (args.directory_flag) {
            create_directory = true;
        }
    } else {
        PrintFault(IoErr(), (STRPTR)"mktemp");
        return EXIT_FAILURE;
    }

    STRPTR tempPath = NULL;
    if (create_directory) {
        tempPath = createTempDir(directory, prefix);
    } else {
        tempPath = createTempFile(directory, prefix);
    }

    if (tempPath != NULL) {
        printf("%s\n", tempPath);
        free(tempPath);
        FreeArgs(rdargs);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Failed to create temporary %s in %s with prefix %s\n", 
                create_directory ? "directory" : "file", directory, prefix);
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }
}
