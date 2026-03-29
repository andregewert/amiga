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
#include <stdio.h>
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
                    ArchiveEntry* entry = (ArchiveEntry*)calloc(1, sizeof(ArchiveEntry));
                    uint32_t nameLen = 0;
                    if (Read(file, &nameLen, sizeof(uint32_t)) == sizeof(uint32_t)) {
                        if (nameLen < ARCHIVE_MAX_FILENAME && Read(file, entry->fileName, nameLen) == nameLen) {
                            entry->fileName[nameLen] = '\0';
                            if (Read(file, &entry->size, sizeof(uint32_t)) == sizeof(uint32_t) &&
                                Read(file, &entry->offset, sizeof(uint32_t)) == sizeof(uint32_t)) {
                                listAppendElement(archive->entries, entry);
                                continue;
                            }
                        }
                    }
                    free(entry);
                    break;
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

    if (strlen(entryName) >= ARCHIVE_MAX_FILENAME) {
        fprintf(stderr, "Filename too long: %s\n", entryName);
        return FALSE;
    }

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
        memset(newEntry->fileName, 0, sizeof(newEntry->fileName));
        strncpy(newEntry->fileName, entryName, sizeof(newEntry->fileName) - 1);
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
        ArchiveEntry* entry = (ArchiveEntry*)current->data;
        uint32_t nameLen = strlen(entry->fileName);
        Write(archFile, &nameLen, sizeof(uint32_t));
        Write(archFile, entry->fileName, nameLen);
        Write(archFile, &entry->size, sizeof(uint32_t));
        Write(archFile, &entry->offset, sizeof(uint32_t));
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

    void* buffer = calloc(entry->size +1, 1);
    //void* buffer = malloc(entry->size + 1);
    if (buffer) {
        Seek(file, entry->offset, OFFSET_BEGINNING);
        if (Read(file, buffer, entry->size) != entry->size) {
            free(buffer);
            buffer = NULL;
        } else {
            ((char*)buffer)[entry->size] = '\0';
            if (outSize) {
                *outSize = entry->size;
            }
        }
    }
    Close(file);
    return buffer;
}

static void createPath(const char* path) {
    char temp[256];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char* p = temp;
    // Skip volume name (e.g., "T:")
    char* colon = strchr(p, ':');
    if (colon) {
        p = colon + 1;
    }

    while (*p) {
        if (*p == '/') {
            *p = '\0';
            BPTR lock = CreateDir((STRPTR)temp);
            if (lock) {
                UnLock(lock);
            }
            *p = '/';
        }
        p++;
    }
}

BOOL archiveExtractFile(Archive* archive, const char* entryName, const char* destPath) {
    if (!archive || !entryName || !destPath) return FALSE;
    ArchiveEntry* entry = findEntry(archive, entryName);
    if (!entry) return FALSE;

    BPTR archFile = Open((STRPTR)archive->filename, MODE_OLDFILE);
    if (!archFile) return FALSE;

    createPath(destPath);
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

BOOL archiveExtractAll(Archive* archive, const char* destDir) {
    if (!archive || !destDir) return FALSE;

    size_t destLen = strlen(destDir);
    char lastChar = destLen > 0 ? destDir[destLen - 1] : '\0';
    BOOL needsSeparator = (lastChar != '\0' && lastChar != ':' && lastChar != '/');

    listElement* current = archive->entries->firstElement;
    while (current) {
        ArchiveEntry* entry = (ArchiveEntry*)current->data;
        char fullDestPath[512];

        if (needsSeparator) {
            snprintf(fullDestPath, sizeof(fullDestPath), "%s/%s", destDir, entry->fileName);
        } else {
            snprintf(fullDestPath, sizeof(fullDestPath), "%s%s", destDir, entry->fileName);
        }

        if (!archiveExtractFile(archive, entry->fileName, fullDestPath)) {
            return FALSE;
        }

        current = current->nextElement;
    }

    return TRUE;
}

static BOOL archiveAddRecursive(Archive* archive, const char* sourceDir, const char* entryPrefix) {
    if (!archive || !sourceDir) return FALSE;

    BPTR lock = Lock((STRPTR)sourceDir, ACCESS_READ);
    if (!lock) return FALSE;

    struct FileInfoBlock* fib = (struct FileInfoBlock*)AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        UnLock(lock);
        return FALSE;
    }

    BOOL success = TRUE;
    if (Examine(lock, fib)) {
        while (ExNext(lock, fib)) {
            char fullPath[512];
            char entryName[512];

            if (strcmp(fib->fib_FileName, ".") == 0 || strcmp(fib->fib_FileName, "..") == 0) continue;

            // Build full local path
            if (entryPrefix && strlen(entryPrefix) > 0) {
                snprintf(entryName, sizeof(entryName), "%s/%s", entryPrefix, fib->fib_FileName);
            } else {
                strncpy(entryName, fib->fib_FileName, sizeof(entryName) - 1);
                entryName[sizeof(entryName) - 1] = '\0';
            }

            size_t dirLen = strlen(sourceDir);
            char lastChar = sourceDir[dirLen - 1];
            if (lastChar == ':' || lastChar == '/') {
                snprintf(fullPath, sizeof(fullPath), "%s%s", sourceDir, fib->fib_FileName);
            } else {
                snprintf(fullPath, sizeof(fullPath), "%s/%s", sourceDir, fib->fib_FileName);
            }

            if (fib->fib_DirEntryType >= 0) {
                // It's a directory, recurse
                if (!archiveAddRecursive(archive, fullPath, entryName)) {
                    success = FALSE;
                    break;
                }
            } else {
                // It's a file, add it
                if (!archiveAddFile(archive, fullPath, entryName)) {
                    success = FALSE;
                    break;
                }
            }
        }
    } else {
        success = FALSE;
    }

    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);
    return success;
}

BOOL archiveAddDirectory(Archive* archive, const char* sourceDir, const char* entryPrefix) {
    return archiveAddRecursive(archive, sourceDir, entryPrefix);
}