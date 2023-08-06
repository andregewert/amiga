#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <

/**
 * Benötigte Funktionen:
 * - Color cycling
 * - Bitmaps und Paletten aus ILBM-Dateien laden
 * - Color cycling aus ILBM-Dateien laden
 * - Color cycling example: Zwischengrafiken bei PANG! sind schönes Beispiel!
 * - Rainbow copper table
 * - Scrollable rainbow!
 */

#pragma region Prototypes

int loadPaletteFromIlbmFile(CONST_STRPTR);

#pragma endregion

typedef struct iffBMHD {
    UWORD width, height;
    WORD x, y;
    UBYTE depth, mask, compression, padbyte;
    UWORD transcolor;
    UBYTE xaspect, yaspect;
    WORD screenWidth, screenHeight;
} iffBMHD;

int main(int argc, char* argv[]) {
    printf("Hallo Welt!\n");
    int success = loadPaletteFromIlbmFile("media/Cheetah.iff");
    printf("Success: %d\n\n", success);
}

int fileExpectString(BPTR file, CONST_STRPTR string) {
    UBYTE buffer[256];
    memset(buffer, 0, sizeof(buffer));
    WORD len = strlen(string);
    LONG read = Read(file, buffer, len);
    
    printf("String length: %d\n", len);

    if (read != len) {
        printf("Expected %d bytes, but read %d bytes\n", len, read);
    }

    if (strcmp(buffer, string)) {
        printf("Did not read the expected string %s, but %s\n", string, buffer);
        return 1;
    }

    printf("Found string %s\n", string);
    return 0;
}

int loadPaletteFromIlbmFile(CONST_STRPTR filename) {
    BPTR file = Open(filename, MODE_OLDFILE);
    
    //UBYTE buffer[4];
    //ULONG chunkLength;

    if (file == 0) {
        printf("Could not open file: %s", filename);
        return 0;
    }

    Seek(file, 0, OFFSET_END);
    LONG filesize = Seek(file, 0, OFFSET_END);
    printf("Filesize with seek: %d\n", filesize);
    
    if (filesize < 12) {
        printf("Filesize is only %d bytes\n", filesize);
        Close(file);
        return 0;
    }

    Seek(file, 0, OFFSET_BEGINNING);
    fileExpectString(file, "FORM");
    Seek(file, 4, OFFSET_CURRENT);
    fileExpectString(file, "ILBMBMHD");


    /*
    Read(file, buffer, 4);
    printf("Another four bytes: %s\n", buffer);
    Read(file, &chunkLength, 4);
    printf("Another four bytes: %d\n", chunkLength);
    */

    Close(file);
    return 1;
}



int loadBitmapFromIlbmFile(CONST_STRPTR filename) {
    BPTR file = Open(filename, MODE_OLDFILE);
    if (file == 0) return 0;

    Close(file);
    return 1;
}

int parseIffChunk(BPTR filehandle) {
    UBYTE buffer[100];
    Read(filehandle, buffer, 4);
    return 1;
}