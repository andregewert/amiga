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

#include "archive.h"
#include <stdlib.h>
#include <string.h>
#include <proto/dos.h>

static ArchiveEntry* findEntry(Archive* archive, const char* entryName);

Archive* archiveOpen(const char* filename) {
    Archive* archive = (Archive*)malloc(sizeof(Archive));
    if (!archive) return NULL;

    archive->filename = strdup(filename);
    archive->entries = listCreate();
    archive->tocOffset = 4; // Default for new archive

    BPTR file = Open((STRPTR)filename, MODE_OLDFILE);
    if (file) {
        // Read Footer
        Seek(file, -12, OFFSET_END);
        ArchiveFooter footer;
        if (Read(file, &footer, sizeof(footer)) == sizeof(footer)) {
            if (strncmp(footer.magic, FOOTER_MAGIC, 4) == 0) {
                archive->tocOffset = footer.tocOffset;

                // Read Entries
                Seek(file, archive->tocOffset, OFFSET_BEGINNING);
                for (uint32_t i = 0; i < footer.entryCount; i++) {
                    ArchiveEntry* entry = (ArchiveEntry*)malloc(sizeof(ArchiveEntry));
                    if (Read(file, entry, sizeof(ArchiveEntry)) == sizeof(ArchiveEntry)) {
                        listAppendElement(archive->entries, entry);
                    } else {
                        free(entry);
                        break;
                    }
                }
            }
        }
        Close(file);
    }

    return archive;
}

static void freeEntry(listElement* el) {
    if (el && el->data) free(el->data);
}

void archiveClose(Archive* archive) {
    if (!archive) return;
    free(archive->filename);

    // Dispose elements in list
    listForeach(archive->entries, freeEntry);
    listDispose(archive->entries);

    free(archive);
}

static ArchiveEntry* findEntry(Archive* archive, const char* entryName) {
    if (!archive || !archive->entries) return NULL;
    listElement* current = archive->entries->firstElement;
    while (current) {
        ArchiveEntry* entry = (ArchiveEntry*)current->data;
        if (entry && strcmp(entry->fileName, entryName) == 0) return entry;
        current = current->nextElement;
    }
    return NULL;
}

BOOL archiveFileExists(Archive* archive, const char* entryName) {
    return findEntry(archive, entryName) != NULL;
}

static BOOL archiveAddOrReplaceFile(Archive* archive, const char* sourcePath, const char* entryName, BOOL replace) {
    if (!archive || !sourcePath || !entryName) return FALSE;

    ArchiveEntry* existing = findEntry(archive, entryName);
    if (existing && !replace) return FALSE;

    BPTR src = Open((STRPTR)sourcePath, MODE_OLDFILE);
    if (!src) return FALSE;

    // Get source size
    Seek(src, 0, OFFSET_END);
    uint32_t size = Seek(src, 0, OFFSET_BEGINNING);

    BPTR archFile = Open((STRPTR)archive->filename, MODE_OLDFILE);
    if (!archFile) {
        archFile = Open((STRPTR)archive->filename, MODE_NEWFILE);
        if (!archFile) {
            Close(src);
            return FALSE;
        }
        Write(archFile, (void*)ARCHIVE_MAGIC, 4);
    }

    // Seek to TOC start to overwrite it with new data
    Seek(archFile, archive->tocOffset, OFFSET_BEGINNING);
    
    char buffer[4096];
    uint32_t remaining = size;
    BOOL success = TRUE;

    while (remaining > 0) {
        uint32_t toRead = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        if (Read(src, buffer, toRead) != (LONG)toRead) {
            success = FALSE;
            break;
        }
        if (Write(archFile, buffer, toRead) != (LONG)toRead) {
            success = FALSE;
            break;
        }
        remaining -= toRead;
    }
    Close(src);

    if (!success) {
        Close(archFile);
        return FALSE;
    }

    // Create or update entry
    if (existing) {
        existing->size = size;
        existing->offset = archive->tocOffset;
    } else {
        ArchiveEntry* newEntry = (ArchiveEntry*)malloc(sizeof(ArchiveEntry));
        if (!newEntry) {
            Close(archFile);
            return FALSE;
        }
        strncpy(newEntry->fileName, entryName, 255);
        newEntry->fileName[255] = '\0';
        newEntry->size = size;
        newEntry->offset = archive->tocOffset;
        listAppendElement(archive->entries, newEntry);
    }

    // Update tocOffset for the NEXT file
    archive->tocOffset += size;

    // Write TOC
    Seek(archFile, archive->tocOffset, OFFSET_BEGINNING);
    uint32_t count = 0;
    listElement* current = archive->entries->firstElement;
    while (current) {
        Write(archFile, current->data, sizeof(ArchiveEntry));
        count++;
        current = current->nextElement;
    }

    // Write Footer
    ArchiveFooter footer;
    footer.tocOffset = archive->tocOffset;
    footer.entryCount = count;
    memcpy(footer.magic, FOOTER_MAGIC, 4);
    Write(archFile, &footer, sizeof(footer));

    Close(archFile);
    return TRUE;
}

BOOL archiveAddFile(Archive* archive, const char* sourcePath, const char* entryName) {
    return archiveAddOrReplaceFile(archive, sourcePath, entryName, FALSE);
}

BOOL archiveReplaceFile(Archive* archive, const char* sourcePath, const char* entryName) {
    return archiveAddOrReplaceFile(archive, sourcePath, entryName, TRUE);
}

linkedList* archiveGetTOC(Archive* archive) {
    return archive ? archive->entries : NULL;
}

void* archiveReadFile(Archive* archive, const char* entryName, uint32_t* outSize) {
    if (!archive || !entryName) return NULL;
    ArchiveEntry* entry = findEntry(archive, entryName);
    if (!entry) return NULL;

    BPTR file = Open((STRPTR)archive->filename, MODE_OLDFILE);
    if (!file) return NULL;

    void* buffer = malloc(entry->size);
    if (buffer) {
        Seek(file, entry->offset, OFFSET_BEGINNING);
        if (Read(file, buffer, entry->size) != entry->size) {
            free(buffer);
            buffer = NULL;
        } else if (outSize) {
            *outSize = entry->size;
        }
    }
    Close(file);
    return buffer;
}

BOOL archiveExtractFile(Archive* archive, const char* entryName, const char* destPath) {
    if (!archive || !entryName || !destPath) return FALSE;
    ArchiveEntry* entry = findEntry(archive, entryName);
    if (!entry) return FALSE;

    BPTR archFile = Open((STRPTR)archive->filename, MODE_OLDFILE);
    if (!archFile) return FALSE;

    BPTR dest = Open((STRPTR)destPath, MODE_NEWFILE);
    if (!dest) {
        Close(archFile);
        return FALSE;
    }

    Seek(archFile, entry->offset, OFFSET_BEGINNING);
    
    char buffer[4096];
    uint32_t remaining = entry->size;
    BOOL success = TRUE;

    while (remaining > 0) {
        uint32_t toRead = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        if (Read(archFile, buffer, toRead) != toRead) {
            success = FALSE;
            break;
        }
        if (Write(dest, buffer, toRead) != (LONG)toRead) {
            success = FALSE;
            break;
        }
        remaining -= toRead;
    }

    Close(dest);
    Close(archFile);
    return success;
}