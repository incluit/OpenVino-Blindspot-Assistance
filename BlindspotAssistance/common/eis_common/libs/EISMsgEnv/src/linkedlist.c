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
 * @brief C singly linked list implementation
 */

#include <stdlib.h>
#include "eis/msgbus/linkedlist.h"

linkedlist_t* linkedlist_new() {
    linkedlist_t* ll = (linkedlist_t*) malloc(sizeof(linkedlist_t));
    if(ll == NULL)
        return NULL;

    ll->root = NULL;
    ll->len = 0;

    return ll;
}

node_t* linkedlist_node_new(void* value, void (*free_fn)(void*)) {
    node_t* node = (node_t*) malloc(sizeof(node_t));
    if(node == NULL)
        return NULL;

    node->next = NULL;
    node->value = value;
    node->free = free_fn;

    return node;
}

linkedlist_ret_t linkedlist_add(linkedlist_t* ll, node_t* node) {
    if(ll->root == NULL) {
        ll->root = node;
    } else {
        node_t* curr = ll->root;
        while(curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = node;
    }

    ll->len++;

    return LL_SUCCESS;
}

node_t* linkedlist_get_at(linkedlist_t* ll, int idx) {
    if(idx >= ll->len || ll->root == NULL)
        return NULL;

    node_t* node = ll->root;
    for(int i = 0; i < idx; i++) {
        node = node->next;
    }

    return node;
}

linkedlist_ret_t linkedlist_remove_at(linkedlist_t* ll, int idx) {
    // Covers out-of-bounds and no elements cases
    if(idx >= ll->len || ll->root == NULL)
        return LL_ERR_NOT_FOUND;

    if(ll->root->next == NULL) {
        // Single element case
        ll->root->free(ll->root->value);
        free(ll->root);
        ll->root = NULL;
        ll->len--;
        return LL_SUCCESS;
    }

    node_t* prev = NULL;
    node_t* curr = ll->root;
    for(int i = 0; i < idx; i++) {
        prev = curr;
        curr = curr->next;
    }

    if(prev == NULL) {
        // Removing the first element
        ll->root = curr->next;
    } else {
        // Removing in the middle or the last
        prev->next = curr->next;
    }

    // Free the curr element now that all data is connected again
    curr->free(curr->value);
    free(curr);

    // Decrement the number of items
    ll->len--;

    return LL_SUCCESS;
}

void linkedlist_destroy(linkedlist_t* ll) {
    if(ll->root != NULL) {
        // Free all the elements
        node_t* prev = NULL;
        node_t* curr = ll->root;
        while(curr != NULL) {
            prev = curr;
            curr = curr->next;

            // Free the element
            prev->free(prev->value);
            free(prev);
        }
    }

    // Finally, free the linked list structure
    free(ll);
}
