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

/**
 * This module includes structures and functions for basic dynamic collections like
 * linked lists, dictionaries, and binary search trees.
 *
 * @todo Helper function for libraries (lists for opening / closing libraries)
 */

#ifndef APPSUPPORT_COLLECTIONS_H
#define APPSUPPORT_COLLECTIONS_H

#include <stdint.h>
#include <stdlib.h>
#include <exec/types.h>

// <editor-fold desc="Linked lists">

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
 * Callback functions for linked list elements.
 * This type is used, for example, by the foreach construct.
 */
typedef void(*listElementCallback)(listElement*);

/**
 * Callback function for comparing two list elements.
 * This callback can be used for sorting linked lists.
 */
typedef int(*listSortCompare)(listElement*, listElement*);

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
     * Optional callback function to compare to list elements.
     * Set this to automatically sort list elements when inserting them
     * via listAddElement().
     */
    listSortCompare compare;

    /**
     * Current number of linked elements.
     */
    uint32_t length;
} linkedList;

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
 * Creates a new linked list element and adds it to the list.
 * If list->compare is set, the element is inserted in sorted order.
 * Otherwise, it is appended to the end of the list.
 * @param list Pointer to the linked list.
 * @param data Pointer to the element's data.
 * @return Pointer to the newly created list element.
 */
listElement* listAddElement(linkedList* list, void* data);

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
 * Caution: The element is being removed from the linked list, but its
 * memory will not be freed. This has to be done by the caller!
 * @param list Pointer to the linked list.
 * @param index Index of the element to be removed.
 * @return Pointer to the removed list element.
 */
listElement* listRemoveElementAt(linkedList* list, uint32_t index);

/**
 * Calls a function for every element of the given list.
 * @param list Pointer to the linked list structure.
 * @param callback Pointer to a callback function.
 */
void listForeach(linkedList* list, listElementCallback callback);

/**
 * Disposes a linked list.
 * Note that the linked elements have to be cleaned up before
 * disposing the list.
 * @param list Pointer to the linked list structure.
 */
void listDispose(linkedList* list);

/**
 * Swaps two elements within the given list.
 * The current implementation is surely not the best / the fastest, so it should be improved
 * some time.
 * @param list Pointer to the linked list.
 * @param i1 Index of the first element to be swapped.
 * @param i2 Index of the second element to be swapped.
 */
void listSwapElementsAt(linkedList* list, uint32_t i1, uint32_t i2);

/**
 * Sorts the elements within a linked list.
 * The current implementation is definitely not the best solution, but for small lists it should work.
 * @param list Pointer to the linked list.
 * @param compare Pointer to the comparison callback function.
 */
void listSort(linkedList* list, listSortCompare compare);

// </editor-fold>


// <editor-fold desc="Dictionaries">

/**
 * Type for dictionary elements.
 */
typedef struct dictionaryElement dictionaryElement;

/**
 * Callback function type for dictionary element processing.
 */
typedef void(*dictElementCallback)(dictionaryElement*);

/**
 * The base structure for dictionaries.
 */
typedef struct {

    /**
     * Pointer to the internal linked list.
     */
    linkedList* list;

    /**
     * Current number of elements.
     */
    uint32_t length;

} dictionary;

/**
 * Defines the element of a dictionary.
 */
struct dictionaryElement {

    /**
     * Hoping that sorting integers is faster or more efficient than
     * sorting strings, an integer value (calculated by a simple hash function)
     * is used for the key.
     */
    uint32_t hash;

    /**
     * The original string is stored, too, but it will not be used for searching
     * and sorting.
     */
    STRPTR key;

    /**
     * A pointer to the actual data.
     */
    void* data;

};

/**
 * Creates an empty dictionary.
 * @return Pointer to the created dictionary structure.
 */
dictionary* dictCreate();

/**
 * Adds or updates an element in the dictionary.
 * @param dict Pointer to the dictionary.
 * @param key The key for the element.
 * @param data Pointer to the element's data.
 */
void dictSet(dictionary* dict, STRPTR key, void* data);

/**
 * Returns the data associated with the given key or NULL if the key
 * does not exist in the dictionary.
 * @param dict Pointer to the dictionary.
 * @param key The key for the requested element.
 * @return Pointer to the requested data or NULL.
 */
void* dictGet(dictionary* dict, STRPTR key);

/**
 * Removes an element from the dictionary.
 * Note that the data pointer in the element has to be cleaned up
 * before unsetting the dictionary entry if it was dynamically allocated.
 * @param dict Pointer to the dictionary.
 * @param key The key of the element to be removed.
 */
void dictUnset(dictionary* dict, STRPTR key);

/**
 * Calls a function for every element of the given dictionary.
 * @param dict Pointer to the dictionary structure.
 * @param callback Pointer to a callback function.
 */
void dictForeach(dictionary* dict, dictElementCallback callback);

/**
 * Disposes a dictionary and its internal structures.
 * Note that the data pointers in the elements have to be cleaned up
 * before disposing the dictionary if they were dynamically allocated.
 * @param dict Pointer to the dictionary structure.
 */
void dictDispose(dictionary* dict);

// </editor-fold>


// <editor-fold desc="Binary Trees">

/**
 * Type for binary tree nodes.
 */
typedef struct treeNode treeNode;

/**
 * Describes a node of a binary tree.
 */
struct treeNode {
    /**
     * A pointer to the actual data.
     */
    void* data;

    /**
     * Pointer to the left child or NULL.
     */
    treeNode* left;

    /**
     * Pointer to the right child or NULL.
     */
    treeNode* right;
};

/**
 * Callback function for binary tree nodes.
 * Used for tree traversals.
 */
typedef void(*treeNodeCallback)(treeNode*);

/**
 * Callback function for comparing two tree nodes.
 * Used for inserting and searching in the binary tree.
 */
typedef int(*treeCompare)(treeNode*, treeNode*);

/**
 * This struct describes the base of a binary tree.
 */
typedef struct {
    /**
     * Pointer to the root node or NULL.
     */
    treeNode* root;

    /**
     * Comparison function to maintain the tree order.
     */
    treeCompare compare;

    /**
     * Current number of nodes.
     */
    uint32_t length;
} binaryTree;

/**
 * Creates an empty binary tree.
 * @param compare Comparison function.
 * @return Pointer to the created tree structure.
 */
binaryTree* treeCreate(treeCompare compare);

/**
 * Creates a new tree node and references the given data.
 * @param data Pointer to the element's data.
 * @return Pointer to the newly created tree node.
 */
treeNode* treeCreateNode(void* data);

/**
 * Adds a new element to the binary tree.
 * @param tree Pointer to the binary tree.
 * @param data Pointer to the element's data.
 * @return Pointer to the newly created tree node.
 */
treeNode* treeAddElement(binaryTree* tree, void* data);

/**
 * Searches for an element in the binary tree.
 * @param tree Pointer to the binary tree.
 * @param searchNode A node containing the data to search for.
 * @return Pointer to the found node or NULL.
 */
treeNode* treeFindElement(binaryTree* tree, treeNode* searchNode);

/**
 * Removes an element from the binary tree.
 * Note that the data pointer in the node has to be cleaned up
 * before unsetting the tree node if it was dynamically allocated.
 * @param tree Pointer to the binary tree.
 * @param searchNode A node containing the data to search and remove.
 */
void treeRemoveElement(binaryTree* tree, treeNode* searchNode);

/**
 * Removes a tree node along with all of its child nodes.
 * @param tree Pointer to the binary tree.
 * @param searchNode A node containing the data to search and remove.
 */
void treeRemoveSubtree(binaryTree* tree, treeNode* searchNode);

/**
 * Traverses the tree in-order and calls the callback for each node.
 * @param tree Pointer to the binary tree.
 * @param callback Pointer to the callback function.
 */
void treeInOrder(binaryTree* tree, treeNodeCallback callback);

/**
 * Traverses the tree pre-order and calls the callback for each node.
 * @param tree Pointer to the binary tree.
 * @param callback Pointer to the callback function.
 */
void treePreOrder(binaryTree* tree, treeNodeCallback callback);

/**
 * Traverses the tree post-order and calls the callback for each node.
 * @param tree Pointer to the binary tree.
 * @param callback Pointer to the callback function.
 */
void treePostOrder(binaryTree* tree, treeNodeCallback callback);

/**
 * Disposes the binary tree and its internal nodes.
 * @param tree Pointer to the binary tree.
 */
void treeDispose(binaryTree* tree);

// </editor-fold>

#endif
