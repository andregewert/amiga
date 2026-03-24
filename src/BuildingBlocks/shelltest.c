#include <stdio.h>
#include <stdlib.h>
#include "../AppSupport/environment.h"
#include "../AppSupport/collections.h"
#include "../AppSupport/filetools.h"
#include "../AppSupport/shell.h"

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

    printf("Unsetting Location\n");
    dictUnset(dict, "Location");
    printf("Get Location: %s\n", (char*)dictGet(dict, "Location") ? (char*)dictGet(dict, "Location") : "Not found");
    printf("Dictionary length: %d\n", dict->length);

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

int compareTreeNodes(treeNode* n1, treeNode* n2) {
    return strcmp((char*)n1->data, (char*)n2->data);
}

void printTreeNode(treeNode* node) {
    printf("%s ", (char*)node->data);
}

void testBinaryTree() {
    printf("Testing binary tree\n");
    binaryTree* tree = treeCreate(compareTreeNodes);

    treeAddElement(tree, "M");
    treeAddElement(tree, "G");
    treeAddElement(tree, "T");
    treeAddElement(tree, "D");
    treeAddElement(tree, "J");
    treeAddElement(tree, "P");
    treeAddElement(tree, "X");

    printf("Tree length (should be 7): %d\n", tree->length);

    printf("In-order traversal: ");
    treeInOrder(tree, printTreeNode);
    printf("\n");

    printf("Pre-order traversal: ");
    treePreOrder(tree, printTreeNode);
    printf("\n");

    printf("Post-order traversal: ");
    treePostOrder(tree, printTreeNode);
    printf("\n");

    treeNode searchNode;
    searchNode.data = "J";
    treeNode* found = treeFindElement(tree, &searchNode);
    printf("Searching for 'J': %s\n", found ? (char*)found->data : "Not found");

    searchNode.data = "Z";
    found = treeFindElement(tree, &searchNode);
    printf("Searching for 'Z': %s\n", found ? (char*)found->data : "Not found");

    printf("Removing 'M' (root with two children): ");
    searchNode.data = "M";
    treeRemoveElement(tree, &searchNode);
    treeInOrder(tree, printTreeNode);
    printf("(Length: %d)\n", tree->length);

    printf("Removing 'D' (leaf node): ");
    searchNode.data = "D";
    treeRemoveElement(tree, &searchNode);
    treeInOrder(tree, printTreeNode);
    printf("(Length: %d)\n", tree->length);

    printf("Removing 'G' (node with one child): ");
    searchNode.data = "G";
    treeRemoveElement(tree, &searchNode);
    treeInOrder(tree, printTreeNode);
    printf("(Length: %d)\n", tree->length);

    printf("Adding nodes back for subtree removal test: ");
    treeAddElement(tree, "G");
    treeAddElement(tree, "D");
    treeInOrder(tree, printTreeNode);
    printf("(Length: %d)\n", tree->length);

    printf("Removing subtree 'G' (should remove 'G' and 'D'): ");
    searchNode.data = "G";
    treeRemoveSubtree(tree, &searchNode);
    treeInOrder(tree, printTreeNode);
    printf("(Length: %d)\n", tree->length);

    treeDispose(tree);
    printf("\n");
}

void testIniParser() {
    printf("Testing INI parser\n");
    dictionary* dict = dictFromIni("test.ini");
    if (dict == NULL) {
        printf("Could not read test.ini\n");
        return;
    }

    printf("INI Dictionary length: %d\n", dict->length);
    dictForeach(dict, printDictElement);

    char* val1 = (char*)dictGet(dict, "Section1.key1");
    printf("Section1.key1: %s\n", val1 ? val1 : "Not found");

    char* val4 = (char*)dictGet(dict, "Section2.key4");
    printf("Section2.key4: [%s]\n", val4 ? val4 : "Not found");

    // Manually free duplicated values before disposing
    listElement* current = dict->list->firstElement;
    while (current != NULL) {
        dictionaryElement* element = (dictionaryElement*)current->data;
        if (element->data != NULL) {
            free(element->data);
        }
        current = current->nextElement;
    }

    dictDispose(dict);
    printf("\n");
}

void testShellColors() {
    printf("Testing shell colors (Full Colors)\n");
    setEnvColorModeOverride(SHELL_ANSI_COLORS);
    shellPrintf(SHELL_FG_RED, "This is red text\n");
    shellPrintf(SHELL_FG_GREEN, "This is green text\n");
    shellPrintf(SHELL_FG_BLUE, "This is blue text\n");
    shellPrintf(SHELL_FG_YELLOW, "This is yellow text\n");
    shellPrintf(SHELL_FG_CYAN, "This is cyan text\n");
    shellPrintf(SHELL_FG_MAGENTA, "This is magenta text\n");
    shellPrintf(SHELL_FG_WHITE, "This is white text\n");
    
    printf("\nTesting shell colors (Four Colors Mapping)\n");
    setEnvColorModeOverride(SHELL_FOUR_COLORS);
    shellPrintf(SHELL_FG_RED, "Red (mapped to Blue?)\n");
    shellPrintf(SHELL_FG_GREEN, "Green (mapped to Blue?)\n");
    shellPrintf(SHELL_FG_BLUE, "Blue (mapped to Blue)\n");
    shellPrintf(SHELL_FG_YELLOW, "Yellow (mapped to White?)\n");
    shellPrintf(SHELL_FG_CYAN, "Cyan (mapped to White?)\n");
    shellPrintf(SHELL_FG_MAGENTA, "Magenta (mapped to Blue?)\n");
    shellPrintf(SHELL_FG_WHITE, "White (mapped to White)\n");
    shellPrintf(SHELL_FG_BLACK, "Black (mapped to Black)\n");

    setEnvColorModeOverride(-1); // Reset override
    
    printf("\nNormal usage:\n");
    shellSetColor(SHELL_FG_YELLOW);
    printf("This should be yellow (if env supports it)\n");
    shellResetColor();
    printf("Back to normal\n");

    shellPrintf(SHELL_BG_RED, "Red background\n");
    shellPrintf(SHELL_FG_WHITE SHELL_BG_BLUE, "White on blue\n");
    printf("\n");
}

int main(int argc, char** argv) {
    testEnvironment();
    testLinkedList();
    testSortedInsert();
    testDictionary();
    testFileTools();
    testBinaryTree();
    testIniParser();
    testShellColors();
    printf("Exit\n");
    exit(EXIT_SUCCESS);
}
