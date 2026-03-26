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
#include <string.h>
#include <strings.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include "../AppSupport/shell.h"

static int isInfoFile(const char* name) {
    size_t len = strlen(name);
    if (len >= 5) {
        return strcasecmp(name + (len - 5), ".info") == 0;
    }
    return 0;
}

typedef struct {
    char name[108];
    LONG type;
    LONG protection;
    LONG size;
    struct DateStamp date;
} DirEntry;

static const char* getColorForEntry(const char* name, LONG type, LONG protection) {
    if (type == ST_LINKDIR || type == ST_LINKFILE) {
        return SHELL_FG_BRIGHT_MAGENTA;
    }

    if (type > 0) {
        return SHELL_FG_BRIGHT_BLUE;
    }

    if (isInfoFile(name)) {
        return SHELL_FG_BRIGHT_BLACK;
    }

    if (!(protection & FIBF_EXECUTE)) {
        return SHELL_FG_BRIGHT_GREEN;
    }

    const char* ext = strrchr(name, '.');
    if (ext) {
        if (strcasecmp(ext, ".lha") == 0 || strcasecmp(ext, ".zip") == 0 || 
            strcasecmp(ext, ".lzx") == 0 || strcasecmp(ext, ".tar") == 0 ||
            strcasecmp(ext, ".gz") == 0) {
            return SHELL_FG_BRIGHT_YELLOW;
        }
        if (strcasecmp(ext, ".iff") == 0 || strcasecmp(ext, ".ilbm") == 0 || 
            strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".png") == 0 ||
            strcasecmp(ext, ".gif") == 0 || strcasecmp(ext, ".bmp") == 0) {
            return SHELL_FG_BRIGHT_MAGENTA;
        }
        if (strcasecmp(ext, ".c") == 0 || strcasecmp(ext, ".h") == 0 || 
            strcasecmp(ext, ".txt") == 0 || strcasecmp(ext, ".doc") == 0 ||
            strcasecmp(ext, ".readme") == 0) {
            return SHELL_FG_BRIGHT_CYAN;
        }
    }

    return SHELL_COLOR_RESET;
}

static void formatProtection(LONG prot, char* buf) {
    buf[0] = (prot & FIBF_HOLD) ? 'h' : '-';
    buf[1] = (prot & FIBF_SCRIPT) ? 's' : '-';
    buf[2] = (prot & FIBF_PURE) ? 'p' : '-';
    buf[3] = (prot & FIBF_ARCHIVE) ? 'a' : '-';
    buf[4] = (prot & FIBF_READ) ? '-' : 'r';
    buf[5] = (prot & FIBF_WRITE) ? '-' : 'w';
    buf[6] = (prot & FIBF_EXECUTE) ? '-' : 'e';
    buf[7] = (prot & FIBF_DELETE) ? '-' : 'd';
    buf[8] = '\0';
}

static void formatDate(struct DateStamp* ds, char* outBuf) {
    char day[LEN_DATSTRING];
    char date[LEN_DATSTRING];
    char time[LEN_DATSTRING];
    struct DateTime dt;
    dt.dat_Stamp = *ds;
    dt.dat_Format = FORMAT_DOS;
    dt.dat_Flags = 0;
    dt.dat_StrDay = day;
    dt.dat_StrDate = date;
    dt.dat_StrTime = time;
    if (DateToStr(&dt)) {
        sprintf(outBuf, "%s %s", date, time);
    } else {
        strcpy(outBuf, "invalid date");
    }
}

static int compareEntries(const void* a, const void* b) {
    const DirEntry* e1 = (const DirEntry*)a;
    const DirEntry* e2 = (const DirEntry*)b;

    // Directories first
    if (e1->type > 0 && e2->type <= 0) return -1;
    if (e1->type <= 0 && e2->type > 0) return 1;

    // Then alphabetical (case-insensitive)
    return strcasecmp(e1->name, e2->name);
}

int main(int argc, char** argv) {
    STRPTR template = (STRPTR)"DIR/M,NOINFO/S";
    struct {
        char** dirs;
        LONG noinfo;
    } args = {NULL, 0};

    struct RDArgs* rdargs = ReadArgs(template, (intptr_t*)&args, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), (STRPTR)"cldir");
        return EXIT_FAILURE;
    }

    struct FileInfoBlock* fib = AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        PrintFault(IoErr(), (STRPTR)"cldir: failed to allocate FileInfoBlock");
        FreeArgs(rdargs);
        return EXIT_FAILURE;
    }

    char** dir_ptr = args.dirs;
    if (!dir_ptr || !*dir_ptr) {
        // Use current directory if no DIR specified
        static char* current_dir[] = {"", NULL};
        dir_ptr = current_dir;
    }

    while (dir_ptr && *dir_ptr) {
        char* dir_path = *dir_ptr;
        BPTR lock = Lock((STRPTR)dir_path, SHARED_LOCK);
        if (lock) {
            if (Examine(lock, fib)) {
                if (fib->fib_DirEntryType >= 0) {
                    // It's a directory, list its content
                    if (args.dirs && args.dirs[1]) {
                        printf("%s:\n", dir_path[0] ? dir_path : ".");
                    }

                    size_t count = 0;
                    size_t capacity = 64;
                    DirEntry* entries = malloc(capacity * sizeof(DirEntry));
                    if (entries) {
                        while (ExNext(lock, fib)) {
                            if (args.noinfo && isInfoFile(fib->fib_FileName)) {
                                continue;
                            }

                            if (count >= capacity) {
                                capacity *= 2;
                                DirEntry* new_entries = realloc(entries, capacity * sizeof(DirEntry));
                                if (!new_entries) {
                                    fprintf(stderr, "cldir: out of memory during realloc\n");
                                    break;
                                }
                                entries = new_entries;
                            }
                            strncpy(entries[count].name, fib->fib_FileName, sizeof(entries[count].name));
                            entries[count].name[sizeof(entries[count].name) - 1] = '\0';
                            entries[count].type = fib->fib_DirEntryType;
                            entries[count].protection = fib->fib_Protection;
                            entries[count].size = fib->fib_Size;
                            entries[count].date = fib->fib_Date;
                            count++;
                        }

                        if (count > 0) {
                            qsort(entries, count, sizeof(DirEntry), compareEntries);

                            for (size_t i = 0; i < count; i++) {
                                const char* color = getColorForEntry(entries[i].name, entries[i].type, entries[i].protection);
                                char protBuf[9];
                                char dateStr[64];
                                formatProtection(entries[i].protection, protBuf);
                                formatDate(&entries[i].date, dateStr);

                                shellPrintf(color, "%-32s", entries[i].name);
                                printf(" %s", protBuf);
                                if (entries[i].type > 0) {
                                    shellPrintf(SHELL_FG_BLUE, "      (dir)");
                                } else {
                                    printf(" %10ld", entries[i].size);
                                }
                                printf(" %s\n", dateStr);
                            }
                        }
                        free(entries);
                    } else {
                        fprintf(stderr, "cldir: out of memory\n");
                    }
                    if (dir_ptr[1]) printf("\n");
                } else {
                    // It's a file, just show it
                    if (args.noinfo && isInfoFile(fib->fib_FileName)) {
                        // skip
                    } else {
                        const char* color = getColorForEntry(fib->fib_FileName, fib->fib_DirEntryType, fib->fib_Protection);
                        char protBuf[9];
                        char dateStr[64];
                        formatProtection(fib->fib_Protection, protBuf);
                        formatDate(&fib->fib_Date, dateStr);

                        shellPrintf(color, "%-32s", fib->fib_FileName);
                        printf(" %s %10ld %s\n", protBuf, fib->fib_Size, dateStr);
                    }
                }
            } else {
                PrintFault(IoErr(), (STRPTR)dir_path);
            }
            UnLock(lock);
        } else {
            PrintFault(IoErr(), (STRPTR)dir_path);
        }
        dir_ptr++;
    }

    FreeDosObject(DOS_FIB, fib);
    FreeArgs(rdargs);
    return EXIT_SUCCESS;
}
