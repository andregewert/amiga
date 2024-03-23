#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"
#include "../AppSupport/collections.h"

void printNodeData(listElement* element) {
    printf("Data: %s\n", (char*)element->data);
}

int main(int argc, char** argv) {
    printf("Color mode: %d\n", getEnvColorMode());
    STRPTR encoding = getEnvCharset();
    if (encoding != NULL) {
        printf("Charset: '%s'\n", encoding);
        free(encoding);
    } else {
        printf("Could not read charset\n");
    }

    linkedList* list = listCreate();
    listElement* firstNode = listAppendElement(list, "Knoten 1");
    printf("Pointer is %p\n", firstNode);

    listElement* secondNode = listAppendElement(list, "Hallo Welt");
    printf("Second pointer is %p\n", secondNode);

    listForeach(list, &printNodeData);

    listDispose(list);

    exit(EXIT_SUCCESS);
}
