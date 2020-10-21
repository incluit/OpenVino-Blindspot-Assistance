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
 * @brief Config Manager interface
 */

#ifndef _EIS_CONFIG_MGR_H
#define _EIS_CONFIG_MGR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /**
     * get_config function gets the value of a key from config manager
     *
     * @param key       - key to be queried on from config manager
     * @return char*    - values returned from config manager based on key
     */
    char* (*get_config)(char *key);

    /**
     *To save a value to config manager if key already exists, if not create/store a new set of key-value to config manager
     * @param key      - key in config manager to set
     * @param value    - value to set the key to
     * @return int     - 0 on success, -1 on failure

    */
    int (*put_config)(char *key, char *value);

    /**
     * register_watch_key function registers to a callback and keeps a watch on a specified key
     * 
     * @param key                                                 - key to keep a watch on
     * @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event 
     *                                                              with updated value on the respective key
     */
    void (*register_watch_key)(
            char *key, 
            void (*register_watch_dir_cb)(char *key, char *value));

    /**
     * register_watch_dir function registers to a callback and keeps a watch on the prefix of a specified key
     *
     * @param key                                                 - prefix of a key to keep a watch on
     * @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event
     *                                                              with updated value on the respective key
     */
    void (*register_watch_dir)(
            char *key, 
            void (*register_watch_dir_cb)(char *key, char *value));
} config_mgr_t;

/**
 * config_mgr_new function to creates a new config manager client
 *  @param storage_type      - Type of key-value storage, Eg. etcd
 *  @param cert_file         - config manager client cert file
 *  @param key_file          - config manager client key file
 *  @param ca_cert           - config manager client ca cert
 *  @return NULL for any errors occured or config_mg_t* on successful
 */
config_mgr_t* config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert);

/**
 * Destroy config_mgr_t* object.
 *
 * @param config_mgr_config - Config manager's configuration to destroy
 */
void config_mgr_config_destroy(config_mgr_t *config_mgr_config);
#ifdef __cplusplus
} //end extern "C"
#endif

#endif
