#include <stdlib.h>
#include <stdio.h>
#include <clib/all_protos.h>
#include <workbench/startup.h>

int main(int argc, char* argv[]) {
    if (argc == 0) {
        printf("Started from workbench\n");
        struct WBStartup* wbinfo = (struct WBStartup*)argv;
        printf(wbinfo->sm_ToolWindow);
    } else {
        printf("Started from cli\n");
        printf("Argument count: %d\n", argc);
        for (int i = 0; i < argc; i++) {
            printf("Argument %d: %s\n", i +1, argv[i]);
        }
    }
    return EXIT_SUCCESS;
}
