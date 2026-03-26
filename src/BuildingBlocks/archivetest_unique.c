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
    const char* archiveName = "T:test_unique.arch";
    const char* file1 = "T:file1.txt";
    const char* content1 = "First version";
    const char* content2 = "Second version (longer)";

    printf("Cleanup old archive...\n");
    DeleteFile((STRPTR)archiveName);

    printf("Step 1: Adding first version...\n");
    createDummyFile(file1, content1);
    Archive* arch = archiveOpen(archiveName);
    if (!arch) { printf("Failed to open archive\n"); return 1; }
    archiveAddFile(arch, file1, "unique.txt");
    archiveClose(arch);

    printf("Step 2: Adding second version (same name)...\n");
    createDummyFile(file1, content2);
    arch = archiveOpen(archiveName);
    if (!arch) { printf("Failed to open archive\n"); return 1; }
    archiveAddFile(arch, file1, "unique.txt");
    archiveClose(arch);

    printf("Step 3: Verifying TOC...\n");
    arch = archiveOpen(archiveName);
    if (!arch) { printf("Failed to open archive\n"); return 1; }
    linkedList* toc = archiveGetTOC(arch);
    listElement* current = toc->firstElement;
    int count = 0;
    while(current) {
        ArchiveEntry* entry = (ArchiveEntry*)current->data;
        printf("Found: %s (size %u, offset %u)\n", entry->fileName, entry->size, entry->offset);
        count++;
        current = current->nextElement;
    }
    printf("Total entries: %d\n", count);
    
    uint32_t size;
    char* data = (char*)archiveReadFile(arch, "unique.txt", &size);
    if (data) {
        printf("Read content: %.*s\n", (int)size, data);
        free(data);
    }
    
    archiveClose(arch);
    DeleteFile((STRPTR)archiveName);
    DeleteFile((STRPTR)file1);
    return 0;
}
