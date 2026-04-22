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

#include "archvfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include "filetools.h"

typedef struct {
    char* path;
    BOOL isArchive;
    Archive* archive;
} SearchPathEntry;

struct ArchVfs {
    linkedList* searchPaths;
};

struct ArchVfsFile {
    ArchVfs* vfs;
    SearchPathEntry* sourceEntry;
    char* virtualPath;
    char* tempPath;
    BPTR dosFile;
    void* archiveData;
    uint32_t archiveSize;
    uint32_t archivePos;
    BOOL writeMode;
};

struct ArchVfsDir {
    ArchVfs* vfs;
    char* virtualPath;
    uint32_t currentSearchPathIndex;
    // For real directories
    BPTR lock;
    struct FileInfoBlock* fib;
    BOOL examined;
    // For archives
    uint32_t currentArchiveEntryIndex;
    linkedList* seenEntries; // To avoid duplicates when same file exists in multiple paths
};

static void freeSearchPathEntry(listElement* el) {
    SearchPathEntry* entry = (SearchPathEntry*)el->data;
    if (entry->archive) {
        archiveClose(entry->archive);
    }
    if (entry->path) {
        free(entry->path);
    }
    free(entry);
}

static BOOL deleteRecursivePhysical(const char* fullPath) {
    BPTR lock = Lock((STRPTR)fullPath, SHARED_LOCK);
    if (!lock) return FALSE;

    struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        UnLock(lock);
        return FALSE;
    }

    BOOL success = TRUE;
    if (Examine(lock, fib)) {
        if (fib->fib_DirEntryType >= 0) {
            // It's a directory, iterate over its contents
            while (ExNext(lock, fib)) {
                char subPath[512];
                snprintf(subPath, sizeof(subPath), "%s/%s", fullPath, fib->fib_FileName);
                if (!deleteRecursivePhysical(subPath)) {
                    success = FALSE;
                }
            }
        }
    } else {
        success = FALSE;
    }

    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);

    if (success) {
        if (!DeleteFile((STRPTR)fullPath)) {
            success = FALSE;
        }
    }

    return success;
}

ArchVfs* archvfsCreate(const char* searchPath) {
    ArchVfs* vfs = malloc(sizeof(ArchVfs));
    if (!vfs) return NULL;

    vfs->searchPaths = listCreate();
    if (!vfs->searchPaths) {
        free(vfs);
        return NULL;
    }
    
    char* pathCopy = strdup(searchPath);
    if (!pathCopy) {
        listDispose(vfs->searchPaths);
        free(vfs);
        return NULL;
    }
    char* token = strtok(pathCopy, ";");
    while (token) {
        SearchPathEntry* entry = malloc(sizeof(SearchPathEntry));
        if (!entry) {
            token = strtok(NULL, ";");
            continue;
        }
        entry->path = strdup(token);
        if (!entry->path) {
            free(entry);
            token = strtok(NULL, ";");
            continue;
        }
        
        // Try opening as archive. We only consider it an archive if it's not a directory.
        BPTR lock = Lock((STRPTR)entry->path, SHARED_LOCK);
        BOOL isDir = FALSE;
        if (lock) {
            struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, NULL);
            if (Examine(lock, fib)) {
                if (fib->fib_DirEntryType >= 0) isDir = TRUE;
            }
            FreeDosObject(DOS_FIB, fib);
            UnLock(lock);
        }

        if (!isDir) {
            entry->archive = archiveOpen(entry->path);
        } else {
            entry->archive = NULL;
        }

        if (entry->archive) {
            entry->isArchive = TRUE;
        } else {
            entry->isArchive = FALSE;
        }
        
        listAppendElement(vfs->searchPaths, entry);
        token = strtok(NULL, ";");
    }
    free(pathCopy);

    return vfs;
}

void archvfsDispose(ArchVfs* vfs) {
    if (!vfs) return;
    listForeach(vfs->searchPaths, freeSearchPathEntry);
    listDispose(vfs->searchPaths);
    free(vfs);
}

static char* translatePath(const char* virtualPath) {
    if (virtualPath[0] == '/') {
        return strdup(virtualPath + 1);
    }
    return strdup(virtualPath);
}

ArchVfsType archvfsGetType(ArchVfs* vfs, const char* path) {
    char* relPath = translatePath(path);
    ArchVfsType type = ARCHVFS_TYPE_NONE;

    for (uint32_t i = 0; i < vfs->searchPaths->length; i++) {
        SearchPathEntry* entry = (SearchPathEntry*)listGetElementAt(vfs->searchPaths, i)->data;
        
        if (entry->isArchive) {
            archiveRefresh(entry->archive);
            if (archiveFileExists(entry->archive, relPath)) {
                type = ARCHVFS_TYPE_FILE;
                break;
            }
            // Directory support in archive is limited, but we can check if it's a prefix
            linkedList* toc = archiveGetTOC(entry->archive);
            for (uint32_t j = 0; j < toc->length; j++) {
                ArchiveEntry* ae = (ArchiveEntry*)listGetElementAt(toc, j)->data;
                if (strncmp(ae->fileName, relPath, strlen(relPath)) == 0) {
                    if (relPath[0] == '\0' || ae->fileName[strlen(relPath)] == '/') {
                        if (strlen(ae->fileName) > strlen(relPath) || relPath[0] == '\0') {
                            type = ARCHVFS_TYPE_DIRECTORY;
                            break;
                        } else {
                            type = ARCHVFS_TYPE_FILE;
                            break;
                        }
                    }
                }
            }
            if (type != ARCHVFS_TYPE_NONE) break;
        } else {
            char fullPath[512];
            if (relPath[0] == '\0') strcpy(fullPath, entry->path);
            else snprintf(fullPath, sizeof(fullPath), "%s/%s", entry->path, relPath);
            BPTR lock = Lock((STRPTR)fullPath, SHARED_LOCK);
            if (lock) {
                struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, NULL);
                if (Examine(lock, fib)) {
                    if (fib->fib_DirEntryType >= 0) {
                        type = ARCHVFS_TYPE_DIRECTORY;
                    } else {
                        type = ARCHVFS_TYPE_FILE;
                    }
                }
                FreeDosObject(DOS_FIB, fib);
                UnLock(lock);
                break;
            }
        }
    }

    free(relPath);
    return type;
}

BOOL archvfsExists(ArchVfs* vfs, const char* path) {
    return archvfsGetType(vfs, path) != ARCHVFS_TYPE_NONE;
}

ArchVfsFile* archvfsOpenFile(ArchVfs* vfs, const char* path, const char* mode) {
    char* relPath = translatePath(path);
    ArchVfsFile* file = NULL;

    BOOL write = (strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL);

    if (write) {
        // For writing, find the first entry that supports writing (real dir or archive)
        for (uint32_t i = 0; i < vfs->searchPaths->length; i++) {
            SearchPathEntry* entry = (SearchPathEntry*)listGetElementAt(vfs->searchPaths, i)->data;
            if (!entry->isArchive) {
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", entry->path, relPath);
                long dosMode = MODE_NEWFILE;
                if (strchr(mode, 'a')) dosMode = MODE_READWRITE;
                
                BPTR dosFile = Open((STRPTR)fullPath, dosMode);
                if (dosFile) {
                    if (strchr(mode, 'a')) Seek(dosFile, 0, OFFSET_END);
                    file = calloc(1, sizeof(ArchVfsFile));
                    if (!file) {
                        Close(dosFile);
                        break;
                    }
                    file->vfs = vfs;
                    file->dosFile = dosFile;
                    file->writeMode = TRUE;
                    file->virtualPath = strdup(relPath);
                    file->sourceEntry = entry;
                    break;
                }
            } else {
                archiveRefresh(entry->archive);
                // Archive writing: extract to temp file first if appending, otherwise create empty temp file
                char* tempFile = createTempFile("T:", "vfs_tmp");
                if (tempFile) {
                    if (strchr(mode, 'a')) {
                        archiveExtractFile(entry->archive, relPath, tempFile);
                    }
                    
                    long dosMode = strchr(mode, 'a') ? MODE_READWRITE : MODE_NEWFILE;
                    BPTR dosFile = Open((STRPTR)tempFile, dosMode);
                    if (dosFile) {
                        if (strchr(mode, 'a')) Seek(dosFile, 0, OFFSET_END);
                        file = calloc(1, sizeof(ArchVfsFile));
                        if (!file) {
                            Close(dosFile);
                            free(tempFile);
                            break;
                        }
                        file->vfs = vfs;
                        file->dosFile = dosFile;
                        file->writeMode = TRUE;
                        file->tempPath = tempFile;
                        file->virtualPath = strdup(relPath);
                        file->sourceEntry = entry;
                        break;
                    }
                    free(tempFile);
                }
            }
        }
    } else {
        // For reading, search through all paths
        for (uint32_t i = 0; i < vfs->searchPaths->length; i++) {
            SearchPathEntry* entry = (SearchPathEntry*)listGetElementAt(vfs->searchPaths, i)->data;
            if (entry->isArchive) {
                archiveRefresh(entry->archive);
                uint32_t size;
                void* data = archiveReadFile(entry->archive, relPath, &size);
                if (data) {
                    file = calloc(1, sizeof(ArchVfsFile));
                    if (!file) {
                        free(data);
                        break;
                    }
                    file->vfs = vfs;
                    file->archiveData = data;
                    file->archiveSize = size;
                    file->archivePos = 0;
                    file->sourceEntry = entry;
                    break;
                }
            } else {
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", entry->path, relPath);
                BPTR dosFile = Open((STRPTR)fullPath, MODE_OLDFILE);
                if (dosFile) {
                    file = calloc(1, sizeof(ArchVfsFile));
                    if (!file) {
                        Close(dosFile);
                        break;
                    }
                    file->vfs = vfs;
                    file->dosFile = dosFile;
                    file->sourceEntry = entry;
                    break;
                }
            }
        }
    }

    free(relPath);
    return file;
}

uint32_t archvfsReadFile(ArchVfsFile* file, void* buffer, uint32_t size) {
    if (file->writeMode) return 0;
    if (file->dosFile) {
        return Read(file->dosFile, buffer, size);
    } else if (file->archiveData) {
        uint32_t remaining = file->archiveSize - file->archivePos;
        uint32_t toRead = (size < remaining) ? size : remaining;
        memcpy(buffer, (char*)file->archiveData + file->archivePos, toRead);
        file->archivePos += toRead;
        return toRead;
    }
    return 0;
}

uint32_t archvfsWriteFile(ArchVfsFile* file, const void* buffer, uint32_t size) {
    if (!file->writeMode || !file->dosFile) return 0;
    return Write(file->dosFile, (void*)buffer, size);
}

void archvfsCloseFile(ArchVfsFile* file) {
    if (!file) return;
    if (file->dosFile) Close(file->dosFile);
    
    if (file->writeMode && file->sourceEntry && file->sourceEntry->isArchive && file->tempPath) {
        // Commit changes to archive
        archiveReplaceFile(file->sourceEntry->archive, file->tempPath, file->virtualPath);
        DeleteFile((STRPTR)file->tempPath);
    }

    if (file->tempPath) free(file->tempPath);
    if (file->virtualPath) free(file->virtualPath);
    if (file->archiveData) free(file->archiveData);
    free(file);
}

ArchVfsDir* archvfsOpenDir(ArchVfs* vfs, const char* path) {
    ArchVfsDir* dir = calloc(1, sizeof(ArchVfsDir));
    if (!dir) {
        return NULL;
    }
    dir->vfs = vfs;
    dir->virtualPath = translatePath(path);
    if (!dir->virtualPath) {
        free(dir);
        return NULL;
    }
    dir->seenEntries = listCreate();
    if (!dir->seenEntries) {
        free(dir->virtualPath);
        free(dir);
        return NULL;
    }
    return dir;
}

BOOL archvfsReadDir(ArchVfsDir* dir, ArchVfsDirEntry* entry) {
    char* relPath = dir->virtualPath;
    
    while (dir->currentSearchPathIndex < dir->vfs->searchPaths->length) {
        SearchPathEntry* spe = (SearchPathEntry*)listGetElementAt(dir->vfs->searchPaths, dir->currentSearchPathIndex)->data;
        if (spe->isArchive) {
            if (dir->currentArchiveEntryIndex == 0) {
                archiveRefresh(spe->archive);
            }
            linkedList* toc = archiveGetTOC(spe->archive);
            while (dir->currentArchiveEntryIndex < toc->length) {
                ArchiveEntry* ae = (ArchiveEntry*)listGetElementAt(toc, dir->currentArchiveEntryIndex++)->data;
                
                // Check if this entry is in the requested virtual directory
                if (relPath[0] == '\0' || (strncmp(ae->fileName, relPath, strlen(relPath)) == 0 && ae->fileName[strlen(relPath)] == '/')) {
                    const char* subName = (relPath[0] == '\0') ? ae->fileName : ae->fileName + strlen(relPath) + 1;
                    if (subName[0] == '\0') continue; // Should not happen for a file in a subdir
                    const char* slash = strchr(subName, '/');
                    
                    if (slash) {
                        // It's a directory entry
                        size_t dirNameLen = slash - subName;
                        if (dirNameLen >= sizeof(entry->name)) dirNameLen = sizeof(entry->name) - 1;
                        strncpy(entry->name, subName, dirNameLen);
                        entry->name[dirNameLen] = '\0';
                        entry->type = ARCHVFS_TYPE_DIRECTORY;
                        entry->size = 0;
                    } else {
                        // It's a file entry
                        strncpy(entry->name, subName, sizeof(entry->name) - 1);
                        entry->name[sizeof(entry->name) - 1] = '\0';
                        entry->type = ARCHVFS_TYPE_FILE;
                        entry->size = ae->size;
                    }
                    
                    // Avoid duplicates
                    BOOL seen = FALSE;
                    for (uint32_t i = 0; i < dir->seenEntries->length; i++) {
                        if (strcmp((char*)listGetElementAt(dir->seenEntries, i)->data, entry->name) == 0) {
                            seen = TRUE; break;
                        }
                    }
                    if (!seen) {
                        listAppendElement(dir->seenEntries, strdup(entry->name));
                        return TRUE;
                    }
                }
            }
            dir->currentArchiveEntryIndex = 0;
            dir->currentSearchPathIndex++;
        } else {
            if (!dir->lock) {
                char fullPath[512];
                if (relPath[0] == '\0') strcpy(fullPath, spe->path);
                else snprintf(fullPath, sizeof(fullPath), "%s/%s", spe->path, relPath);
                
                dir->lock = Lock((STRPTR)fullPath, SHARED_LOCK);
                if (dir->lock) {
                    dir->fib = AllocDosObject(DOS_FIB, NULL);
                    dir->examined = FALSE;
                } else {
                    dir->currentSearchPathIndex++;
                }
            }
            
            while (dir->lock) {
                if (!dir->examined) {
                    if (Examine(dir->lock, dir->fib)) {
                        dir->examined = TRUE;
                    } else {
                        FreeDosObject(DOS_FIB, dir->fib);
                        UnLock(dir->lock);
                        dir->lock = 0;
                        dir->fib = NULL;
                        dir->currentSearchPathIndex++;
                        break;
                    }
                }

                if (ExNext(dir->lock, dir->fib)) {
                    strncpy(entry->name, dir->fib->fib_FileName, sizeof(entry->name) - 1);
                    entry->name[sizeof(entry->name) - 1] = '\0';
                    entry->type = (dir->fib->fib_DirEntryType >= 0) ? ARCHVFS_TYPE_DIRECTORY : ARCHVFS_TYPE_FILE;
                    entry->size = dir->fib->fib_Size;
                    
                    BOOL seen = FALSE;
                    for (uint32_t i = 0; i < dir->seenEntries->length; i++) {
                        if (strcmp((char*)listGetElementAt(dir->seenEntries, i)->data, entry->name) == 0) {
                            seen = TRUE; break;
                        }
                    }
                    if (!seen) {
                        listAppendElement(dir->seenEntries, strdup(entry->name));
                        return TRUE;
                    }
                } else {
                    FreeDosObject(DOS_FIB, dir->fib);
                    UnLock(dir->lock);
                    dir->lock = 0;
                    dir->fib = NULL;
                    dir->currentSearchPathIndex++;
                    break; // Go to next search path
                }
            }
        }
    }
    return FALSE;
}

void archvfsCloseDir(ArchVfsDir* dir) {
    if (!dir) return;
    if (dir->lock) {
        FreeDosObject(DOS_FIB, dir->fib);
        UnLock(dir->lock);
    }
    if (dir->seenEntries) {
        for (uint32_t i = 0; i < dir->seenEntries->length; i++) {
            free(listGetElementAt(dir->seenEntries, i)->data);
        }
        listDispose(dir->seenEntries);
    }
    free(dir->virtualPath);
    free(dir);
}

BOOL archvfsDelete(ArchVfs* vfs, const char* path) {
    char* relPath = translatePath(path);
    BOOL success = FALSE;
    
    // Search through all search paths to find where the item is
    for (uint32_t i = 0; i < vfs->searchPaths->length; i++) {
        SearchPathEntry* entry = (SearchPathEntry*)listGetElementAt(vfs->searchPaths, i)->data;
        
        if (entry->isArchive) {
            archiveRefresh(entry->archive);
            // Check if it's a file in archive
            if (archiveFileExists(entry->archive, relPath)) {
                success = archiveDeleteFile(entry->archive, relPath);
                if (success) break;
            }
            
            // Check if it's a "directory" in archive
            // We consider it a directory if any file starts with relPath + "/"
            linkedList* toc = archiveGetTOC(entry->archive);
            listElement* current = toc->firstElement;
            BOOL isDir = FALSE;
            while (current) {
                ArchiveEntry* ae = (ArchiveEntry*)current->data;
                if (strncmp(ae->fileName, relPath, strlen(relPath)) == 0) {
                    if (relPath[0] == '\0' || ae->fileName[strlen(relPath)] == '/') {
                        if (strlen(ae->fileName) > strlen(relPath) || relPath[0] == '\0') {
                            isDir = TRUE;
                            break;
                        }
                    }
                }
                current = current->nextElement;
            }
            
            if (isDir) {
                // Delete all files that have this prefix
                // We must collect names first to avoid iterator invalidation if archiveDeleteFile modifies the list
                // Actually archiveDeleteFile in my current implementation is safe to call if we restart or use a robust way.
                // Let's collect names.
                linkedList* toDelete = listCreate();
                current = toc->firstElement;
                while (current) {
                    ArchiveEntry* ae = (ArchiveEntry*)current->data;
                    if (strncmp(ae->fileName, relPath, strlen(relPath)) == 0) {
                        if (relPath[0] == '\0' || ae->fileName[strlen(relPath)] == '/') {
                            if (strlen(ae->fileName) > strlen(relPath) || relPath[0] == '\0') {
                                listAppendElement(toDelete, strdup(ae->fileName));
                            }
                        }
                    }
                    current = current->nextElement;
                }
                
                success = (toDelete->length > 0);
                while (toDelete->length > 0) {
                    listElement* el = listRemoveElementAt(toDelete, 0);
                    char* name = (char*)el->data;
                    if (!archiveDeleteFile(entry->archive, name)) success = FALSE;
                    free(name);
                    free(el);
                }
                listDispose(toDelete);
                if (success) break;
            }
        } else {
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", entry->path, relPath);
            
            BPTR lock = Lock((STRPTR)fullPath, SHARED_LOCK);
            if (lock) {
                UnLock(lock);
                // Recursive delete for Amiga
                if (DeleteFile((STRPTR)fullPath)) {
                    success = TRUE;
                } else {
                    // Try recursive
                    if (deleteRecursivePhysical(fullPath)) {
                        success = TRUE;
                    }
                }
                if (success) break;
            }
        }
    }
    
    free(relPath);
    return success;
}

BOOL archvfsGetInfo(ArchVfs* vfs, const char* path, ArchVfsInfo* info) {
    char* relPath = translatePath(path);
    BOOL found = FALSE;

    for (uint32_t i = 0; i < vfs->searchPaths->length; i++) {
        SearchPathEntry* entry = (SearchPathEntry*)listGetElementAt(vfs->searchPaths, i)->data;
        
        if (entry->isArchive) {
            archiveRefresh(entry->archive);
            ArchiveEntry* ae = NULL;
            linkedList* toc = archiveGetTOC(entry->archive);
            listElement* current = toc->firstElement;
            while (current) {
                ArchiveEntry* candidate = (ArchiveEntry*)current->data;
                if (strcmp(candidate->fileName, relPath) == 0) {
                    ae = candidate;
                    break;
                }
                current = current->nextElement;
            }

            if (ae) {
                info->type = ARCHVFS_TYPE_FILE;
                info->size = ae->size;
                found = TRUE;
                break;
            }
            // Check for directory in archive (same logic as getType)
            current = toc->firstElement;
            while (current) {
                ae = (ArchiveEntry*)current->data;
                if (strncmp(ae->fileName, relPath, strlen(relPath)) == 0) {
                    if (relPath[0] == '\0' || ae->fileName[strlen(relPath)] == '/') {
                        if (strlen(ae->fileName) > strlen(relPath) || relPath[0] == '\0') {
                            info->type = ARCHVFS_TYPE_DIRECTORY;
                            info->size = 0;
                            found = TRUE;
                            break;
                        }
                    }
                }
                current = current->nextElement;
            }
            if (found) break;
        } else {
            char fullPath[512];
            if (relPath[0] == '\0') strcpy(fullPath, entry->path);
            else snprintf(fullPath, sizeof(fullPath), "%s/%s", entry->path, relPath);
            BPTR lock = Lock((STRPTR)fullPath, SHARED_LOCK);
            if (lock) {
                struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, NULL);
                if (Examine(lock, fib)) {
                    if (fib->fib_DirEntryType >= 0) {
                        info->type = ARCHVFS_TYPE_DIRECTORY;
                        info->size = 0;
                    } else {
                        info->type = ARCHVFS_TYPE_FILE;
                        info->size = fib->fib_Size;
                    }
                    found = TRUE;
                }
                FreeDosObject(DOS_FIB, fib);
                UnLock(lock);
                break;
            }
        }
    }

    free(relPath);
    return found;
}
