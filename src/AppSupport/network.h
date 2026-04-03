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

#ifndef APPSUPPORT_NETWORK_H
#define APPSUPPORT_NETWORK_H

#include "collections.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <exec/types.h>

typedef struct FetchResponse {
    uint32_t statusCode;
    dictionary* headers;
    void* body;
    size_t bodySize;
    STRPTR finalUrl;
} FetchResponse;

/**
 * Fetches data from a URL.
 * @param url The URL to fetch.
 * @param followRedirects If true, follows HTTP redirects.
 * @return A pointer to a FetchResponse structure, or NULL on error.
 *         The caller is responsible for calling freeResponse() on the result.
 */
FetchResponse* fetchUrl(const char* url, bool followRedirects);

/**
 * Frees a FetchResponse structure and its contents.
 * @param response The response to free.
 */
void freeResponse(FetchResponse* response);

#endif //APPSUPPORT_NETWORK_H
