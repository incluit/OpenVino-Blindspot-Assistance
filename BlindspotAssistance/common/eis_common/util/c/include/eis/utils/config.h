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
 * @brief EIS configuration interface
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#ifndef _EIS_CONFIG_IFACE_H
#define _EIS_CONFIG_IFACE_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Valid configuration value types
 */
typedef enum {
    CVT_INTEGER  = 0,
    CVT_FLOATING = 1,
    CVT_STRING   = 2,
    CVT_BOOLEAN  = 3,
    CVT_OBJECT   = 4,
    CVT_ARRAY    = 5,
    CVT_NONE     = 6,
} config_value_type_t;

// Forward declaration of config_value_t struct
typedef struct _config_value config_value_t;

/**
 * Config value object representation. Includes method for freeing the object
 * when the caller that obtained the object is finished with it.
 */
typedef struct {
    void* object;
    config_value_t* (*get)(const void* obj, const char* key);
    void (*free)(void* object);
} config_value_object_t;

/**
 * Config value array representation. Includes methods for getting elements
 * at a given index and for freeing the array.
 */
typedef struct {
    void* array;
    size_t length;
    config_value_t* (*get)(const void* array, int idx);
    void (*free)(void* array);
} config_value_array_t;

/**
 * Structure representing a configuration value.
 */
typedef struct _config_value {
    config_value_type_t type;

    union {
        int64_t      integer;
        double       floating;
        char*        string;
        bool         boolean;
        config_value_object_t* object;
        config_value_array_t*  array;
    } body;
} config_value_t;

/**
 * Configuration object
 */
typedef struct {
    void* cfg;
    void (*free)(void*);
    config_value_t* (*get_config_value)(const void*,const char*);
} config_t;

/**
 * Create a new configuration object.
 *
 * @param cfg              - Configuration context
 * @param free_fn          - Method to free the configuration context
 * @param get_config_value - Method to retrieve a key from the configuration
 * @return config_t, or NULL if an error occurs
 */
config_t* config_new(
        void* cfg, void (*free_fn)(void*),
        config_value_t* (*get_config_value)(const void*,const char*));

/**
 * Get value from configuration object.
 *
 * \note Returns NULL if the value cannot be found in the configuration object.
 *
 * @param config - Configuration object pointer
 * @param key    - Key for the configuration value
 * @return @c config_value_t
 */
config_value_t* config_get(const config_t* config, const char* key);

/**
 * Destroy the configuration object.
 *
 * @param config - Configuration to destroy
 */
void config_destroy(config_t* config);

/**
 * Retreive a configuration value from a configuration value object.
 *
 * \note The obj parameter must be a @c CVT_OBJECT type, NULL will be returned
 *       if it is not.
 *
 * @param obj    - Configuration value object
 * @param key    - Key to retrieve from the configuration object
 * @return config_value_t
 */
config_value_t* config_value_object_get(
        const config_value_t* obj, const char* key);

/**
 * Retreive a configuration value from a configuration array.
 *
 * \note The arr parameter mustb be of type @c CVT_ARRAY, otherwise NULL will
 *       be returned.
 *
 * @param arr    - Array configuration value
 * @param idx    - Index in the array to retrieve
 * @return config_value_t*
 */
config_value_t* config_value_array_get(const config_value_t* arr, int idx);

/**
 * Get the length of a configuration array.
 *
 * @param arr - Array configuration value
 * @return int
 */
size_t config_value_array_len(const config_value_t* arr);

/**
 * Helper function to create a new config_value_t pointer to the given integer
 * value.
 *
 * @param value - Integer value
 * @return config_value_t*
 */
config_value_t* config_value_new_integer(int64_t value);

/**
 * Helper to create a new config_value_t pointer to the given double value.
 *
 * @param value - Floating point value
 * @return config_value_t*
 */
config_value_t* config_value_new_floating(double value);

/**
 * Helper function to create a new config_value_t pointer to the given string
 * value.
 *
 * @param value - String value
 * @return config_value_t*
 */
config_value_t* config_value_new_string(const char* value);

/**
 * Helper function to create a new config_value_t pointer to the given boolean
 * value.
 *
 * @param value - Boolean value
 * @return config_value_t*
 */
config_value_t* config_value_new_boolean(bool value);

/**
 * Helper function to create a new config_value_t pointer to the given
 * configuration object value.
 *
 * @param value    - Object value
 * @param free_fn  - Free method for the object value
 * @return config_value_t*
 */
config_value_t* config_value_new_object(
        void* value, config_value_t* (*get)(const void*,const char*),
        void (*free_fn)(void*));

/**
 * Helper function to create a new config_value_t pointer to the given array
 * value.
 *
 * @param array   - Pointer to array context
 * @param length  - Array length
 * @param get     - Get method for getting an element in the array
 * @param free_fn - Function to free the array object
 * @return config_value_t*
 */
config_value_t* config_value_new_array(
        void* array, size_t length, config_value_t* (*get)(const void*,int),
        void (*free_fn)(void*));

/**
 * Helper function to create a new config_value_t pointer to an empty
 * configuration value.
 *
 * @return config_value_t*
 */
config_value_t* config_value_new_none();

/**
 * Destroy a configuration value.
 *
 * @param value - Configuration value to destroy
 */
void config_value_destroy(config_value_t* value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _EIS_CONFIG_IFACE_H
