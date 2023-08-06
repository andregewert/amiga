#include <stdio.h>
#include <stdlib.h>
#include <clib/lowlevel_protos.h>
#include <clib/dos_protos.h>

char* langCodes[] = {
    "",
    "en",
    "en",
    "de",
    "fr",
    "es",
    "it",
    "pt",
    "da",
    "nl",
    "no",
    "fi",
    "sv",
    "ja",
    "zh",
    "ar",
    "el",
    "he",
    "ko"
};

char* getLanguageSelectionCode() {
    UBYTE lang = GetLanguageSelection();
    if (lang < sizeof(langCodes) / sizeof(langCodes[0])) {
        return langCodes[lang];
    }
    return NULL;
}

int main(int argc, char** argv) {
    //UBYTE lang = GetLanguageSelection();
    //printf("Language selection: %d\n", lang);
    //printf("Language code is: %s\n", langCodes[lang]);
    //printf("Sizeof langCodes: %d\n", sizeof(langCodes) /sizeof(langCodes[0]));
    printf("Language code via lowlevel library: %s\n", getLanguageSelectionCode());

    // Getting language via environment var
    STRPTR dummy[64] = "";
    if (GetVar("language", dummy, 64, 0) > 0) {
        printf("Language via env: %s\n", dummy);
    }
    exit(EXIT_SUCCESS);
}