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
    const char* archiveName = "T:test2.arch";
    const char* rec_dir = "T:recursive";
    const char* rec_subdir = "T:recursive/subdir";
    const char* fileA = "T:recursive/fileA.txt";
    const char* fileB = "T:recursive/subdir/fileB.txt";
    const char* contentA = "Content A";
    const char* contentB = "Content B";

    BPTR d1 = CreateDir((STRPTR)rec_dir);
    if (d1) UnLock(d1);
    BPTR d2 = CreateDir((STRPTR)rec_subdir);
    if (d2) UnLock(d2);

    createDummyFile(fileA, contentA);
    createDummyFile(fileB, contentB);

    Archive* arch = archiveOpen(archiveName);
    ASSERT_NOT_NULL(arch);
    ASSERT_TRUE(archiveAddDirectory(arch, rec_dir, "rec"));
    archiveClose(arch);

    arch = archiveOpen(archiveName);
    ASSERT_NOT_NULL(arch);

    linkedList* toc = archiveGetTOC(arch);
    ASSERT_NOT_NULL(toc);

    printf("Table of Contents:\n");
    for (uint32_t i = 0; i < 100; i++) {
        listElement* el = listGetElementAt(toc, i);
        if (!el) break;
        ArchiveEntry* entry = (ArchiveEntry*)el->data;
        printf("- %s (Size: %u, Offset: %u)\n", entry->fileName, entry->size, entry->offset);
    }
    ASSERT_TRUE(archiveFileExists(arch, "rec/fileA.txt"));
    ASSERT_TRUE(archiveFileExists(arch, "rec/subdir/fileB.txt"));

    uint32_t sA, sB;
    char* dA = (char*)archiveReadFile(arch, "rec/fileA.txt", &sA);
    char* dB = (char*)archiveReadFile(arch, "rec/subdir/fileB.txt", &sB);

    ASSERT_NOT_NULL(dA);
    if (dA) {
        ASSERT_STR_EQ(contentA, dA);
        free(dA);
    }
    ASSERT_NOT_NULL(dB);
    if (dB) {
        ASSERT_STR_EQ(contentB, dB);
        free(dB);
    }

    archiveClose(arch);

    return testSummary();
}
