// Copyright (c) 2019 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief C singly linked list
 */

#ifndef _EIS_MSGENV_LINKEDLIST_H
#define _EIS_MSGENV_LINKEDLIST_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Return types for the linked list.
 */
typedef enum {
    LL_SUCCESS       = 0,
    LL_ERR_APPEND    = 1,
    LL_ERR_NOT_FOUND = 2,
} linkedlist_ret_t;

// Forward declaration of node_t
typedef struct _node_t node_t;

/**
 * Linked list node type.
 */
typedef struct _node_t {
    // Pointer to next node
    node_t* next;

    // Value at the current node
    void* value;

    // Function to free the value
    void (*free)(void*);
} node_t;

/**
 * Linked list context
 */
typedef struct {
    // Root element of the linked list
    node_t* root;
    int len;
} linkedlist_t;

/**
 * Helper macro to loop through each linked list value.
 *
 * \note A variable value will be available that is casted to the underlying
 *      type for the void* in the node given via the VALUE_TYPE parameter.
 *
 * @param LL - Linked list to loop through
 * @param VALUE_TYPE - Type to cast the void* to
 * @param CODE       - Code block to execute for each node
 */
#define LINKEDLIST_FOREACH(LL, VALUE_TYPE, CODE) { \
    node_t* curr = LL->root; \
    while(curr != NULL) { \
        VALUE_TYPE* value = (VALUE_TYPE*) curr->value; \
        CODE \
        curr = curr->next; \
    } \
}

/**
 * Create a new linked list.
 *
 * @return @c linkedlist_t, or NULL if an error occurs
 */
linkedlist_t* linkedlist_new();

/**
 * Create a new node for the linked list.
 */
node_t* linkedlist_node_new(void* value, void (*free_fn)(void*));

/**
 * Add new item onto the end of the linked list.
 *
 * @param ll   - Linked list
 * @param node - Node to be appended
 * @return @c linkedlist_ret_t
 */
linkedlist_ret_t linkedlist_add(linkedlist_t* ll, node_t* node);

/**
 * Get node at index from the linked list.
 *
 * @param ll - Linked list
 * @param idx - Index from which to get the element
 * @return @c node_t, or NULL if not found
 */
node_t* linkedlist_get_at(linkedlist_t* ll, int idx);

/**
 * Remove item at the given index from the linked list.
 *
 * @param ll  - Linked list
 * @param idx - Index at which to remove a node
 */
linkedlist_ret_t linkedlist_remove_at(linkedlist_t* ll, int idx);

/**
 * Destroy the given linked list freeing all of its memory (including all of
 * the node's memory).
 *
 * @param ll - Linked list to destroy
 */
void linkedlist_destroy(linkedlist_t* ll);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EIS_MSGENV_LINKEDLIST_H
