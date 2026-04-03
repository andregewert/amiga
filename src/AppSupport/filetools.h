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

//
// Created by agewert on 19.03.26.
//

#ifndef APPSUPPORT_FILETOOLS_H
#define APPSUPPORT_FILETOOLS_H

#include <stdint.h>
#include <exec/types.h>
#include "collections.h"

/**
 * Creates a unique temporary file in the given directory.
 * @param directory The directory path where the temporary file will be created.
 * @param prefix The prefix for the filename.
 * @return The full path of the created file. The caller is responsible for freeing the memory.
 */
STRPTR createTempFile(const char* directory, const char* prefix);

/**
 * Creates a unique temporary directory in the given path.
 * @param parent_dir The parent directory path where the temporary directory will be created.
 * @param prefix The prefix for the directory name.
 * @return The full path of the created directory. The caller is responsible for freeing the memory.
 */
STRPTR createTempDir(const char* parent_dir, const char* prefix);

/**
 * Reads an INI configuration file and creates a dictionary from it.
 * The keys are prefixed with the section name (e.g., "Section.Key").
 * Keys without a section are added directly.
 * @param filename The path to the INI file.
 * @return A pointer to a new dictionary, or NULL on error.
 */
dictionary* dictFromIni(const char* filename);

/**
 * Returns the base name of a path (the filename).
 * @param path The path to the file.
 * @return The base name of the path. The caller is responsible for freeing the memory.
 */
STRPTR basename(const char* path);

/**
 * Returns the directory name of a path.
 * @param path The path to the file.
 * @return The directory name of the path. The caller is responsible for freeing the memory.
 */
STRPTR dirname(const char* path);

#endif
