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

#ifndef APPSUPPORT_ARCHVFS_H
#define APPSUPPORT_ARCHVFS_H

#include <exec/types.h>
#include <stdint.h>
#include "collections.h"
#include "archive.h"

typedef enum {
    ARCHVFS_TYPE_NONE = 0,
    ARCHVFS_TYPE_FILE,
    ARCHVFS_TYPE_DIRECTORY
} ArchVfsType;

typedef struct {
    ArchVfsType type;
    uint32_t size;
    // Potentially more metadata like permissions, date, etc.
} ArchVfsInfo;

typedef struct ArchVfs ArchVfs;
typedef struct ArchVfsFile ArchVfsFile;
typedef struct ArchVfsDir ArchVfsDir;

typedef struct {
    char name[256];
    ArchVfsType type;
    uint32_t size;
} ArchVfsDirEntry;

/**
 * Initializes a new virtual file system instance.
 * @param searchPath Semicolon-separated list of paths (directories or archives).
 * @return A pointer to the ArchVfs instance, or NULL on error.
 */
ArchVfs* archvfsCreate(const char* searchPath);

/**
 * Disposes the virtual file system instance and frees all resources.
 * @param vfs The ArchVfs instance.
 */
void archvfsDispose(ArchVfs* vfs);

/**
 * Checks if a file or directory exists in the virtual file system.
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @return TRUE if it exists, FALSE otherwise.
 */
BOOL archvfsExists(ArchVfs* vfs, const char* path);

/**
 * Determines the type of a virtual file system item.
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @return The type of the item (FILE, DIRECTORY, or NONE).
 */
ArchVfsType archvfsGetType(ArchVfs* vfs, const char* path);

/**
 * Gets information about a virtual file system item.
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @param info Pointer to an ArchVfsInfo structure to fill.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL archvfsGetInfo(ArchVfs* vfs, const char* path, ArchVfsInfo* info);

/**
 * Opens a virtual file.
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @param mode Access mode ("r", "w", "a", "rb", "wb", "ab").
 * @return A pointer to the ArchVfsFile instance, or NULL on error.
 */
ArchVfsFile* archvfsOpenFile(ArchVfs* vfs, const char* path, const char* mode);

/**
 * Reads from a virtual file.
 * @param file The ArchVfsFile instance.
 * @param buffer Buffer to read into.
 * @param size Number of bytes to read.
 * @return Number of bytes read.
 */
uint32_t archvfsReadFile(ArchVfsFile* file, void* buffer, uint32_t size);

/**
 * Writes to a virtual file.
 * @param file The ArchVfsFile instance.
 * @param buffer Buffer to write from.
 * @param size Number of bytes to write.
 * @return Number of bytes written.
 */
uint32_t archvfsWriteFile(ArchVfsFile* file, const void* buffer, uint32_t size);

/**
 * Closes a virtual file.
 * @param file The ArchVfsFile instance.
 */
void archvfsCloseFile(ArchVfsFile* file);

/**
 * Opens a virtual directory.
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @return A pointer to the ArchVfsDir instance, or NULL on error.
 */
ArchVfsDir* archvfsOpenDir(ArchVfs* vfs, const char* path);

/**
 * Reads the next entry from a virtual directory.
 * @param dir The ArchVfsDir instance.
 * @param entry Pointer to an ArchVfsDirEntry structure to fill.
 * @return TRUE if an entry was read, FALSE if no more entries.
 */
BOOL archvfsReadDir(ArchVfsDir* dir, ArchVfsDirEntry* entry);

/**
 * Closes a virtual directory.
 * @param dir The ArchVfsDir instance.
 */
void archvfsCloseDir(ArchVfsDir* dir);

/**
 * Deletes a file or directory (recursively).
 * @param vfs The ArchVfs instance.
 * @param path The virtual path.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL archvfsDelete(ArchVfs* vfs, const char* path);

#endif // APPSUPPORT_ARCHVFS_H
