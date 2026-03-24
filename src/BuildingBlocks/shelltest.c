#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"
#include "../AppSupport/collections.h"
#include "../AppSupport/filetools.h"

void printNodeData(listElement* element) {
    printf("Data: %s\n", (char*)element->data);
}

int compareStringNodes(listElement* e1, listElement* e2) {
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

void testSortedInsert() {
    printf("Testing sorted insert\n");
    linkedList* list = listCreate();
    list->compare = compareStringNodes;

    listAddElement(list, "C - Middle");
    listAddElement(list, "A - First");
    listAddElement(list, "E - Last");
    listAddElement(list, "B - Second");
    listAddElement(list, "D - Fourth");

    printf("List length (should be 5): %d\n", list->length);
    listForeach(list, printNodeData);

    listDispose(list);
    printf("\n");
}

void printDictElement(dictionaryElement* element) {
    printf("Key: %s, Value: %s (Hash: %u)\n", (char*)element->key, (char*)element->data, element->hash);
}

void testDictionary() {
    printf("Testing dictionary\n");
    dictionary* dict = dictCreate();

    dictSet(dict, "Name", "André");
    dictSet(dict, "Location", "Germany");
    dictSet(dict, "Project", "Amiga Collections");
    dictSet(dict, "Status", "Working");

    printf("Dictionary length: %d\n", dict->length);
    printf("Foreach:\n");
    dictForeach(dict, printDictElement);

    char* name = (char*)dictGet(dict, "Name");
    printf("Get Name: %s\n", name ? name : "Not found");

    char* unknown = (char*)dictGet(dict, "Unknown");
    printf("Get Unknown: %s\n", unknown ? unknown : "Not found");

    printf("Updating Name\n");
    dictSet(dict, "Name", "André Gewert");
    printf("Get Name: %s\n", (char*)dictGet(dict, "Name"));

    dictDispose(dict);
    printf("\n");
}

void testFileTools() {
    printf("Testing file tools\n");
    STRPTR tempFile = createTempFile("T:", "testfile_");
    if (tempFile != NULL) {
        printf("Created unique temp file: %s\n", tempFile);
        free(tempFile);
    } else {
        printf("Failed to create unique temp file\n");
    }

    STRPTR tempDir = createTempDir("T:", "testdir_");
    if (tempDir != NULL) {
        printf("Created unique temp directory: %s\n", tempDir);
        // On AmigaOS, we'd need DeleteFile or DeleteDir, but for now let's just free the path string
        free(tempDir);
    } else {
        printf("Failed to create unique temp directory\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    testEnvironment();
    testLinkedList();
    testSortedInsert();
    testDictionary();
    testFileTools();
    printf("Exit\n");
    exit(EXIT_SUCCESS);
}
