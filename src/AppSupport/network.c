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

#include "network.h"
#include <stdio.h>
#include <stdlib.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <netdb.h>
#include <proto/socket.h>
#include <amissl/amissl.h>
#include <libraries/amisslmaster.h>
#include <proto/amisslmaster.h>

struct Library *SocketBase = NULL;
struct Library *AmiSSLMasterBase = NULL;
struct Library *AmiSSLBase = NULL;
struct Library *AmiSSLExtBase = NULL;
struct Library *UtilityBase = NULL;

static char *strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* res = malloc(len);
    if (res) memcpy(res, str, len);
    return res;
}

static void closeSocketLibrary();

static bool openSocketLibrary() {
    static bool atexit_registered = false;
    if (!atexit_registered) {
        atexit(closeSocketLibrary);
        atexit_registered = true;
    }

    if (!UtilityBase) {
        UtilityBase = OpenLibrary("utility.library", 0);
    }

    if (!SocketBase) {
        SocketBase = OpenLibrary("bsdsocket.library", 4);
        if (!SocketBase) return false;
    }

    if (!AmiSSLMasterBase) {
        AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION);
        if (AmiSSLMasterBase) {
            if (InitAmiSSLMaster(AMISSL_V11x, TRUE)) {
                AmiSSLBase = OpenAmiSSL();
                if (AmiSSLBase) {
                    if (InitAmiSSL(AmiSSL_ErrNoPtr, (uintptr_t)&errno,
                                   AmiSSL_SocketBase, (uintptr_t)SocketBase,
                                   TAG_DONE) != 0) {
                        CloseAmiSSL();
                        AmiSSLBase = NULL;
                        CloseLibrary(AmiSSLMasterBase);
                        AmiSSLMasterBase = NULL;
                    } else {
                        OPENSSL_init_ssl(OPENSSL_INIT_SSL_DEFAULT, NULL);
                    }
                } else {
                    CloseLibrary(AmiSSLMasterBase);
                    AmiSSLMasterBase = NULL;
                }
            } else {
                CloseLibrary(AmiSSLMasterBase);
                AmiSSLMasterBase = NULL;
            }
        }
    }

    return true;
}

static void closeSocketLibrary() {
    if (AmiSSLBase) {
        CleanupAmiSSLA(NULL);
        CloseAmiSSL();
        AmiSSLBase = NULL;
    }
    if (AmiSSLMasterBase) {
        CloseLibrary(AmiSSLMasterBase);
        AmiSSLMasterBase = NULL;
    }
    if (SocketBase) {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
    }
    if (UtilityBase) {
        CloseLibrary(UtilityBase);
        UtilityBase = NULL;
    }
}

typedef struct ParsedUrl {
    STRPTR scheme;
    STRPTR host;
    int port;
    STRPTR path;
} ParsedUrl;

static void freeParsedUrl(ParsedUrl* p) {
    if (p->scheme) free(p->scheme);
    if (p->host) free(p->host);
    if (p->path) free(p->path);
    free(p);
}

static ParsedUrl* parseUrl(const char* url) {
    ParsedUrl* p = calloc(1, sizeof(ParsedUrl));
    if (!p) return NULL;

    const char* start = url;
    const char* schemeEnd = strstr(start, "://");
    if (schemeEnd) {
        p->scheme = malloc(schemeEnd - start + 1);
        strncpy(p->scheme, start, schemeEnd - start);
        p->scheme[schemeEnd - start] = '\0';
        start = schemeEnd + 3;
    } else {
        p->scheme = strdup("http");
    }

    const char* hostEnd = strchr(start, '/');
    if (!hostEnd) hostEnd = start + strlen(start);

    const char* portStart = strchr(start, ':');
    if (portStart && portStart < hostEnd) {
        p->host = malloc(portStart - start + 1);
        strncpy(p->host, start, portStart - start);
        p->host[portStart - start] = '\0';
        p->port = atoi(portStart + 1);
    } else {
        p->host = malloc(hostEnd - start + 1);
        strncpy(p->host, start, hostEnd - start);
        p->host[hostEnd - start] = '\0';
        p->port = (strcmp(p->scheme, "https") == 0) ? 443 : 80;
    }

    if (*hostEnd == '/') {
        p->path = strdup(hostEnd);
    } else {
        p->path = strdup("/");
    }

    return p;
}

void freeResponse(FetchResponse* response) {
    if (!response) return;
    if (response->headers) dictDispose(response->headers);
    if (response->body) free(response->body);
    if (response->finalUrl) free(response->finalUrl);
    free(response);
}

static FetchResponse* internalFetch(ParsedUrl* p, bool followRedirects, int depth);

FetchResponse* fetchUrl(const char* url, bool followRedirects) {
    if (!openSocketLibrary()) return NULL;
    ParsedUrl* p = parseUrl(url);
    if (!p) return NULL;

    FetchResponse* res = internalFetch(p, followRedirects, 0);
    if (res && !res->finalUrl) {
        res->finalUrl = strdup(url);
    }

    freeParsedUrl(p);
    return res;
}

static FetchResponse* internalFetch(ParsedUrl* p, bool followRedirects, int depth) {
    if (depth > 10) return NULL; // Limit redirects

    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    bool isHttps = (strcmp(p->scheme, "https") == 0);

    if (isHttps) {
        if (!AmiSSLBase) {
            fprintf(stderr, "HTTPS requested but AmiSSL not available.\n");
            return NULL;
        }
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) return NULL;
        SSL_CTX_set_default_verify_paths(ctx);
    }

    struct hostent* he = gethostbyname(p->host);
    if (!he) {
        if (ctx) SSL_CTX_free(ctx);
        return NULL;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        if (ctx) SSL_CTX_free(ctx);
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p->port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        CloseSocket(sock);
        if (ctx) SSL_CTX_free(ctx);
        return NULL;
    }

    if (isHttps) {
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);
        SSL_set_tlsext_host_name(ssl, p->host);
        if (SSL_connect(ssl) <= 0) {
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            CloseSocket(sock);
            return NULL;
        }
    }

    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "User-Agent: AmigaFetch/1.0\r\n\r\n",
             p->path, p->host);

    if (isHttps) {
        SSL_write(ssl, request, strlen(request));
    } else {
        send(sock, request, strlen(request), 0);
    }

    // Naive response reading
    char* buffer = malloc(4096);
    size_t totalRead = 0;
    size_t bufferSize = 4096;
    int n;

    while (1) {
        if (isHttps) {
            n = SSL_read(ssl, buffer + totalRead, bufferSize - totalRead - 1);
        } else {
            n = recv(sock, buffer + totalRead, bufferSize - totalRead - 1, 0);
        }

        if (n <= 0) break;

        totalRead += n;
        if (totalRead >= bufferSize - 1) {
            bufferSize *= 2;
            buffer = realloc(buffer, bufferSize);
        }
    }
    buffer[totalRead] = '\0';

    if (isHttps) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
    CloseSocket(sock);

    FetchResponse* res = calloc(1, sizeof(FetchResponse));
    res->headers = dictCreate();

    char* headerEnd = strstr(buffer, "\r\n\r\n");
    if (!headerEnd) {
        headerEnd = strstr(buffer, "\n\n"); // Fallback for some servers
    }
    if (!headerEnd) {
        freeResponse(res);
        free(buffer);
        return NULL;
    }
    size_t headerLen;
    char* bodyStart;
    if (strncmp(headerEnd, "\r\n\r\n", 4) == 0) {
        *headerEnd = '\0';
        bodyStart = headerEnd + 4;
    } else {
        *headerEnd = '\0';
        bodyStart = headerEnd + 2;
    }
    res->bodySize = totalRead - (bodyStart - buffer);
    char* body = malloc(res->bodySize + 1);
    if (body) {
        memcpy(body, bodyStart, res->bodySize);
        body[res->bodySize] = '\0';
        res->body = body;
    }

    // Parse status line
    char* line = strtok(buffer, "\r\n");
    if (line && strncmp(line, "HTTP/", 5) == 0) {
        char* statusPtr = strchr(line, ' ');
        if (statusPtr) res->statusCode = atoi(statusPtr + 1);
    }

    // Parse headers
    while ((line = strtok(NULL, "\r\n"))) {
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* key = line;
            char* val = colon + 1;
            while (*val == ' ') val++;
            dictSet(res->headers, strdup(key), strdup(val));
        }
    }

    free(buffer);

    if (followRedirects && (res->statusCode >= 300 && res->statusCode < 400)) {
        char* location = dictGet(res->headers, "Location");
        if (location) {
            ParsedUrl* nextP = parseUrl(location);
            if (nextP) {
                // Handle relative redirects
                if (!nextP->scheme) {
                    free(nextP->scheme);
                    nextP->scheme = strdup(p->scheme);
                    free(nextP->host);
                    nextP->host = strdup(p->host);
                    nextP->port = p->port;
                }
                FetchResponse* nextRes = internalFetch(nextP, followRedirects, depth + 1);
                if (nextRes) {
                    freeResponse(res);
                    freeParsedUrl(nextP);
                    return nextRes;
                }
                freeParsedUrl(nextP);
            }
        }
    }

    return res;
}
