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
#include "../AppSupport/network.h"
#include "../AppSupport/shell.h"

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"URL/A,FOLLOW/S";
    struct {
        char* url;
        long follow;
    } args = {NULL, 0};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);

    if (!rdargs) {
        PrintFault(IoErr(), (STRPTR)"fetchtest");
        return EXIT_FAILURE;
    }

    const char* url = args.url;
    bool follow = args.follow != 0;

    shellPrintf(SHELL_FG_CYAN, "Fetching URL: %s (follow: %d)\n", url, follow);

    FetchResponse* res = fetchUrl(url, follow);
    if (!res) {
        shellPrintf(SHELL_FG_RED, "Failed to fetch URL.\n");
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }

    shellPrintf(SHELL_FG_GREEN, "Status: %d\n", res->statusCode);
    shellPrintf(SHELL_FG_YELLOW, "Final URL: %s\n", res->finalUrl);

    if (res->body) {
        shellPrintf(SHELL_COLOR_RESET, "Body size: %d bytes\n", res->bodySize);
        // Print first 100 bytes of body
        int toPrint = res->bodySize > 100 ? 100 : res->bodySize;
        printf("Body (preview):\n%.*s\n", toPrint, (char*)res->body);
    }

    freeResponse(res);
    FreeArgs(rdargs);
    return EXIT_SUCCESS;
}
