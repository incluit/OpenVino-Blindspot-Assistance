// Copyright (c) 2019 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or // sell copies of the Software, and to permit persons to whom the Software is
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
 * @brief JSON configuration file utility implementation
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "eis/utils/json_config.h"
#include "eis/utils/logger.h"


// prototypes
config_value_t* json_to_cvt(cJSON* obj);

config_t* json_config_new(const char* config_file) {
    char* buffer = NULL;
    FILE* fp = fopen(config_file, "r");
    if(fp == NULL) {
        LOG_ERROR("JSON file '%s' does not exist", config_file);
        goto err;
    }

    // Seek back to the beginning
    fseek(fp, 0, SEEK_END);

    // Get file length
    long file_len = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    // Initialize buffer
    buffer = (char*) malloc(sizeof(char) * (file_len + 1));
    if(buffer == NULL) {
        LOG_ERROR_0("Out of memory initializing buffer");
        goto err;
    }

    // Read all of file into buffer and close file
    fread(buffer, sizeof(char), file_len, fp);
    fclose(fp);
    fp = NULL;
    buffer[file_len] = '\0';

    // Parse as JSON
    cJSON* json = cJSON_Parse(buffer);
    if(json == NULL) {
        LOG_ERROR("Failed to parse JSON file: %s", cJSON_GetErrorPtr());
        goto err;
    }

    // Create configuration object
    config_t* config = config_new(
            (void*) json, free_json, get_config_value);
    if(config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

    // Free buffer
    free(buffer);

    return config;

err:
    if(fp != NULL)
        fclose(fp);
    if(buffer != NULL)
        free(buffer);
    return NULL;
}

config_t* json_config_new_from_buffer(const char* buffer) {
    // Parse as JSON
    cJSON* json = cJSON_Parse(buffer);
    if(json == NULL) {
        LOG_ERROR("Failed to parse JSON file: %s", cJSON_GetErrorPtr());
        return NULL;
    }

    // Create configuration object
    config_t* config = config_new(
            (void*) json, free_json, get_config_value);
    if(config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        return NULL;
    }

    return config;
}

void free_json(void* ctx) {
    cJSON* json = (cJSON*) ctx;
    cJSON_Delete(json);
}

config_value_t* get_array_item(const void* array, int idx) {
    cJSON* json = (cJSON*) array;
    cJSON* item = cJSON_GetArrayItem(json, idx);
    if(item == NULL) {
        LOG_ERROR("No item at index '%d' in JSON array", idx);
        return NULL;
    }
    return json_to_cvt(item);
}

config_value_t* get_config_value(const void* o, const char* key) {
    if(o == NULL) {
        LOG_ERROR_0("Configuration object given is NULL");
        return NULL;
    }

    cJSON* json = (cJSON*) o;
    cJSON* obj = cJSON_GetObjectItem(json, key);
    if(obj == NULL) {
        LOG_WARN("JSON does not contain key: %s", key);
        return NULL;
    }
    return json_to_cvt(obj);
}

// Helper function to convert cJSON object to config_value_t structure.
config_value_t* json_to_cvt(cJSON* obj) {
    config_value_t* config_value = NULL;

    if(cJSON_IsNumber(obj)) {
        double value = obj->valuedouble;

        if(value == (int64_t) value) {
            config_value = config_value_new_integer((int64_t) value);
        } else {
            config_value = config_value_new_floating(value);
        }
    } else if(cJSON_IsString(obj)) {
        config_value = config_value_new_string(obj->valuestring);
    } else if(cJSON_IsBool(obj)) {
        if(cJSON_IsTrue(obj)) {
            config_value = config_value_new_boolean(true);
        } else {
            config_value = config_value_new_boolean(false);
        }
    } else if(cJSON_IsObject(obj)) {
        // No need to have a free method, since all of the JSON references will
        // be freed when the config is freed
        config_value = config_value_new_object(
                (void*) obj, get_config_value, NULL);
    } else if(cJSON_IsArray(obj)) {
        // No need to have a free method, since all of the JSON references will
        // be freed when the config is freed
        config_value = config_value_new_array(
                (void*) obj, cJSON_GetArraySize(obj), get_array_item, NULL);
    } else if(cJSON_IsNull(obj)) {
        config_value = config_value_new_none();
    }

    return config_value;
}
