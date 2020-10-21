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
 * @brief EIS configuration interface unit tests
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

// NOTE: The configuration interface is tested using the JSON configuration
// loading utility

#include <gtest/gtest.h>
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"

// Test JSON configuration file for verifying configuration interface
#define TEST_JSON_FN "./test_config.json"

// Expected values
#define INT_KEY "int"
#define INT_VAL 43
#define FLT_KEY "float"
#define FLT_VAL 25.2
#define STR_KEY "string"
#define STR_VAL "TEST"
#define STR_LEN 4
#define BLN_KEY "boolean"
#define BLN_VAL true
#define NIL_KEY "empty"
#define ARR_KEY "array"
#define ARR_LEN 1
#define OBJ_KEY "object"

config_value_t* helper_get(config_t* cfg, const char* key) {
    config_value_t* cv = config_get(cfg, key);
    if(cv == NULL) {
        LOG_ERROR("Failed to get config value: %s", key);
        return NULL;
    }
    return cv;
}

TEST(config_tests, config) {
    set_log_level(LOG_LVL_DEBUG);

    config_t* cfg = json_config_new(TEST_JSON_FN);
    if(cfg == NULL) {
        FAIL() << "Failed to load configuration";
    }

    // Test getting all values
    config_value_t* integer = helper_get(cfg, INT_KEY);
    if(integer == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(integer->type, CVT_INTEGER);
    ASSERT_EQ(integer->body.integer, INT_VAL);
    config_value_destroy(integer);

    config_value_t* floating = helper_get(cfg, FLT_KEY);
    if(floating == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(floating->type, CVT_FLOATING);
    ASSERT_EQ(floating->body.floating, FLT_VAL);
    config_value_destroy(floating);

    config_value_t* string = helper_get(cfg, STR_KEY);
    if(string == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(string->type, CVT_STRING);
    ASSERT_STREQ(string->body.string, STR_VAL);
    config_value_destroy(string);

    config_value_t* boolean = helper_get(cfg, BLN_KEY);
    if(boolean == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(boolean->type, CVT_BOOLEAN);
    ASSERT_EQ(boolean->body.boolean, BLN_VAL);
    config_value_destroy(boolean);

    config_value_t* empty = helper_get(cfg, NIL_KEY);
    if(empty == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(empty->type, CVT_NONE);
    config_value_destroy(empty);

    config_value_t* array = helper_get(cfg, ARR_KEY);
    if(array == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(array->type, CVT_ARRAY);
    ASSERT_EQ(config_value_array_len(array), ARR_LEN);
    config_value_t* arr_item = config_value_array_get(array, 0);
    if(arr_item == NULL) { FAIL() << "Failed to get element 0 from array"; }
    ASSERT_EQ(arr_item->type, CVT_STRING);
    ASSERT_STREQ(arr_item->body.string, STR_VAL);
    config_value_destroy(arr_item);
    config_value_destroy(array);

    config_value_t* object = helper_get(cfg, OBJ_KEY);
    if(object == NULL) { FAIL() << "Failed to get config value"; }
    ASSERT_EQ(object->type, CVT_OBJECT);
    config_value_t* obj_item = config_value_object_get(object, INT_KEY);
    if(obj_item == NULL) {
        LOG_ERROR("Failed to get element \"%s\" from object", INT_KEY);
        FAIL() << "Failed to get element";
    }
    ASSERT_EQ(obj_item->type, CVT_INTEGER);
    ASSERT_EQ(obj_item->body.integer, INT_VAL);
    config_value_destroy(obj_item);
    config_value_destroy(object);

    config_destroy(cfg);
}
