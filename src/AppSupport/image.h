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

#ifndef APPSUPPORT_IMAGE_H
#define APPSUPPORT_IMAGE_H

#include <exec/types.h>

/**
 * Reads an IFF ILBM image and converts it into colored ASCII output printed to stdout.
 * The width of the created text is user definable.
 *
 * @param filename The path to the IFF ILBM image.
 * @param targetWidth The desired width of the ASCII output in characters.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL imagePrintAscii(const char* filename, uint32_t targetWidth);

#endif // APPSUPPORT_IMAGE_H
