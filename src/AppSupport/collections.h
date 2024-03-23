#ifndef APPSUPPORT_COLLECTIONS_H
#define APPSUPPORT_COLLECTIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <exec/types.h>

// <editor-fold desc="Structures">

typedef struct listElement listElement;

/**
 * Describes a member of a linked list.
 */
struct listElement {
    /**
     * A pointer to the actual data.
     */
    void* data;

    /**
     * A pointer to the next list element or NULL.
     */
    listElement* nextElement;
};

/**
 * This struct describes the base of a linked list.
 * It includes the current number of linked elements and a pointer
 * to the first element (or NULL).
 */
typedef struct {
    /**
     * Pointer to the first elements or NULL.
     */
    listElement* firstElement;

    /**
     * Current number of linked elements.
     */
    uint32_t length;
} linkedList;

/**
 * Callback functions for linked list elements.
 * This type is used for example by the foreach construct.
 */
typedef void(*listElementCallback)(listElement*);

// </editor-fold>


// <editor-fold desc="Functions">

/**
 * Creates an empty linked list.
 * @return Pointer to the created list structure.
 */
linkedList* listCreate();

/**
 * Creates a new linked list element and references the given data.
 * @param data Pointer to the element's data
 * @return Pointer to the newly created list element struct.
 */
listElement* listCreateElement(void* data);

/**
 * Creates a new linked list element and appends it to the given list.
 * @param list Pointer to the linked list structure.
 * @param data Pointer to the element's data.
 * @return Pointer to the newly created list element.
 */
listElement* listAppendElement(linkedList* list, void* data);

/**
 * Calls a functions for every element of the given list.
 * @param list Pointer to the linked list structure.
 * @param iterate Pointer to a callback function.
 */
void listForeach(linkedList* list, listElementCallback callback);

/**
 * Disposes a linked list.
 * Note that the linked elements have to be cleaned up before
 * disposing the list.
 * @param list Pointer to the linked list stucture.
 */
void listDispose(linkedList* list);

//struct listElement* listElementAt(struct listElement* list, uint32_t index);
//uint32_t listSize(struct listElement* list);
//void listSort(struct listElement* list, int (*compare)(void*, void*));

// </editor-fold>

#endif
