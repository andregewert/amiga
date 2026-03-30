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

dictionary* dictFromIni(const char* filename) {
    if (filename == NULL) return NULL;

    FILE* file = fopen(filename, "r");
    if (file == NULL) return NULL;

    dictionary* dict = dictCreate();
    if (dict == NULL) {
        fclose(file);
        return NULL;
    }

    char line[512];
    char currentSection[128] = "";
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline and spaces
        char* end = line + strlen(line) - 1;
        while (end >= line && (*end == '\n' || *end == '\r' || *end == ' ')) {
            *end-- = '\0';
        }

        // Skip leading spaces
        char* start = line;
        while (*start == ' ') start++;

        // Skip comments and empty lines
        if (*start == '\0' || *start == ';' || *start == '#') continue;

        // Check for section
        if (*start == '[' ) {
            char* sectionEnd = strchr(start, ']');
            if (sectionEnd != NULL) {
                *sectionEnd = '\0';
                strncpy(currentSection, start + 1, sizeof(currentSection) - 1);
                currentSection[sizeof(currentSection) - 1] = '\0';
            }
            continue;
        }

        // Check for key=value
        char* equals = strchr(start, '=');
        if (equals != NULL) {
            *equals = '\0';
            char* key = start;
            char* value = equals + 1;

            // Trim key
            char* keyEnd = key + strlen(key) - 1;
            while (keyEnd >= key && *keyEnd == ' ') *keyEnd-- = '\0';

            // Trim value
            while (*value == ' ') value++;
            char* valueEnd = value + strlen(value) - 1;
            while (valueEnd >= value && *valueEnd == ' ') *valueEnd-- = '\0';

            char fullKey[256];
            if (currentSection[0] != '\0') {
                snprintf(fullKey, sizeof(fullKey), "%s.%s", currentSection, key);
            } else {
                strncpy(fullKey, key, sizeof(fullKey) - 1);
                fullKey[sizeof(fullKey) - 1] = '\0';
            }

            // dictSet duplicates the key string, but uses the data pointer as-is.
            // Since we need to store the value string, we should duplicate it.
            dictSet(dict, (STRPTR)fullKey, (void*)strdup(value));
        }
    }

    fclose(file);
    return dict;
}

STRPTR basename(const char* path) {
    if (path == NULL) return NULL;
    if (path[0] == '\0') return (STRPTR)strdup("");

    size_t len = strlen(path);
    // Remove trailing slashes (but not if it's the only character)
    while (len > 1 && path[len - 1] == '/') {
        len--;
    }

    // Special case for "/"
    if (len == 1 && path[0] == '/') {
        return (STRPTR)strdup("/");
    }

    // Find the last separator (/ or :)
    const char* last_sep = NULL;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/' || path[i] == ':') {
            last_sep = &path[i];
        }
    }

    if (last_sep == NULL) {
        // No separator found, the whole path is the basename
        char* res = (char*)malloc(len + 1);
        if (res) {
            strncpy(res, path, len);
            res[len] = '\0';
        }
        return (STRPTR)res;
    }

    // If it's a volume separator, we might want different behavior
    // User wants basename("work:") = "work:"
    
    if (last_sep == &path[len - 1]) {
        if (*last_sep == ':') {
            // Volume separator at the end, return it as part of the basename
            char* res = (char*)malloc(len + 1);
            if (res) {
                strncpy(res, path, len);
                res[len] = '\0';
            }
            return (STRPTR)res;
        }

        // Separator '/' is at the end of the (possibly trimmed) string.
        const char* prev_sep = NULL;
        for (const char* p = path; p < last_sep; p++) {
            if (*p == '/' || *p == ':') prev_sep = p;
        }
        
        if (prev_sep == NULL) {
            size_t final_len = last_sep - path;
            char* res = (char*)malloc(final_len + 1);
            if (res) {
                strncpy(res, path, final_len);
                res[final_len] = '\0';
            }
            return (STRPTR)res;
        } else {
            size_t final_len = last_sep - (prev_sep + 1);
            char* res = (char*)malloc(final_len + 1);
            if (res) {
                strncpy(res, prev_sep + 1, final_len);
                res[final_len] = '\0';
            }
            return (STRPTR)res;
        }
    }

    size_t final_len = (path + len) - (last_sep + 1);
    char* res = (char*)malloc(final_len + 1);
    if (res) {
        strncpy(res, last_sep + 1, final_len);
        res[final_len] = '\0';
    }
    return (STRPTR)res;
}

STRPTR dirname(const char* path) {
    if (path == NULL) return NULL;
    if (path[0] == '\0') return (STRPTR)strdup("");

    size_t len = strlen(path);
    // If it's ".", Amiga dirname should be empty string
    if (len == 1 && path[0] == '.') {
        return (STRPTR)strdup("");
    }
    // Remove trailing slashes (but not if it's the only character)
    while (len > 1 && path[len - 1] == '/') {
        len--;
    }

    // Special case for "/"
    if (len == 1 && path[0] == '/') {
        return (STRPTR)strdup("/");
    }

    // Find the last separator
    const char* last_sep = NULL;
    int is_colon = 0;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/' || path[i] == ':') {
            last_sep = &path[i];
            is_colon = (path[i] == ':');
        }
    }

    if (last_sep == NULL) {
        return (STRPTR)strdup("");
    }

    if (last_sep == path && !is_colon) {
        // Path is like "/foo" -> dirname is "/"
        char* res = (char*)malloc(2);
        if (res) {
            res[0] = path[0];
            res[1] = '\0';
        }
        return (STRPTR)res;
    }

    size_t dir_len = last_sep - path;
    if (is_colon) {
        // Amiga specific: "work:foo" -> "work:"
        dir_len++; 
    }

    if (dir_len == 0 && !is_colon) {
        return (STRPTR)strdup("");
    }

    char* res = (char*)malloc(dir_len + 1);
    if (res) {
        strncpy(res, path, dir_len);
        res[dir_len] = '\0';
    }
    return (STRPTR)res;
}