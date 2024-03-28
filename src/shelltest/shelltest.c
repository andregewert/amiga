#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"
#include "../AppSupport/collections.h"

void printNodeData(listElement* element) {
    printf("Data: %s\n", (char*)element->data);
}

int8_t compareStringNodes(listElement* e1, listElement* e2) {
    return strcmp(e1->data, e2->data);
}

void testEnvironment() {
    printf("Testing environment functions\n");
    printf("Color mode: %d\n", getEnvColorMode());
    STRPTR encoding = getEnvCharset();
    if (encoding != NULL) {
        printf("Charset: '%s'\n", encoding);
        free(encoding);
    } else {
        printf("Could not read charset\n");
    }
    printf("\n");
}

void testLinkedList() {
    listElement* dummy;
    linkedList* list = listCreate();

    listElement* firstNode = listAppendElement(list, "Knoten 1");
    printf("Pointer is %p\n", firstNode);
    printf("List length is: %d\n", list->length);

    listElement* secondNode = listAppendElement(list, "Hallo Welt");
    printf("Second pointer is %p\n", secondNode);
    printf("List length is: %d\n", list->length);

    listElement* thirdNode = listAppendElement(list, "Aber Hallo");
    printf("Third pointer is %p\n", thirdNode);
    printf("List length is: %d\n", list->length);

    listElement* fourthNode = listAppendElement(list, "K ... what?");
    printf("Third pointer is %p\n", fourthNode);
    printf("List length is: %d\n", list->length);

    printf("Foreach:\n");
    listForeach(list, printNodeData);

    printf("Sorting list\n");
    listSort(list, compareStringNodes);

    printf("Foreach:\n");
    listForeach(list, printNodeData);

    printf("Remove element at index 0\n");
    dummy = listRemoveElementAt(list, 0);
    dummy->data = NULL;
    free(dummy);
    printf("List length is: %d\n", list->length);
    listForeach(list, printNodeData);

    printf("Remove element at index 0\n");
    dummy = listRemoveElementAt(list, 0);
    dummy->data = NULL;
    free(dummy);
    printf("List length is: %d\n", list->length);
    listForeach(list, printNodeData);

    printf("Disposing list\n");
    printf("\n");
    listDispose(list);
}

int main(int argc, char** argv) {
    testEnvironment();
    testLinkedList();
    printf("Exit\n");
    exit(EXIT_SUCCESS);
}
