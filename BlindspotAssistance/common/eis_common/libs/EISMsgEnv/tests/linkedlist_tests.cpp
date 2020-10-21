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
 * @brief Linked list tests
 */

#include <gtest/gtest.h>
#include "eis/msgbus/linkedlist.h"

#define ASSERT_NULL(val) { \
    if(val != NULL) FAIL() << "Value should be NULL"; \
}

#define ASSERT_NOT_NULL(val) { \
    if(val == NULL) FAIL() << "Value shoud not be NULL"; \
}

TEST(linkedlist_tests, simple_init) {
    linkedlist_t* ll = linkedlist_new();
    ASSERT_NOT_NULL(ll);
    linkedlist_destroy(ll);
}

TEST(linkedlist_tests, remove_single) {
    linkedlist_ret_t ret = LL_SUCCESS;
    linkedlist_t* ll = linkedlist_new();
    ASSERT_NOT_NULL(ll);

    // Add 10 items to the linked list
    for(int i = 0; i < 1; i++) {
        char* data = (char*) malloc(sizeof(char) * 8);
        ASSERT_NOT_NULL(data);
        sprintf(data, "test-%02d", i);
        data[7] = '\0';

        node_t* node = linkedlist_node_new((void*) data, free);
        ASSERT_NOT_NULL(node);

        ret = linkedlist_add(ll, node);
        ASSERT_EQ(ret, LL_SUCCESS);
    }

    // Verify length is correct
    ASSERT_EQ(ll->len, 1);

    node_t* node = linkedlist_get_at(ll, 0);
    ASSERT_NOT_NULL(node);
    ASSERT_STREQ((char*) node->value, "test-00");

    ret = linkedlist_remove_at(ll, 0);
    ASSERT_EQ(ret, LL_SUCCESS);

    ASSERT_EQ(ll->len, 0);

    // Get out-of-bounds
    node_t* oob = linkedlist_get_at(ll, 50);
    ASSERT_NULL(oob);

    // Remove out-of-bounds
    ret = linkedlist_remove_at(ll, 50);
    ASSERT_EQ(ret, LL_ERR_NOT_FOUND);

    linkedlist_destroy(ll);
}

TEST(linkedlist_tests, simple_add_get_remove) {
    linkedlist_ret_t ret = LL_SUCCESS;
    linkedlist_t* ll = linkedlist_new();
    ASSERT_NOT_NULL(ll);

    // Add 10 items to the linked list
    for(int i = 0; i < 10; i++) {
        char* data = (char*) malloc(sizeof(char) * 8);
        ASSERT_NOT_NULL(data);
        sprintf(data, "test-%02d", i);
        data[7] = '\0';

        node_t* node = linkedlist_node_new((void*) data, free);
        ASSERT_NOT_NULL(node);

        ret = linkedlist_add(ll, node);
        ASSERT_EQ(ret, LL_SUCCESS);
    }

    // Verify length is correct
    ASSERT_EQ(ll->len, 10);

    // Get 1st element
    node_t* elem = linkedlist_get_at(ll, 0);
    ASSERT_NOT_NULL(elem);
    ASSERT_STREQ((char*) elem->value, "test-00");

    // Get 5th element
    node_t* fifth = linkedlist_get_at(ll, 4);
    ASSERT_NOT_NULL(fifth);
    ASSERT_STREQ((char*) fifth->value, "test-04");

    // Remove first element
    ret = linkedlist_remove_at(ll, 0);
    ASSERT_EQ(ret, LL_SUCCESS);
    ASSERT_EQ(ll->len, 9);

    // Remove 5th element
    ret = linkedlist_remove_at(ll, 4);
    ASSERT_EQ(ret, LL_SUCCESS);

    // Verify length is correct
    ASSERT_EQ(ll->len, 8);

    // Get 5th to verify it is different now
    fifth = linkedlist_get_at(ll, 4);
    ASSERT_NOT_NULL(fifth);
    ASSERT_STREQ((char*) fifth->value, "test-06");

    // Remove last element
    ret = linkedlist_remove_at(ll, 7);
    ASSERT_EQ(ret, LL_SUCCESS);
    ASSERT_EQ(ll->len, 7);

    // Get out-of-bounds
    node_t* oob = linkedlist_get_at(ll, 50);
    ASSERT_NULL(oob);

    // Remove out-of-bounds
    ret = linkedlist_remove_at(ll, 50);
    ASSERT_EQ(ret, LL_ERR_NOT_FOUND);

    linkedlist_destroy(ll);
}
