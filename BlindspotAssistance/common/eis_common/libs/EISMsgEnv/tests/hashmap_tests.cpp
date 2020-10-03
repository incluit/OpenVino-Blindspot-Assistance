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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief Hashmap tests
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "eis/msgbus/hashmap.h"

/**
 * Simple initialization and destroy case with no data.
 */
TEST(hashmap_tests, simple_init) {
    hashmap_t* map = hashmap_new(128);
    hashmap_destroy(map);
}

/**
 * Test to test the put->get->remove flow
 */
TEST(hashmap_tests, simple_put_get_remove) {
    hashmap_t* map = hashmap_new(128);
    char* data = (char*) malloc(sizeof(char) * 14);
    memcpy(data, "Hello, world!", 13);
    data[13] = '\0';

    hashmap_ret_t ret = hashmap_put(map, "test", (void*) data, free);
    ASSERT_EQ(ret, MAP_SUCCESS);

    void* get = hashmap_get(map, "test");
    if(get == NULL)
        FAIL() << "Failed to get \"test\" key";
    ASSERT_STREQ((char*) get, data);

    ret = hashmap_remove(map, "test");
    ASSERT_EQ(ret, MAP_SUCCESS);

    // Verify that the remove worked
    get = hashmap_get(map, "test");
    if(get != NULL)
        FAIL() << "Removal of \"test\" failed";

    hashmap_destroy(map);
}

/**
 * Test to validate correct error if a key already exists
 */
TEST(hashmap_tests, already_exists) {
    hashmap_t* map = hashmap_new(128);
    char* data = (char*) malloc(sizeof(char) * 14);
    memcpy(data, "Hello, world!", 13);
    data[13] = '\0';

    hashmap_ret_t ret = hashmap_put(map, "test", (void*) data, free);
    ASSERT_EQ(ret, MAP_SUCCESS);

    ret = hashmap_put(map, "test", (void*) data, free);
    ASSERT_EQ(ret, MAP_KEY_EXISTS);

    hashmap_destroy(map);
}

/**
 * Test to validate rehashing behaivior.
 */
TEST(hashmap_tests, rehash) {
    hashmap_t* map = hashmap_new(256);
    char** keys = (char**) malloc(sizeof(char*) * 260);

    for(int i = 0; i < 260; i++) {
        char* key = (char*) malloc(sizeof(char) * 12);
        sprintf(key, "testing-%03d", i);
        keys[i] = key;
        hashmap_ret_t ret = hashmap_put(map, key, key, free);
        ASSERT_EQ(ret, MAP_SUCCESS) << "Failed to put element " << i;
    }

    hashmap_destroy(map);
    free(keys);
}
