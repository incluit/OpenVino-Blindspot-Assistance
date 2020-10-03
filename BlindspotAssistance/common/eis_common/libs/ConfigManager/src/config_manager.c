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
 * @brief Config Manager implementation
 */

#include "eis/config_manager/config_manager.h"
#include "cgo/go_config_manager.h"
#include <ctype.h>
#include <eis/utils/logger.h>

char supported_storage_types[][5] = {"etcd"};

static char* get_config(char *key){
    return getConfig(key);
}

static int put_config(char *key, char* value){
    return putConfig(key, value);
}

static void register_watch_key(char *key, callback_fcn user_callback){
    registerWatchKey(key, user_callback);
}

static void register_watch_dir(char *key, callback_fcn user_callback){
    registerWatchDir(key, user_callback);
}


config_mgr_t* config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert){
    for(int i = 0; i < sizeof(supported_storage_types) / sizeof(supported_storage_types[0]); i++){
        if (!strcmp(storage_type, supported_storage_types[i])){
            break;
        }
        return NULL;
    }
    int status = initialize(storage_type,
                            cert_file,
                            key_file,
                            ca_cert);
    if(status == -1) {
        return NULL;
    }
    config_mgr_t *config_mgr = (config_mgr_t *)malloc(sizeof(config_mgr_t));
    if(config_mgr == NULL) {
        LOG_ERROR_0("Malloc failed for Config_mgr_t");
        return NULL;
    }
    config_mgr->get_config = get_config;
    config_mgr->put_config = put_config;
    config_mgr->register_watch_key = register_watch_key;
    config_mgr->register_watch_dir = register_watch_dir;
    return config_mgr;
}

void config_mgr_config_destroy(config_mgr_t *config_mgr_config){
    if(config_mgr_config != NULL)
        free(config_mgr_config);
}
