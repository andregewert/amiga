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

//
// Created by agewert on 19.03.26.
//

#include "filetools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/dos.h>

STRPTR createTempFile(const char* directory, const char* prefix) {
    if (directory == NULL || prefix == NULL) {
        return NULL;
    }

    char* filename = (char*)malloc(256);
    if (filename == NULL) {
        return NULL;
    }

    // Use a simple counter for uniqueness if srand/rand/time is problematic
    static uint32_t counter = 0;
    if (counter == 0) {
        // We could try to get some entropy here if needed, but for now let's just use the counter.
        // Actually, let's use the address of a local variable as a weak seed.
        counter = (uint32_t)&counter;
    }

    for (int attempt = 0; attempt < 100; attempt++) {
        uint32_t val = counter++;
        
        // Amiga paths can use / or : as separators
        size_t dir_len = strlen(directory);
        char separator = '\0';
        if (dir_len > 0) {
            char last_char = directory[dir_len - 1];
            if (last_char != '/' && last_char != ':') {
                separator = '/';
            }
        }

        if (separator != '\0') {
            snprintf(filename, 256, "%s/%s%u", directory, prefix, val);
        } else {
            snprintf(filename, 256, "%s%s%u", directory, prefix, val);
        }

        // Try to open the file exclusively to ensure it's new
        BPTR file = Open((STRPTR)filename, MODE_NEWFILE);
        if (file != 0) {
            Close(file);
            return (STRPTR)filename;
        }
    }

    free(filename);
    return NULL;
}

STRPTR createTempDir(const char* parent_dir, const char* prefix) {
    if (parent_dir == NULL || prefix == NULL) {
        return NULL;
    }

    char* dirname = (char*)malloc(256);
    if (dirname == NULL) {
        return NULL;
    }

    // Use a simple counter for uniqueness
    static uint32_t counter = 0;
    if (counter == 0) {
        counter = (uint32_t)&counter;
    }

    for (int attempt = 0; attempt < 100; attempt++) {
        uint32_t val = counter++;

        // Amiga paths can use / or : as separators
        size_t dir_len = strlen(parent_dir);
        char separator = '\0';
        if (dir_len > 0) {
            char last_char = parent_dir[dir_len - 1];
            if (last_char != '/' && last_char != ':') {
                separator = '/';
            }
        }

        if (separator != '\0') {
            snprintf(dirname, 256, "%s/%s%u", parent_dir, prefix, val);
        } else {
            snprintf(dirname, 256, "%s%s%u", parent_dir, prefix, val);
        }

        // Try to create the directory
        BPTR lock = CreateDir((STRPTR)dirname);
        if (lock != 0) {
            UnLock(lock);
            return (STRPTR)dirname;
        }
    }

    free(dirname);
    return NULL;
}