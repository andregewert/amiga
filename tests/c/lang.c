#include <stdio.h>
#include <clib/lowlevel_protos.h>

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
    printf("Language code: %s\n\n", getLanguageSelectionCode());
    return 0;
}