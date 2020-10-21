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
 * @brief EIS configuration interface implementation
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#include <string.h>
#include <safe_lib.h>

#include "eis/utils/config.h"
#include "eis/utils/logger.h"

config_t* config_new(
        void* cfg, void (*free_fn)(void*),
        config_value_t* (*get_config_value)(const void*,const char*)) {
    if(cfg != NULL && free_fn == NULL) {
        LOG_ERROR_0("Free method not specified for cfg object");
        return NULL;
    } else if(cfg == NULL && free_fn != NULL) {
        LOG_ERROR_0("Free method specified for NULL cfg");
        return NULL;
    }

    config_t* config = (config_t*) malloc(sizeof(config_t));
    if(config == NULL) {
        LOG_ERROR_0("config malloc failed");
        return NULL;
    }

    config->cfg = cfg;
    config->free = free_fn;
    config->get_config_value = get_config_value;

    return config;
}

config_value_t* config_get(const config_t* config, const char* key) {
    return config->get_config_value(config->cfg, key);
}

config_value_t* config_value_object_get(
        const config_value_t* obj, const char* key)
{
    if(obj->type != CVT_OBJECT) {
        LOG_ERROR_0("Attempted to retrieve value from non CVT_OBJECT value");
        return NULL;
    }

    config_value_t* cv = obj->body.object->get(obj->body.object->object, key);
    return cv;
}

config_value_t* config_value_array_get(const config_value_t* arr, int idx) {
    if(arr->type != CVT_ARRAY) {
        LOG_ERROR_0("Attempted to retrieve value from non CVT_ARRAY value");
        return NULL;
    }

    config_value_t* cv = arr->body.array->get(arr->body.array->array, idx);
    return cv;
}

size_t config_value_array_len(const config_value_t* arr) {
    if(arr->type != CVT_ARRAY) {
        LOG_ERROR_0("Attempted to retrieve value from non CVT_ARRAY value");
        return 0;
    }
    return arr->body.array->length;
}

void config_destroy(config_t* config) {
    if(config->cfg != NULL) {
        config->free(config->cfg);
    }
    free(config);
}

config_value_t* config_value_new_integer(int64_t value) {
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_INTEGER;
    cv->body.integer = value;
    return cv;
}

config_value_t* config_value_new_floating(double value) {
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_FLOATING;
    cv->body.floating = value;
    return cv;
}

config_value_t* config_value_new_string(const char* value) {
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    size_t len = strlen(value);

    cv->type = CVT_STRING;
    cv->body.string = (char*) malloc(sizeof(char) * (len + 1));
    memcpy_s(cv->body.string, len, value, len);
    cv->body.string[len] = '\0';

    return cv;
}

config_value_t* config_value_new_boolean(bool value) {
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_BOOLEAN;
    cv->body.boolean = value;

    return cv;
}

config_value_t* config_value_new_object(
        void* value, config_value_t* (*get)(const void*,const char*),
        void (*free_fn)(void*))
{
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_OBJECT;
    cv->body.object = (config_value_object_t*) malloc(
            sizeof(config_value_object_t));
    if(cv->body.object == NULL) {
        LOG_ERROR_0("Out of memory creating config object wrapper");
        free(cv);
        return NULL;
    }

    cv->body.object->object = value;
    cv->body.object->get = get;
    cv->body.object->free = free_fn;

    return cv;
}

config_value_t* config_value_new_array(
        void* array, size_t length, config_value_t* (*get)(const void*,int),
        void (*free_fn)(void*))
{
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_ARRAY;
    cv->body.array = (config_value_array_t*) malloc(
            sizeof(config_value_array_t));
    if(cv->body.array == NULL) {
        LOG_ERROR_0("Out of memory creating config array wrapper");
        free(cv);
        return NULL;
    }

    cv->body.array->array = array;
    cv->body.array->length = length;
    cv->body.array->get = get;
    cv->body.array->free = free_fn;

    return cv;
}

config_value_t* config_value_new_none() {
    config_value_t* cv = (config_value_t*) malloc(sizeof(config_value_t));
    if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
    }

    cv->type = CVT_NONE;

    return cv;
}

void config_value_destroy(config_value_t* value) {
    if(value == NULL) return;

    if(value->type == CVT_OBJECT) {
        if(value->body.object->free != NULL)
            value->body.object->free(value->body.object->object);
        free(value->body.object);
    } else if(value->type == CVT_ARRAY) {
        if(value->body.array->free != NULL)
            value->body.array->free(value->body.array->array);
        free(value->body.array);
    } else if(value->type == CVT_STRING) {
        free(value->body.string);
    }

    // NOTE: Above if-else-if block ignores all CVT_* values which do not have
    // any additional data which needs to be freed

    free(value);
}
