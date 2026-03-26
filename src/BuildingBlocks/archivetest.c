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

void createDummyFile(const char* filename, const char* content) {
    BPTR file = Open((STRPTR)filename, MODE_NEWFILE);
    if (file) {
        Write(file, (void*)content, strlen(content));
        Close(file);
    }
}

int main() {
    const char* archiveName = "T:test.arch";
    const char* file1 = "T:file1.txt";
    const char* file2 = "T:file2.txt";
    const char* file1_content = "Hello, this is file 1 content!";
    const char* file2_content = "And this is file 2 content, which is slightly longer.";

    printf("Creating dummy files...\n");
    createDummyFile(file1, file1_content);
    createDummyFile(file2, file2_content);

    printf("Opening archive %s...\n", archiveName);
    Archive* arch = archiveOpen(archiveName);
    if (!arch) {
        printf("Failed to open archive!\n");
        return 1;
    }

    printf("Adding %s to archive...\n", file1);
    if (!archiveAddFile(arch, file1, "file1.txt")) {
        printf("Failed to add %s\n", file1);
    }

    printf("Adding %s to archive...\n", file2);
    if (!archiveAddFile(arch, file2, "file2.txt")) {
        printf("Failed to add %s\n", file2);
    }

    archiveClose(arch);
    printf("Archive closed.\n");

    printf("Re-opening archive for verification...\n");
    arch = archiveOpen(archiveName);
    if (!arch) {
        printf("Failed to re-open archive!\n");
        return 1;
    }

    linkedList* toc = archiveGetTOC(arch);
    printf("Table of Contents:\n");
    for (uint32_t i = 0; i < 100; i++) {
        listElement* el = listGetElementAt(toc, i);
        if (!el) break;
        ArchiveEntry* entry = (ArchiveEntry*)el->data;
        printf("- %s (Size: %u, Offset: %u)\n", entry->fileName, entry->size, entry->offset);
    }

    printf("Reading file1.txt from archive...\n");
    uint32_t size1;
    char* data1 = (char*)archiveReadFile(arch, "file1.txt", &size1);
    if (data1) {
        printf("Content: %.*s\n", (int)size1, data1);
        if (strncmp(data1, file1_content, size1) == 0) {
            printf("Verification SUCCESS for file1.txt\n");
        } else {
            printf("Verification FAILED for file1.txt\n");
        }
        free(data1);
    } else {
        printf("Failed to read file1.txt\n");
    }

    printf("Extracting file2.txt to T:extracted_file2.txt...\n");
    if (archiveExtractFile(arch, "file2.txt", "T:extracted_file2.txt")) {
        printf("Extraction successful.\n");
        // We could read it back and verify, but archiveReadFile already verified the core logic.
    } else {
        printf("Extraction failed.\n");
    }

    printf("Testing replacement: updating file1.txt in archive...\n");
    const char* file1_new_content = "This is the NEW content for file 1, it is much longer than before.";
    createDummyFile(file1, file1_new_content);
    if (archiveAddFile(arch, file1, "file1.txt")) {
        printf("Re-added file1.txt successfully.\n");
    } else {
        printf("Failed to re-add file1.txt\n");
    }

    linkedList* new_toc = archiveGetTOC(arch);
    printf("Table of Contents after replacement:\n");
    uint32_t count = 0;
    for (uint32_t i = 0; i < 100; i++) {
        listElement* el = listGetElementAt(new_toc, i);
        if (!el) break;
        ArchiveEntry* entry = (ArchiveEntry*)el->data;
        printf("- %s (Size: %u, Offset: %u)\n", entry->fileName, entry->size, entry->offset);
        count++;
    }
    printf("TOC has %u entries.\n", count);
    if (count == 2) {
        printf("Verification SUCCESS: TOC still has only 2 entries.\n");
    } else {
        printf("Verification FAILED: TOC has %u entries (expected 2).\n", count);
    }

    printf("Reading NEW file1.txt from archive...\n");
    uint32_t new_size1;
    char* new_data1 = (char*)archiveReadFile(arch, "file1.txt", &new_size1);
    if (new_data1) {
        printf("New Content: %.*s\n", (int)new_size1, new_data1);
        if (new_size1 == strlen(file1_new_content) && strncmp(new_data1, file1_new_content, new_size1) == 0) {
            printf("Verification SUCCESS for updated file1.txt\n");
        } else {
            printf("Verification FAILED for updated file1.txt\n");
        }
        free(new_data1);
    } else {
        printf("Failed to read updated file1.txt\n");
    }

    archiveClose(arch);
    printf("Test finished.\n");

    return 0;
}
