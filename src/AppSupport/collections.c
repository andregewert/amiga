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

#include "collections.h"

linkedList* listCreate() {
    linkedList* newList = (linkedList*) malloc(sizeof(linkedList));
    newList->firstElement = NULL;
    newList->length = 0;
    return newList;
}

listElement* listCreateElement(void* data) {
    listElement* element =
            (listElement*)malloc(sizeof(listElement));
    if (element != NULL) {
        element->data = data;
        element->nextElement = NULL;
    }
    return element;
}

listElement* listAppendElement(linkedList* list, void* data) {
    if (list == NULL) return NULL;

    listElement* newElement = listCreateElement(data);
    if (newElement == NULL) return NULL;

    if (list->firstElement == NULL) {
        list->firstElement = newElement;
    } else {
        listElement* currentNode = list->firstElement;
        while (currentNode->nextElement != NULL) {
            currentNode = currentNode->nextElement;
        }
        currentNode->nextElement = newElement;
    }
    list->length++;
    return newElement;
}

listElement* listGetElementAt(linkedList* list, uint32_t index) {
    if (list == NULL || list->length <= index) {
        return NULL;
    }

    listElement* node = list->firstElement;
    for (uint32_t i = 0; i < index; i++) {
        node = node->nextElement;
    }
    return node;
}

listElement* listRemoveElementAt(linkedList* list, uint32_t index) {
    if (list == NULL || list->firstElement == NULL) return NULL;
    listElement* element = listGetElementAt(list, index);
    if (element == NULL) return NULL;

    if (index == 0) {
        list->firstElement = element->nextElement;
    } else {
        listElement* previous = listGetElementAt(list, index -1);
        if (previous != NULL) {
            previous->nextElement = element->nextElement;
        }
    }
    element->nextElement = NULL;
    list->length--;
    return element;
}

void listForeach(linkedList* list, listElementCallback callback) {
    if (callback == NULL) return;
    if (list == NULL || list->firstElement == NULL) return;

    listElement* node = list->firstElement;
    while (node != NULL) {
        callback(node);
        node = node->nextElement;
    }
}

void listDispose(linkedList* list) {
    if (list == NULL) return;

    listElement* node = list->firstElement;
    listElement* dispose = NULL;

    while (node != NULL) {
        dispose = node;
        node = dispose->nextElement;
        printf("Disposing %p\n", dispose);
        dispose->data = NULL;
        dispose->nextElement = NULL;
        free(dispose);
    }

    free(list);
}


void listSwapElementsAt(linkedList* list, uint32_t i1, uint32_t i2) {
    if (list == NULL || list->length < 2 || i1 == i2) return;
    // TODO
}

void listSort(linkedList* list, listSortCompare compare) {
    if (list == NULL || list->length == 0) return;

    for (int i = 0; i < list->length; i++) {
        for (int k = i +1; k < list->length; k++) {
            if (compare(listGetElementAt(list, i), listGetElementAt(list, k)) > 0) {
                // swap
                printf("Swapping element %d and %d\n", i, k);
                listSwapElementsAt(list, i, k);
            }
        }
    }
}
