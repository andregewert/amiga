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
#include <string.h>

// <editor-fold desc="Linked lists">

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

listElement* listAddElement(linkedList* list, void* data) {
    if (list == NULL) return NULL;

    listElement* newElement = listCreateElement(data);
    if (newElement == NULL) return NULL;

    if (list->firstElement == NULL) {
        list->firstElement = newElement;
    }

    // We assume the existing list contents are sorted
    else if (list->compare != NULL) {
        listElement* current = list->firstElement;
        listElement* previous = NULL;

        while (current != NULL && list->compare(newElement, current) > 0) {
            previous = current;
            current = current->nextElement;
        }

        if (previous == NULL) {
            newElement->nextElement = list->firstElement;
            list->firstElement = newElement;
        } else {
            newElement->nextElement = current;
            previous->nextElement = newElement;
        }
    }

    // List is not sorted, the new element will be appended
    else {
        listElement* currentNode = list->firstElement;
        while (currentNode->nextElement != NULL) {
            currentNode = currentNode->nextElement;
        }
        currentNode->nextElement = newElement;
    }

    list->length++;
    return newElement;
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
        dispose->data = NULL;
        dispose->nextElement = NULL;
        free(dispose);
    }

    free(list);
}

void listSwapElementsAt(linkedList* list, uint32_t i1, uint32_t i2) {
    if (list == NULL || list->length < 2 || i1 == i2) return;

    listElement* element1 = listGetElementAt(list, i1);
    listElement* element2 = listGetElementAt(list, i2);
    if (element1 == NULL || element2 == NULL) return;

    listElement* element1Next = element1->nextElement;
    listElement* element2Next = element2->nextElement;
    listElement* element1Prev = NULL;
    listElement* element2Prev = NULL;

    if (i1 > 0) {
        element1Prev = listGetElementAt(list, i1 -1);
    }
    if (i2 > 0) {
        element2Prev = listGetElementAt(list, i2 -1);
    }

    if (element2 == element1Next) {
        element2->nextElement = element1;
        element1->nextElement = element2Next;
        if (element1Prev != NULL) element1Prev->nextElement = element2;
        else list->firstElement = element2;
        return;
    }
    if (element1 == element2Next) {
        element1->nextElement = element2;
        element2->nextElement = element1Next;
        if (element2Prev != NULL) element2Prev->nextElement = element1;
        else list->firstElement = element1;
        return;
    }

    element1->nextElement = element2Next;
    element2->nextElement = element1Next;

    if (element1Prev != NULL) element1Prev->nextElement = element2;
    else list->firstElement = element2;

    if (element2Prev != NULL) element2Prev->nextElement = element1;
    else list->firstElement = element1;
}

void listSort(linkedList* list, listSortCompare compare) {
    if (list == NULL || list->length == 0) return;

    for (int i = 0; i < list->length; i++) {
        for (int k = i +1; k < list->length; k++) {
            if (compare(listGetElementAt(list, i), listGetElementAt(list, k)) > 0) {
                listSwapElementsAt(list, i, k);
            }
        }
    }
}

// </editor-fold>


// <editor-fold desc="Dictionaries">

/**
 * Simple hash function for strings (DJB2)
 * @param str The string to hash.
 * @return The calculated hash.
 */
static uint32_t calculateHash(const char* str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/**
 * Comparison function for dictionary elements based on hash.
 * Used for sorted insert into the internal linked list.
 * @param e1 First element.
 * @param e2 Second element.
 * @return Comparison result.
 */
static int dictCompareElements(listElement* e1, listElement* e2) {
    dictionaryElement* d1 = (dictionaryElement*)e1->data;
    dictionaryElement* d2 = (dictionaryElement*)e2->data;
    if (d1->hash < d2->hash) return -1;
    if (d1->hash > d2->hash) return 1;
    return 0;
}

dictionary* dictCreate() {
    dictionary* dict = (dictionary*)malloc(sizeof(dictionary));
    if (dict != NULL) {
        dict->list = listCreate();
        if (dict->list != NULL) {
            dict->list->compare = dictCompareElements;
        }
        dict->length = 0;
    }
    return dict;
}

void dictSet(dictionary* dict, STRPTR key, void* data) {
    if (dict == NULL || dict->list == NULL || key == NULL) return;

    uint32_t hash = calculateHash((const char*)key);

    // Check if key already exists
    listElement* current = dict->list->firstElement;
    while (current != NULL) {
        dictionaryElement* element = (dictionaryElement*)current->data;
        if (element->hash == hash && strcmp((const char*)element->key, (const char*)key) == 0) {
            // Update existing element
            element->data = data;
            return;
        }
        // Since the list is sorted by hash, we can stop if we've passed the hash
        if (element->hash > hash) break;
        current = current->nextElement;
    }

    // Key doesn't exist, create new element
    dictionaryElement* newElement = (dictionaryElement*)malloc(sizeof(dictionaryElement));
    if (newElement != NULL) {
        newElement->hash = hash;
        newElement->key = (STRPTR)strdup((const char*)key);
        newElement->data = data;
        listAddElement(dict->list, newElement);
        dict->length++;
    }
}

void* dictGet(dictionary* dict, STRPTR key) {
    if (dict == NULL || dict->list == NULL || key == NULL) return NULL;

    uint32_t hash = calculateHash((const char*)key);

    listElement* current = dict->list->firstElement;
    while (current != NULL) {
        dictionaryElement* element = (dictionaryElement*)current->data;
        if (element->hash == hash && strcmp((const char*)element->key, (const char*)key) == 0) {
            return element->data;
        }
        if (element->hash > hash) break;
        current = current->nextElement;
    }

    return NULL;
}

void dictUnset(dictionary* dict, STRPTR key) {
    if (dict == NULL || dict->list == NULL || key == NULL) return;

    uint32_t hash = calculateHash((const char*)key);

    listElement* current = dict->list->firstElement;
    listElement* previous = NULL;
    while (current != NULL) {
        dictionaryElement* element = (dictionaryElement*)current->data;
        if (element->hash == hash && strcmp((const char*)element->key, (const char*)key) == 0) {
            // Found it, remove from list
            if (previous == NULL) {
                dict->list->firstElement = current->nextElement;
            } else {
                previous->nextElement = current->nextElement;
            }
            dict->list->length--;
            dict->length--;

            // Free dictionary element data
            if (element->key != NULL) {
                free(element->key);
            }
            free(element);

            // Free the list element itself
            current->data = NULL;
            current->nextElement = NULL;
            free(current);
            return;
        }
        if (element->hash > hash) break;
        previous = current;
        current = current->nextElement;
    }
}

void dictForeach(dictionary* dict, dictElementCallback callback) {
    if (dict == NULL || dict->list == NULL || callback == NULL) return;

    listElement* current = dict->list->firstElement;
    while (current != NULL) {
        callback((dictionaryElement*)current->data);
        current = current->nextElement;
    }
}

void dictDispose(dictionary* dict) {
    if (dict == NULL) return;

    if (dict->list != NULL) {
        listElement* current = dict->list->firstElement;
        while (current != NULL) {
            dictionaryElement* element = (dictionaryElement*)current->data;
            if (element->key != NULL) {
                free(element->key);
            }
            free(element);
            current = current->nextElement;
        }
        listDispose(dict->list);
    }
    free(dict);
}

// </editor-fold>
