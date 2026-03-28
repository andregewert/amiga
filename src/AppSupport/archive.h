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

#ifndef APPSUPPORT_ARCHIVE_H
#define APPSUPPORT_ARCHIVE_H

#include <stdint.h>
#include <exec/types.h>
#include "collections.h"

#define ARCHIVE_MAGIC "ARCH"
#define FOOTER_MAGIC  "FOOT"

struct Archive {
    char* filename;
    linkedList* entries;
    uint32_t tocOffset;
};

typedef struct {
    uint32_t tocOffset;
    uint32_t entryCount;
    char magic[4];
} ArchiveFooter;

typedef struct {
    char fileName[256];
    uint32_t size;
    uint32_t offset;
} ArchiveEntry;

typedef struct Archive Archive;

/**
 * Opens an archive file. If the file doesn't exist, it will be created on the first addition.
 * @param filename The path to the archive.
 * @return A pointer to the archive handle, or NULL on error.
 */
Archive* archiveOpen(const char* filename);

/**
 * Closes the archive and frees all resources.
 * @param archive The archive handle.
 */
void archiveClose(Archive* archive);

/**
 * Adds a file to the archive. If a file with the same entry name already exists, 
 * it will return FALSE.
 * @param archive The archive handle.
 * @param sourcePath The path to the source file to add.
 * @param entryName The name the file should have in the archive.
 * @return TRUE if successful, FALSE otherwise (e.g. if file already exists).
 */
BOOL archiveAddFile(Archive* archive, const char* sourcePath, const char* entryName);

/**
 * Replaces an existing file in the archive with a new one. 
 * If the entry does not exist, it will be added.
 * @param archive The archive handle.
 * @param sourcePath The path to the source file.
 * @param entryName The name of the file to replace in the archive.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL archiveReplaceFile(Archive* archive, const char* sourcePath, const char* entryName);

/**
 * Checks if a file exists in the archive.
 * @param archive The archive handle.
 * @param entryName The name of the file to check.
 * @return TRUE if it exists, FALSE otherwise.
 */
BOOL archiveFileExists(Archive* archive, const char* entryName);

/**
 * Returns the Table of Contents (TOC) of the archive.
 * @param archive The archive handle.
 * @return A pointer to a linked list of ArchiveEntry pointers. Do not free this list.
 */
linkedList* archiveGetTOC(Archive* archive);

/**
 * Reads a single file from the archive into memory.
 * @param archive The archive handle.
 * @param entryName The name of the file in the archive.
 * @param outSize A pointer where the file size will be stored.
 * @return A pointer to the file data (allocated with malloc), or NULL on error. The caller is responsible for freeing.
 */
void* archiveReadFile(Archive* archive, const char* entryName, uint32_t* outSize);

/**
 * Extracts a single file from the archive to a local file.
 * @param archive The archive handle.
 * @param entryName The name of the file in the archive.
 * @param destPath The destination path.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL archiveExtractFile(Archive* archive, const char* entryName, const char* destPath);

#endif // APPSUPPORT_ARCHIVE_H
