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
 * @brief C hashmap implementation
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <safe_lib.h>

#include "eis/msgbus/crc32.h"
#include "eis/msgbus/hashmap.h"

#define MAX_CHAIN_LEN 8

// Function prototypes (documentation with methods below).
static unsigned int hash_int(hashmap_t* map, const char* key);
static int hash(hashmap_t* map, const char* key);
static int rehash(hashmap_t* map);
static hashmap_ret_t put_helper(
        hashmap_t* map, char* key, void* val, void (*free_fn)(void*));

//
// Implementation of header file functions
//

hashmap_t* hashmap_new(size_t init_size) {
    hashmap_t* map = (hashmap_t*) malloc(sizeof(hashmap_t));
    if(map == NULL)
        return NULL;

    map->init_size = init_size;
    map->max_size = init_size;
    map->size = 0;
    // map->elems = (hashmap_elem_t*) malloc(sizeof(hashmap_elem_t) * init_size);
    map->elems = (hashmap_elem_t*) calloc(
            map->max_size, sizeof(hashmap_elem_t));
    if(map->elems == NULL) {
        free(map);
        return NULL;
    }

    // Initialize elements
    for(int i = 0; i < init_size; i++) {
        map->elems[i].key = NULL;
        map->elems[i].key_len = 0;
        map->elems[i].in_use = false;
        map->elems[i].value = NULL;
        map->elems[i].free = NULL;
    }

    return map;
}

hashmap_ret_t hashmap_put(
        hashmap_t* map, const char* key, void* val, void (*free_fn)(void*)) {
    // Copying the key value
    size_t len = strlen(key);
    char* key_cpy = (char*) malloc(sizeof(char) * len + 1);
    if(key_cpy == NULL) {
        return MAP_OMEM;
    }

    memcpy_s(key_cpy, len, key, len);
    key_cpy[len] = '\0';

    // Put the value into the hashmap
    hashmap_ret_t ret = put_helper(map, key_cpy, val, free_fn);
    if(ret != MAP_SUCCESS)
        free(key_cpy);

    return ret;
}

void* hashmap_get(hashmap_t* map, const char* key) {
    // Get intiial hash value
    uint32_t curr = hash_int(map, key);
    size_t key_len;
    int ind;

    // Linear probing
    for(int i = 0; i < MAX_CHAIN_LEN; i++) {
        if(map->elems[curr].in_use) {
            key_len = map->elems[curr].key_len;
            strcmp_s(map->elems[curr].key, key_len, key, &ind);
            if(ind == 0) {
                // Found the element
                return map->elems[curr].value;
            }
        }

        curr = (curr + 1) % map->max_size;
    }

    // Searched whole hashmap and did not find the element
    return NULL;
}

hashmap_ret_t hashmap_remove(hashmap_t* map, const char* key) {
    // Get intiial hash value
    uint32_t curr = hash_int(map, key);
    size_t key_len;
    int ind;

    // Linear probing
    for(int i = 0; i < MAX_CHAIN_LEN; i++) {
        if(map->elems[curr].in_use) {
            key_len = map->elems[curr].key_len;
            strcmp_s(map->elems[curr].key, key_len, key, &ind);
            if(ind == 0) {
                // Found the element, free user provided values
                map->elems[curr].free(map->elems[curr].value);
                free(map->elems[curr].key);

                // Reset the values
                map->elems[curr].key = NULL;
                map->elems[curr].key_len = 0;
                map->elems[curr].in_use = false;
                map->elems[curr].value = NULL;
                map->elems[curr].free = NULL;

                // Return early
                return MAP_SUCCESS;
            }
        }

        curr = (curr + 1) % map->max_size;
    }

    // Searched whole hashmap and did not find the element
    return MAP_KEY_NOT_EXISTS;
}

void hashmap_destroy(hashmap_t* map) {
    // Free all elements
    for(int i = 0; i < map->max_size; i++) {
        if(map->elems[i].in_use) {
            // Free user provided values
            map->elems[i].free(map->elems[i].value);
            free(map->elems[i].key);
            // Reset the values
            map->elems[i].key = NULL;
            map->elems[i].key_len = 0;
            map->elems[i].in_use = false;
            map->elems[i].value = NULL;
            map->elems[i].free = NULL;
        }
    }

    // Free the elements
    free(map->elems);

    // Free the map itself
    free(map);

    // Set to NULL (just in case)
    map = NULL;
}

//
// Helper function implementations
//

/**
 * Hash the given key to get the preferred index in the envelope
 *
 * @param map - Hashmap
 * @param key - Key to hash
 * @return index
 */
static unsigned int hash_int(hashmap_t* map, const char* key) {
    uint32_t crc = msgbus_crc32(key, strlen(key));

	// Robert Jenkins' 32 bit Mix Function
	crc += (crc << 12);
	crc ^= (crc >> 22);
	crc += (crc << 4);
	crc ^= (crc >> 9);
	crc += (crc << 10);
	crc ^= (crc >> 2);
	crc += (crc << 7);
	crc ^= (crc >> 12);

	// Knuth's Multiplicative Method
	crc = (crc >> 3) * 2654435761;

	return crc % map->max_size;
}

/**
 * Hash the given key.
 *
 * @param[in]  map  - Message envelope
 * @param[in]  key  - Key to hash
 * @param[out] hash - Index to store the value at if found
 * @return hashmap_ret_t
 */
static int hash(hashmap_t* map, const char* key) {
    int curr;
    int i;

    // Check if map is full, if so return
    if(map->size >= (map->max_size / 2)) return MAP_FULL;

    // Get preferred has index for the key
    curr = hash_int(map, key);
    curr = hash_int(map, key);

    int ind = 0;

    // Linear probing
    for(i = 0; i < MAX_CHAIN_LEN; i++) {
        // We have a good index to use
        if(!map->elems[curr].in_use) return curr;

        strcmp_s(map->elems[curr].key, map->elems[curr].key_len, key, &ind);

        // For a message envelope only one value at a key can exist, there is
        // no updating currently supported, a new message envelope must be
        // created for each message
        if(map->elems[curr].in_use && ind == 0)
            return MAP_KEY_EXISTS;

        curr = (curr + 1) % map->max_size;
    }

    // Reached max chain size and therefore the map is full for those CRC
    // collisions
    return MAP_FULL;
}

/**
 * Rehash the hash map to add init_size more items.
 *
 * @param map - Hashmap
 * @return hashmap_ret_t
 */
static hashmap_ret_t rehash(hashmap_t* map) {
    int i;
    hashmap_ret_t status;
    int old_size;
    hashmap_elem_t* curr;

    // Initialize new message envelope elements buffer
    hashmap_elem_t* temp = (hashmap_elem_t*) calloc(
            map->max_size + map->init_size, sizeof(hashmap_elem_t));
    if(!temp) return MAP_OMEM;

    // Update the array
    curr = map->elems;
    map->elems = temp;

    // Update the size values
    old_size = map->max_size;
    map->max_size = map->init_size + old_size;
    map->size = 0;

    // Rehash all of the elements
    for(i = 0; i < old_size; i++) {

        // If dealing with an empty slot, continue to the next one
        if(!curr[i].in_use) continue;

        // Put value into resized envelope
        status = put_helper(map, curr[i].key, curr[i].value, curr[i].free);
        if(status != MAP_SUCCESS) goto err;
    }

    free(curr);
    return MAP_SUCCESS;

err:
    free(curr);
    return status;
}

static hashmap_ret_t put_helper(
        hashmap_t* map, char* key, void* val, void (*free_fn)(void*)) {
    // Get hash index
    int index = hash(map, key);
    hashmap_ret_t ret;

    // Keep on rehashing until we can do something
    while(index == MAP_FULL) {
        ret = rehash(map);
        if(ret != MAP_SUCCESS)
            return ret;

        index = hash(map, key);
    }

    if(index == MAP_KEY_EXISTS)
        return MAP_KEY_EXISTS;

    map->elems[index].key = key;
    map->elems[index].key_len = strlen(key);
    map->elems[index].in_use = true;
    map->elems[index].value = val;
    map->elems[index].free = free_fn;

    return MAP_SUCCESS;
}
