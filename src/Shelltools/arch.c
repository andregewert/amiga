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
#include <proto/dos.h>
#include "AppSupport/archive.h"
#include "AppSupport/collections.h"

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"ARCHIVE/A,COMMAND/A,FILES/M,TO/K";
    struct {
        char* archive_path;
        char* command;
        char** files;
        char* to;
    } args = {NULL, NULL, NULL, NULL};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), (STRPTR)"arch");
        return EXIT_FAILURE;
    }

    Archive* archive = archiveOpen(args.archive_path);
    if (!archive) {
        fprintf(stderr, "Could not open archive: %s\n", args.archive_path);
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }

    if (strcasecmp(args.command, "LIST") == 0) {
        linkedList* toc = archiveGetTOC(archive);
        if (toc) {
            listElement* curr = toc->firstElement;
            printf("Archive: %s\n", args.archive_path);
            printf("%-32s %10s %10s\n", "Name", "Size", "Offset");
            printf("------------------------------------------------------\n");
            while (curr) {
                ArchiveEntry* entry = (ArchiveEntry*)curr->data;
                printf("%-32s %10u %10u\n", entry->fileName, entry->size, entry->offset);
                curr = curr->nextElement;
            }
        }
    } else if (strcasecmp(args.command, "ADD") == 0) {
        if (args.files) {
            char** add_ptr = args.files;
            while (*add_ptr) {
                const char* sourcePath = *add_ptr;
                const char* entryName = strrchr(sourcePath, '/');
                if (!entryName) entryName = strrchr(sourcePath, ':');
                if (entryName) entryName++; else entryName = sourcePath;

                if (archiveAddFile(archive, sourcePath, entryName)) {
                    printf("Added: %s as %s\n", sourcePath, entryName);
                } else {
                    fprintf(stderr, "Failed to add: %s\n", sourcePath);
                }
                add_ptr++;
            }
        }
    } else if (strcasecmp(args.command, "REPLACE") == 0) {
        if (args.files) {
            char** replace_ptr = args.files;
            while (*replace_ptr) {
                const char* sourcePath = *replace_ptr;
                const char* entryName = strrchr(sourcePath, '/');
                if (!entryName) entryName = strrchr(sourcePath, ':');
                if (entryName) entryName++; else entryName = sourcePath;

                if (archiveReplaceFile(archive, sourcePath, entryName)) {
                    printf("Replaced: %s\n", entryName);
                } else {
                    fprintf(stderr, "Failed to replace: %s\n", entryName);
                }
                replace_ptr++;
            }
        }
    } else if (strcasecmp(args.command, "EXTRACT") == 0) {
        if (args.files) {
            char** extract_ptr = args.files;
            while (*extract_ptr) {
                const char* entryName = *extract_ptr;
                char destPath[256];
                if (args.to) {
                    snprintf(destPath, sizeof(destPath), "%s/%s", args.to, entryName);
                } else {
                    strncpy(destPath, entryName, sizeof(destPath));
                }

                if (archiveExtractFile(archive, entryName, destPath)) {
                    printf("Extracted: %s\n", entryName);
                } else {
                    fprintf(stderr, "Failed to extract: %s\n", entryName);
                }
                extract_ptr++;
            }
        }
    } else if (strcasecmp(args.command, "EXTRACTALL") == 0) {
        const char* destDir = args.to ? args.to : ".";
        if (archiveExtractAll(archive, destDir)) {
            printf("Extracted all to: %s\n", destDir);
        } else {
            fprintf(stderr, "Failed to extract all files\n");
        }
    } else if (strcasecmp(args.command, "EXISTS") == 0) {
        if (args.files && *args.files) {
            if (archiveFileExists(archive, *args.files)) {
                printf("File exists: %s\n", *args.files);
            } else {
                printf("File does NOT exist: %s\n", *args.files);
            }
        }
    } else if (strcasecmp(args.command, "DIR") == 0) {
        if (args.files) {
            char** dir_ptr = args.files;
            while (*dir_ptr) {
                const char* sourceDir = *dir_ptr;
                if (archiveAddDirectory(archive, sourceDir, NULL)) {
                    printf("Added directory: %s\n", sourceDir);
                } else {
                    fprintf(stderr, "Failed to add directory: %s\n", sourceDir);
                }
                dir_ptr++;
            }
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", args.command);
    }

    archiveClose(archive);
    FreeArgs(rdargs);
    return EXIT_SUCCESS;
}
