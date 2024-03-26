/*
 * Copyright (c) 2024 André Gewert <agewert@ubergeek.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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

/**
 * Callback function for comparing two list elements.
 * This callback can be used for sorting linked lists.
 */
typedef int8_t(*listSortCompare)(listElement*, listElement*);

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
 * Returns the element at a specific index or NULL if the requested
 * element does not exist.
 * @param list Pointer to the linked list.
 * @param index Index for the requested element.
 * @return Pointer to the requested element or NULL
 */
listElement* listGetElementAt(linkedList* list, uint32_t index);

/**
 * Removes an element from a linked list and returns a pointer to
 * the removed element.
 * @param list
 * @param index
 * @return
 */
listElement* listRemoveElementAt(linkedList* list, uint32_t index);

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

void listSwapElementsAt(linkedList* list, uint32_t i1, uint32_t i2);

/**
 * Sorts the elements within a linked list.
 * @param list Pointer to the linked list.
 * @param compare Pointer to the comparison callback function.
 */
void listSort(linkedList* list, listSortCompare compare);

// </editor-fold>

#endif
