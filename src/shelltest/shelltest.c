#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"

int main(int argc, char** argv) {
    printf("Color mode: %d\n", getEnvColorMode());
    STRPTR encoding = getEnvCharset();
    if (encoding != NULL) {
        printf("Charset: '%s'\n", encoding);
        free(encoding);
    } else {
        printf("Could not read charset\n");
    }
    exit(EXIT_SUCCESS);
}
