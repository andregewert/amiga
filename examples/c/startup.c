#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <exec/types.h>
#include <clib/all_protos.h>
#include <workbench/startup.h>
#include <dos/rdargs.h>

int main(int argc, char* argv[]) {
    struct RDArgs* rdArgs;
    LONG cmdLineOptions[1];
    CONST_STRPTR cmdLineTemplate = "File/A,Names/M/A";

    if (argc == 0) {
        printf("Started from workbench\n");
        struct WBStartup* wbinfo = (struct WBStartup*)argv;
        //printf(wbinfo->sm_ToolWindow);

        // TODO Parse workbench arguments

    } else {
        printf("Started from cli\n");
        
        // Manually parsing command line options
        printf("Argument count: %d\n", argc);
        for (int i = 0; i < argc; i++) {
            printf("Argument %d: %s\n", i +1, argv[i]);
        }

        // Amiga way of parsing command line options
        rdArgs = ReadArgs(cmdLineTemplate, cmdLineOptions, NULL);
        if (rdArgs == NULL) {
            PrintFault(IoErr(), "startup");
            return EXIT_FAILURE;
        } else {
            // File option -> single string
            printf("File: %s\n", cmdLineOptions[0]);

            // Names option -> multiple strings
            STRPTR* arr = (STRPTR*)cmdLineOptions[1];
            do {
                printf("Argument: %s\n", *arr);
            } while (*(++arr));
        }
    }

    if (rdArgs != NULL) {
        FreeArgs(rdArgs);
    }
    return EXIT_SUCCESS;
}
