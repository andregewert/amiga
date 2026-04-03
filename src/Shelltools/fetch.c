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
#include <stdbool.h>
#include <proto/dos.h>
#include "AppSupport/network.h"

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"URL/A,TO/K,FOLLOW/S";
    struct {
        char* url;
        char* to;
        long follow;
    } args = {NULL, NULL, 0};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);

    if (!rdargs) {
        PrintFault(IoErr(), (STRPTR)"fetch");
        return EXIT_FAILURE;
    }

    const char* url = args.url;
    const char* to = args.to;
    bool follow = args.follow != 0;

    FetchResponse* res = fetchUrl(url, follow);
    if (!res) {
        fprintf(stderr, "Failed to fetch URL: %s\n", url);
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }

    if (res->statusCode < 200 || res->statusCode >= 300) {
        fprintf(stderr, "HTTP Error: %u (URL: %s)\n", (unsigned int)res->statusCode, res->finalUrl);
        freeResponse(res);
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }

    if (to) {
        BPTR file = Open((STRPTR)to, MODE_NEWFILE);
        if (!file) {
            PrintFault(IoErr(), (STRPTR)to);
            freeResponse(res);
            FreeArgs(rdargs);
            return EXIT_FAILURE;
        }
        if (res->bodySize > 0) {
            if (Write(file, res->body, (LONG)res->bodySize) != (LONG)res->bodySize) {
                PrintFault(IoErr(), (STRPTR)to);
                Close(file);
                freeResponse(res);
                FreeArgs(rdargs);
                return EXIT_FAILURE;
            }
        }
        Close(file);
    } else {
        if (res->body && res->bodySize > 0) {
            fwrite(res->body, 1, res->bodySize, stdout);
        }
    }

    freeResponse(res);
    FreeArgs(rdargs);
    return EXIT_SUCCESS;
}
