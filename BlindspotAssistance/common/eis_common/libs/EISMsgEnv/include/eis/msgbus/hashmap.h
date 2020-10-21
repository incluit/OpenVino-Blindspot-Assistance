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
 * @brief C hashmap implementation
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#ifndef _EIS_MSGENV_HASHMAP_H
#define _EIS_MSGENV_HASHMAP_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Hashmap return values
 */
typedef enum {
    MAP_SUCCESS        = 0,
    MAP_FULL           = -1,
    MAP_OMEM           = -2,
    MAP_KEY_EXISTS     = -3,
    MAP_KEY_NOT_EXISTS = -4,
} hashmap_ret_t;

/**
 * Hashmap element structure.
 */
typedef struct {
    // Element key
    char* key;

    // Length of the key
    size_t key_len;

    // Whether or not the element slot is currently in use (i.e. value != NULL)
    bool in_use;

    // Value of the (key, value) pair
    void* value;

    // Free function for the value
    void (*free)(void*);
} hashmap_elem_t;

/**
 * Hashmap context.
 */
typedef struct {
    // Initial size of the hashmap (used for growing the hashmap).
    int init_size;

    // Current maximum number of keys (currently allocated memory)
    int max_size;

    // Number of assigned slots
    int size;

    // Hashmap elements
    hashmap_elem_t* elems;
} hashmap_t;

/**
 * Helper macro for looping over all of the elements in the hashmap.
 *
 * For the given code block (i.e. the CODE parameter), there is a value and
 * key variable made available with the const char* for the key and the value
 * casted to the given VALUE_TYPE.
 *
 * \note This is not necessarily super efficient, because it must loop over
 *      all of the allocated/in-use elements. So it will be O(N) where N is
 *      the current max_size of the hashmap.
 *
 * @param MAP - The hashmap to loop over
 * @param VALUE_TYPE - Type to cast the void* value to to make available
 * @param CODE       - Code block to be executed on each (key, value) pair
 */
#define HASHMAP_LOOP(MAP, VALUE_TYPE, CODE) \
    for(int i = 0; i < MAP->max_size; i++) { \
        if(MAP->elems[i].in_use) { \
            VALUE_TYPE* value = MAP->elems[i].value; \
            const char* key = MAP->elems[i].key; \
            CODE \
        } \
    }

/**
 * Create a new hashmap.
 *
 * \note init_size will be the initial size of the hashmap. When it grows past
 *      that it will allocated init_size amount of additional memory.
 *
 * @param init_size - Initial size of the hashmap
 * @return @c hashmap_t*
 */
hashmap_t* hashmap_new(size_t init_size);

/**
 * Put a (key, value) pair into the hashmap.
 *
 * @param map     - Hashmap to put the pair into
 * @param key     - Key for the value
 * @param val     - Value associated with the key
 * @param free_fn - Free function for the value
 * @return @c hashmap_ret_t
 */
hashmap_ret_t hashmap_put(
        hashmap_t* map, const char* key, void* val, void (*free_fn)(void*));

/**
 * Get the value associated with the given key if it exists.
 *
 * @param map - Hashmap to retrieve a value from.
 * @param key - Key for the value
 * @return void*, or NULL if the key does not exist
 */
void* hashmap_get(hashmap_t* map, const char* key);

/**
 * Remove item from the hashmap.
 *
 * @param map - Hashmap to remove value from
 * @param key - Key for the value
 * @return @c hashmap_ret_t
 */
hashmap_ret_t hashmap_remove(hashmap_t* map, const char* key);

/**
 * Destroy the given hashmap.
 *
 * @param map - Hashmap to destroy
 */
void hashmap_destroy(hashmap_t* map);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EIS_MSGENV_HASHMAP_H
