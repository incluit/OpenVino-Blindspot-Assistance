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
 * @brief Env Config interface
 */

#ifndef _EIS_ENV_CONFIG_H
#define _EIS_ENV_CONFIG_H

#include <cjson/cJSON.h>
#include <eis/config_manager/config_manager.h>
#include "eis/utils/json_config.h"
#include "eis/utils/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

    /**
     * get_topics_from_env function gives the topics with respect to publisher/subscriber.
     *
     * @param topic_type       - Topic type. Either pub or sub
     * @return char**          - topics returned from env config based on topic type (PUB/SUB). 
     *                           topic_type is case insensitive.
     *                           Eg: pub/sub, PUB/SUB, Pub/Sub.
     */

    char** (*get_topics_from_env)(const char* topic_type);

    /**
     * get_messagebus_config function gives the configuration of the pub/sub topic to communicate over EIS messagebus
     *
     * @param configmgr       - Config Manager object
     * @param topic[]           -C string array having list of topics for which msg bus config needs to be constructed.
     *                          In case the topic is being published, it will be the stream name like `camera1_stream`
     *                          and in case the topic is being subscribed, it will be of the format
     *                          `[Publisher_AppName]/[stream_name]`.
     *                          Eg: `VideoIngestion/camera1_stream`
     * @param num_of_topics    - num of topics
     * @param topic_type      - TopicType for which msg bus config needs to be constructed (PUB/SUB, SERVER/CLIENT). 
     *                          topic_type is case insensitive.
     *                          Eg: pub/sub, PUB/SUB, Pub/Sub, server/client, SERVER/CLIENT, Server/Client.
     * @return config_t*      - JSON msg bus config of type config_t
     */
    config_t* (*get_messagebus_config)(const config_mgr_t* configmgr, char* topic[], size_t num_of_topics, const char* topic_type);

    /**
     * get_topics_count function gives the number of topics returned by the other Env_config API "get_topics_from_env"
     *
     * @param topics[]       -  An array of C strings provided by get_messagebus_config
     * @return size_t      -    Number of topics returned by by get_messagebus_config API (type is unsigned int)
     */
    size_t (*get_topics_count)(char* topics[]);

    /**
     * trim function removes white spaces around the string value
     *
     * @param str_value       - str_value will be trimmed of from white spaces
     */
    void (*trim)(char* str_value);

    /**
     * free_mem function is to de-allocate the memory of char**
     *
     * @param arr       - char** variable that needs memory deallocation
     */
    void (*free_mem)(char** arr);

} env_config_t;

/**
 * env_config_destroy function to delete the env_config_client instance
 * @param env_config_t*     - env_config_client object
 */
void env_config_destroy(env_config_t* env_config);

/**
 * env_config_new function to creates a new env config client
 * @return env_config_t     - env_config object
 */
env_config_t* env_config_new();

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
