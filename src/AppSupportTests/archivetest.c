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
#include "AppSupport/testutils.h"

void createDummyFile(const char* filename, const char* content) {
    BPTR file = Open((STRPTR)filename, MODE_NEWFILE);
    if (file) {
        Write(file, (void*)content, strlen(content));
        Close(file);
    }
}

int main() {
    testInit("Archive Tests");
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
    ASSERT_NOT_NULL(arch);
    if (!arch) {
        return testSummary();
    }

    printf("Adding %s to archive...\n", file1);
    ASSERT_TRUE(archiveAddFile(arch, file1, "file1.txt"));

    printf("Adding %s to archive...\n", file2);
    ASSERT_TRUE(archiveAddFile(arch, file2, "file2.txt"));

    archiveClose(arch);
    printf("Archive closed.\n");

    printf("Re-opening archive for verification...\n");
    arch = archiveOpen(archiveName);
    ASSERT_NOT_NULL(arch);
    if (!arch) {
        return testSummary();
    }

    linkedList* toc = archiveGetTOC(arch);
    ASSERT_NOT_NULL(toc);
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
    ASSERT_NOT_NULL(data1);
    if (data1) {
        printf("Content: %.*s\n", (int)size1, data1);
        ASSERT_INT_EQ(strlen(file1_content), size1);
        ASSERT_STR_EQ(file1_content, data1);
        free(data1);
    }

    printf("Extracting file2.txt to T:extracted_file2.txt...\n");
    ASSERT_TRUE(archiveExtractFile(arch, "file2.txt", "T:extracted_file2.txt"));

    printf("Testing replacement: updating file1.txt in archive...\n");
    const char* file1_new_content = "This is the NEW content for file 1, it is much longer than before.";
    createDummyFile(file1, file1_new_content);
    ASSERT_TRUE(archiveAddFile(arch, file1, "file1.txt"));

    linkedList* new_toc = archiveGetTOC(arch);
    uint32_t count = 0;
    for (uint32_t i = 0; i < 100; i++) {
        listElement* el = listGetElementAt(new_toc, i);
        if (!el) break;
        count++;
    }
    ASSERT_INT_EQ(2, count);

    printf("Reading NEW file1.txt from archive...\n");
    uint32_t new_size1;
    char* new_data1 = (char*)archiveReadFile(arch, "file1.txt", &new_size1);
    ASSERT_NOT_NULL(new_data1);
    if (new_data1) {
        ASSERT_INT_EQ(strlen(file1_new_content), new_size1);
        ASSERT_STR_EQ(file1_new_content, new_data1);
        free(new_data1);
    }

    archiveClose(arch);
    return testSummary();
}
