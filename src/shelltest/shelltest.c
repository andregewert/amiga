#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"
#include "../AppSupport/collections.h"

void printNodeData(listElement* element) {
    printf("Data: %s\n", (char*)element->data);
}

int8_t compareStringNodes(listElement* e1, listElement* e2) {
    //return strncasecmp(e1->data, e1->data, sizeof(char));
    return strcmp(e1->data, e2->data);
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
    printf("List length is: %d\n", list->length);

    listElement* secondNode = listAppendElement(list, "Hallo Welt");
    printf("Second pointer is %p\n", secondNode);
    printf("List length is: %d\n", list->length);

    listForeach(list, &printNodeData);

    printf("Node at 0: %p\n", listGetElementAt(list, 0));
    printf("Node at 1: %p\n", listGetElementAt(list, 1));

    //listSort(list, compareStringNodes);
    printf("Remove element at index 0\n");
    listRemoveElementAt(list, 0);
    printf("List length is: %d\n", list->length);
    listForeach(list, &printNodeData);

    printf("Remove element at index 0\n");
    listRemoveElementAt(list, 0);
    printf("List length is: %d\n", list->length);
    listForeach(list, &printNodeData);

    listDispose(list);

    exit(EXIT_SUCCESS);
}
