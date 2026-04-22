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
#include <proto/dos.h>
#include "../AppSupport/archvfs.h"
#include "../AppSupport/testutils.h"
#include "../AppSupport/filetools.h"

void createTestFile(const char* filename, const char* content) {
    BPTR file = Open((STRPTR)filename, MODE_NEWFILE);
    if (file) {
        Write(file, (void*)content, strlen(content));
        Close(file);
    } else {
        printf("Failed to create file %s\n", filename);
    }
}

int main() {
    testInit("ArchVFS Tests");

    /*
    ArchVfs* avfs = archvfsCreate("RAM:");
    ArchVfsDir* avfsDir = archvfsOpenDir(avfs, "/");
    ArchVfsDirEntry dirEntry;
    while (archvfsReadDir(avfsDir, &dirEntry)) {
        printf("Dir entry: '%s' (type: %d)\n", dirEntry.name, dirEntry.type);
    }
    return 0;
    */

    // Setup: Create a real directory and an archive
    const char* realDir = "T:vfs_real";
    const char* archName = "T:vfs_test.arch";
    const char* fileReal = "T:vfs_real/real.txt";
    const char* contentReal = "This is a real file.";
    const char* contentArch = "This is an archived file.";
    
    BPTR d = CreateDir((STRPTR)realDir);
    if (d) UnLock(d);
    if (!d) {
        printf("Failed to create directory %s\n", realDir);
        return 1;
    }

    createTestFile(fileReal, contentReal);

    // Create archive
    Archive* arch = archiveOpen(archName);
    const char* tempFile = "T:vfs_temp.txt";
    createTestFile(tempFile, contentArch);
    archiveAddFile(arch, tempFile, "archived.txt");
    archiveClose(arch);
    DeleteFile((STRPTR)tempFile);

    // Initialize VFS
    char searchPath[512];
    snprintf(searchPath, sizeof(searchPath), "%s;%s", realDir, archName);
    ArchVfs* vfs = archvfsCreate(searchPath);
    ASSERT_NOT_NULL(vfs);

    // Test: Existence and Type
    ASSERT_TRUE(archvfsExists(vfs, "/real.txt"));
    ASSERT_TRUE(archvfsExists(vfs, "/archived.txt"));
    ASSERT_INT_EQ(ARCHVFS_TYPE_FILE, archvfsGetType(vfs, "/real.txt"));
    ASSERT_INT_EQ(ARCHVFS_TYPE_FILE, archvfsGetType(vfs, "/archived.txt"));
    ASSERT_FALSE(archvfsExists(vfs, "/nonexistent.txt"));

    // Test: Reading real file
    ArchVfsFile* f = archvfsOpenFile(vfs, "/real.txt", "r");
    ASSERT_NOT_NULL(f);
    char buffer[100];
    uint32_t read = archvfsReadFile(f, buffer, sizeof(buffer)-1);
    buffer[read] = '\0';
    ASSERT_STR_EQ(contentReal, buffer);
    archvfsCloseFile(f);

    // Test: Reading archived file
    f = archvfsOpenFile(vfs, "/archived.txt", "r");
    ASSERT_NOT_NULL(f);
    read = archvfsReadFile(f, buffer, sizeof(buffer)-1);
    buffer[read] = '\0';
    ASSERT_STR_EQ(contentArch, buffer);
    archvfsCloseFile(f);

    // Test: Writing new file (should go to first search path: realDir)
    f = archvfsOpenFile(vfs, "/newfile.txt", "w");
    ASSERT_NOT_NULL(f);
    const char* contentNew = "New file content";
    archvfsWriteFile(f, contentNew, strlen(contentNew));
    archvfsCloseFile(f);

    ASSERT_TRUE(archvfsExists(vfs, "/newfile.txt"));
    BPTR checkFile = Open("T:vfs_real/newfile.txt", MODE_OLDFILE);
    ASSERT_TRUE(checkFile != 0);
    if (checkFile) Close(checkFile);

    // Test: Directory listing
    ArchVfsDir* dir = archvfsOpenDir(vfs, "/");
    ASSERT_NOT_NULL(dir);
    ArchVfsDirEntry de;
    BOOL foundReal = FALSE;
    BOOL foundArch = FALSE;
    BOOL foundNew = FALSE;
    while (archvfsReadDir(dir, &de)) {
        if (strcmp(de.name, "real.txt") == 0) foundReal = TRUE;
        if (strcmp(de.name, "archived.txt") == 0) foundArch = TRUE;
        if (strcmp(de.name, "newfile.txt") == 0) foundNew = TRUE;
    }
    if (!foundReal) printf("FAILED: real.txt not found in directory listing\n");
    if (!foundArch) printf("FAILED: archived.txt not found in directory listing\n");
    if (!foundNew) printf("FAILED: newfile.txt not found in directory listing\n");
    ASSERT_TRUE(foundReal);
    ASSERT_TRUE(foundArch);
    ASSERT_TRUE(foundNew);
    archvfsCloseDir(dir);

    // Test: Deletion
    ASSERT_TRUE(archvfsDelete(vfs, "/newfile.txt"));
    ASSERT_FALSE(archvfsExists(vfs, "/newfile.txt"));

    archvfsDispose(vfs);

    // Test: Writing when only archive is in search path
    vfs = archvfsCreate(archName);
    ASSERT_NOT_NULL(vfs);
    f = archvfsOpenFile(vfs, "/new_archive_file.txt", "w");
    ASSERT_NOT_NULL(f);
    const char* archContentNew = "New content in archive";
    archvfsWriteFile(f, archContentNew, strlen(archContentNew));
    archvfsCloseFile(f);

    ASSERT_TRUE(archvfsExists(vfs, "/new_archive_file.txt"));
    
    // Verify by opening again
    f = archvfsOpenFile(vfs, "/new_archive_file.txt", "r");
    ASSERT_NOT_NULL(f);
    read = archvfsReadFile(f, buffer, sizeof(buffer)-1);
    buffer[read] = '\0';
    ASSERT_STR_EQ(archContentNew, buffer);
    archvfsCloseFile(f);

    // Test: Deleting from archive
    ASSERT_TRUE(archvfsDelete(vfs, "/new_archive_file.txt"));
    ASSERT_FALSE(archvfsExists(vfs, "/new_archive_file.txt"));

    // Test: Deleting "directory" from archive
    // Create some files in a virtual subdir in archive
    Archive* arch_update = archiveOpen(archName);
    createTestFile(tempFile, "sub 1");
    archiveAddFile(arch_update, tempFile, "subdir/file1.txt");
    createTestFile(tempFile, "sub 2");
    archiveAddFile(arch_update, tempFile, "subdir/file2.txt");
    archiveClose(arch_update);
    DeleteFile((STRPTR)tempFile);

    // VFS is already using archName
    ASSERT_TRUE(archvfsExists(vfs, "/subdir/file1.txt"));
    ASSERT_TRUE(archvfsExists(vfs, "/subdir/file2.txt"));
    ASSERT_INT_EQ(ARCHVFS_TYPE_DIRECTORY, archvfsGetType(vfs, "/subdir"));

    ASSERT_TRUE(archvfsDelete(vfs, "/subdir"));
    ASSERT_FALSE(archvfsExists(vfs, "/subdir/file1.txt"));
    ASSERT_FALSE(archvfsExists(vfs, "/subdir/file2.txt"));
    ASSERT_FALSE(archvfsExists(vfs, "/subdir"));

    archvfsDispose(vfs);
    
    // Cleanup
    DeleteFile((STRPTR)fileReal);
    DeleteFile((STRPTR)"T:vfs_real/newfile.txt"); // In case delete failed
    DeleteFile((STRPTR)realDir);
    DeleteFile((STRPTR)archName);

    return testSummary();
}
