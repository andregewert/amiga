#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/all_protos.h>

int main(int argc, char** argv) {
    BPTR lock;
    struct FileInfoBlock* fileInfoBlock;

    fileInfoBlock = AllocMem(sizeof(struct FileInfoBlock), MEMF_PUBLIC | MEMF_CLEAR);
    if (fileInfoBlock == NULL) {
        printf("Could not allocate memory");
        return EXIT_FAILURE;
    }

    lock = Lock("s:", SHARED_LOCK);

    if (lock == 0L) {
        FreeMem(fileInfoBlock, sizeof(struct FileInfoBlock));
        printf("Coul not lock directory s:");
        return EXIT_FAILURE;
    }

    if (Examine(lock, fileInfoBlock) == 0L) {
        FreeMem(fileInfoBlock, sizeof(struct FileInfoBlock));
        UnLock(lock);
        printf("Could not examine directory s:");
        return EXIT_FAILURE;
    }

    printf("File name: %s\n", FileInfoBlock->fib_FileName);
    while (ExNext(lock, FileInfoBlock) != FALSE) {
        printf("File name: %s\n", FileInfoBlock->fib_FileName);
        printf("Entry type: %d\n", FileInfoBlock->fib_DirEntryType);
    }

    UnLock(lock);
    FreeMem(fileInfoBlock, sizeof(struct FileInfoBlock));

    return EXIT_SUCCESS;
}
