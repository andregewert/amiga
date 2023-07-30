#include <stdlib.h>
#include <stdio.h>
#include <clib/all_protos.h>

const char* CON_RED = "\033[31m";
const char* CON_RESET = "\033[0m";

int main(int argc, char** argv) {
    // Foreground colors
    for (int i = 0; i <= 7; i++) {
        printf("\033[%imForeground%s\n", 30 + i, CON_RESET);
    }
    for (int i = 0; i <= 7; i++) {
        printf("\033[%imForeground%s\n", 90 + i, CON_RESET);
    }

    // Background colors
    for (int i = 0; i <= 7; i++) {
        printf("\033[%imBackground%s\n", 40 + i, CON_RESET);
    }
    for (int i = 0; i <= 7; i++) {
        printf("\033[%imBackground%s\n", 100 + i, CON_RESET);
    }

    return EXIT_SUCCESS;
}
