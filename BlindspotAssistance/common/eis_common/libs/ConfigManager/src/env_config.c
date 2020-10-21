// Copyright (c) 2020 Intel Corporation.
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
 * @brief Env Config implementation
 */

#include "eis/config_manager/env_config.h"
#include <safe_str_lib.h>

#define PUBLIC_PRIVATE_KEY_SIZE 500
#define PUB "PUB"
#define SUB "SUB"
#define SERVER_ENV "Server"
#define CLIENTS_ENV "Clients"
#define PUBTOPICS_ENV "PubTopics"
#define SUBTOPICS_ENV "SubTopics"
#define DEV_MODE_ENV "DEV_MODE"
#define REQUEST_EP "RequestEP"
#define APPNAME_ENV "AppName"
#define ZMQ_RECV_HWM_ENV "ZMQ_RECV_HWM"
#define CFG "_cfg"
#define SOCKET_FILE "socket_file"
#define SERVER "SERVER"
#define CLIENT "CLIENT"

// Forward declaration
void trim(char* str_value);

static void free_mem(char** arr) {
    int k = 0;
    while (arr[k] != NULL) {
        free(arr[k]);
        k++;
    }
    free(arr);
}

static void to_upper(char arr[]) {
    // to_upper is called to maintain case-insensitive for topic_type by converting every 
    // character of a string to upper case.
    int j = 0;
    while (arr[j] != '\0') {
        if (arr[j] >= 'a' && arr[j] <= 'z') {
            arr[j] = arr[j] - 32;
        }
        j++;
    }
}

static char** get_topics_from_env(const char* topic_type) {
    char* env_topics = NULL;
    char* individual_topic = NULL;
    char* topics_env = NULL;
    char* ref_ptr = NULL;
    char* str = NULL;
    int ret = 0;
    int j = 0;
    size_t topic_type_len = strlen(topic_type);
    char topic_type_arr[topic_type_len +1];
    ret = strncpy_s(topic_type_arr, topic_type_len + 1, topic_type, topic_type_len);
    if (ret != 0) {
        LOG_ERROR("String copy failed (errno: %d): Failed to copy topic_type %s", ret, topic_type);
        return NULL;
    }

    to_upper(topic_type_arr);

    if (!strcmp(topic_type_arr, PUB)) {
        topics_env = PUBTOPICS_ENV;
    } else if (!strcmp(topic_type_arr, SUB)) {
        topics_env = SUBTOPICS_ENV;
    } else {
        LOG_ERROR("topic type: %s is not supported", topic_type_arr);
        return NULL;
    }
    env_topics = getenv(topics_env);
    if (env_topics == NULL) {
        LOG_ERROR("getenv failed for topics %s\n", topics_env);
        return NULL;
    }

    size_t env_len = strlen(env_topics);
    char topic_list[env_len + 1];
    ret = strncpy_s(topic_list, env_len + 1, env_topics, env_len);
    if (ret != 0) {
        LOG_ERROR("String copy failed (errno: %d): Failed to copy env_topics %s", ret, env_topics);
        return NULL;
    }

    char **topics = (char **)calloc(env_len + 1, sizeof(char*));
    if (topics == NULL) {
        LOG_ERROR_0("Calloc failed for topics");
        return NULL;
    }
    size_t individual_topic_len;
    for (j = 0, str = topic_list; ; str = NULL, j++) {
        individual_topic = strtok_r(str, ",", &ref_ptr);
        if (individual_topic == NULL) {
            break;
        }
        individual_topic_len = strlen(individual_topic);
        if (topics[j] == NULL) {
            topics[j] = (char*) malloc(individual_topic_len + 1);
        }
        if (topics[j] == NULL) {
            LOG_ERROR_0("Malloc failed for individual topics");
            free_mem(topics);
            return NULL;
        }
        ret = strncpy_s(topics[j], individual_topic_len + 1, individual_topic, individual_topic_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy individual_topic- \" %s \"to topics", ret, individual_topic);
            free_mem(topics);
            return NULL;
        }
    }
    topics[j] = NULL;
    // freeing up the "char** topics" memory has to be done from caller's (application's) side, with the help of free_mem() api.
    // User/caller has to free the topics after its usage.
    /* usage:
       free_mem(topics);
    */
    return topics;
}

static size_t get_topics_count(char* topics[]) {
    size_t i = 0;
    size_t topic_count = 0;
    while (topics[i++] != NULL) {
        ++topic_count;
    }
    return topic_count;
}

static config_t* get_messagebus_config(const config_mgr_t* configmgr, char* topic[], size_t num_of_topics, const char* topic_type) {
    config_t* config = NULL;
    char publicKey[PUBLIC_PRIVATE_KEY_SIZE] = {0, };
    char app_private_key[PUBLIC_PRIVATE_KEY_SIZE] = {0, };
    char* data = NULL;
    char* publisher = NULL;
    char* temp_topic = NULL;
    char* address = NULL;
    char* mode = NULL;
    char* host = NULL;
    char* port = NULL;
    char* str = NULL;
    char* clients_env = NULL;
    char zmq_hwm[] = {0, 1};
    size_t data_len;
    size_t zmq_recv_hwm_len;
    int ret = 0;
    int i = 0;
    int j;

    char** mode_address = NULL;
    char** host_port = NULL;
    char** allowed_clients = NULL;
    char** pub_topic = NULL;

    size_t topic_type_len = strlen(topic_type);
    char topic_type_arr[topic_type_len +1];
    ret = strncpy_s(topic_type_arr, topic_type_len + 1, topic_type, topic_type_len);
    if (ret != 0) {
        LOG_ERROR("String copy failed (errno: %d): Failed to copy topic_type %s", ret, topic_type);
        return NULL;
    }
    to_upper(topic_type_arr);

    size_t cfg_len = strlen(CFG);
    size_t topic_arr_len = strlen(topic[0]);
    char topic_arr[topic_arr_len + 1];
    char* zmq_recv_hwm = getenv(ZMQ_RECV_HWM_ENV);
    if (zmq_recv_hwm == NULL) {
        LOG_WARN("getenv failed for zmq_recv_hwm %s\n", ZMQ_RECV_HWM_ENV);
    } else {
        trim(zmq_recv_hwm);
        zmq_recv_hwm_len = strlen(zmq_recv_hwm);
        zmq_hwm[zmq_recv_hwm_len + 1];
    }

    char* dev_mode_env = getenv(DEV_MODE_ENV);
    if (dev_mode_env == NULL) {
        LOG_ERROR("getenv failed for dev_mode_env %s\n", DEV_MODE_ENV);
        goto err;
    }
    trim(dev_mode_env);

    const char* app_name_env = getenv(APPNAME_ENV);
    if (app_name_env == NULL) {
        LOG_ERROR("getenv failed for app_name_env %s\n", APPNAME_ENV);
        goto err;
    }
    trim(app_name_env);

    bool dev_mode = false;
    if (!strcmp(dev_mode_env, "true")) {
        dev_mode = true;
    } else if (!strcmp(dev_mode_env, "false")) {
        dev_mode = false;
    } else {
        LOG_WARN("%s env is not set to true or false, so using false as default", DEV_MODE_ENV);
    }
    char* topic_cfg = NULL;
    char topic_cfg_arr[]={0, 1};
    size_t topic_cfg_len;
    if (!strcmp(topic_type_arr, SUB)) { 
        char* individual_topic;
        ret = strncpy_s(topic_arr, topic_arr_len + 1, topic[0], topic_arr_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic \" %s \" to topic_arr", ret, topic[0]);
            goto err;
        }
        temp_topic = topic_arr;
        pub_topic = (char **)calloc(strlen(topic_arr) + 1, sizeof(char*));
        if (pub_topic == NULL) {
            LOG_ERROR_0("Calloc failed for pub_topic");
            goto err;
        }
        j = 0;
        size_t individual_topic_len;
        while ((individual_topic = strtok_r(temp_topic, "/", &temp_topic))) {
            individual_topic_len = strlen(individual_topic);
            if (pub_topic[j] == NULL) {
                pub_topic[j] = (char*) malloc(individual_topic_len + 1);
            }
            if (pub_topic[j] == NULL) {
                LOG_ERROR_0("Malloc failed for individual pub_topic");
                goto err;
            }
            ret = strncpy_s(pub_topic[j], individual_topic_len + 1, individual_topic, individual_topic_len);
            if (ret != 0) {
                LOG_ERROR("String copy failed (errno: %d): Failed to copy individual_topic \" %s \" to pub_topic", ret, individual_topic);
                goto err;
            }
            trim(pub_topic[j]);
            j++;
        }

        if (j == 1 || j > 2) {
            LOG_ERROR("sub topic should be of the format [AppName]/[stream_name]");
            goto err;
        }
        publisher = pub_topic[0];
        LOG_DEBUG("publisher: %s", publisher);
        size_t publisher_topic_len = strlen(pub_topic[1]);
        char publisher_topic[publisher_topic_len + cfg_len + 1];
        ret = strncpy_s(publisher_topic, publisher_topic_len + 1 , pub_topic[1], publisher_topic_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy pub_topic[1] \" %s \" to publisher_topic", ret, pub_topic[1]);
            goto err;
        }
        ret = strncat_s(publisher_topic, publisher_topic_len + cfg_len + 1, CFG, cfg_len);
        if (ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate cfg to publisher_topic \" %s \" ", ret, publisher_topic);
            goto err;
        }
        LOG_DEBUG("publisher_topic: %s", publisher_topic);
        topic_cfg = getenv(publisher_topic);
        if (topic_cfg == NULL) {
            LOG_ERROR("getenv failed for publisher_topic %s\n", publisher_topic);
            goto err;
        }
        topic_cfg_len = strlen(topic_cfg);
        topic_cfg_arr[topic_cfg_len + 1];

        ret = strncpy_s(topic_cfg_arr, topic_cfg_len + 1, topic_cfg, topic_cfg_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic_cfg \" %s \" to topic_cfg_arr", ret, topic_cfg);
            goto err;
        }
    } else if (!strcmp(topic_type_arr, PUB)) {
        char publisher_topic[topic_arr_len + cfg_len + 1];
        ret = strncpy_s(publisher_topic, topic_arr_len + 1, topic[0], topic_arr_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic \" %s \" to publisher_topic", ret, topic[0]);
            goto err;
        }
        trim(publisher_topic);
        ret = strncat_s(publisher_topic, topic_arr_len + cfg_len + 1, CFG, cfg_len);
        if (ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d):  Failed to concatenate cfg to publisher_topic: \" %s \" ", ret, publisher_topic);
            goto err;
        }
        LOG_DEBUG("publisher_topic: %s", publisher_topic);
        topic_cfg = getenv(publisher_topic);
        if (topic_cfg == NULL) {
            LOG_ERROR("getenv failed for publisher_topic %s\n", publisher_topic);
            goto err;
        }
        topic_cfg_len = strlen(topic_cfg);
        topic_cfg_arr[topic_cfg_len + 1];
        ret = strncpy_s(topic_cfg_arr, topic_cfg_len + 1, topic_cfg, topic_cfg_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic_cfg \" %s \" env value to topic_cfg_arr", ret, topic_cfg);
            goto err;
        }
    } else if (!strcmp(topic_type_arr, SERVER)) {
        topic_cfg = getenv(SERVER_ENV);
        if (topic_cfg == NULL) {
            LOG_ERROR("getenv failed for server %s\n", SERVER);
            goto err;
        }
        topic_cfg_len = strlen(topic_cfg);
        topic_cfg_arr[topic_cfg_len + 1];
        ret = strncpy_s(topic_cfg_arr, topic_cfg_len + 1, topic_cfg, topic_cfg_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy server env \" %s \" value to topic_cfg_arr", ret, topic_cfg);
            goto err;
        }
    } else if (!strcmp(topic_type_arr, CLIENT)) {
        char publisher_topic[topic_arr_len + cfg_len + 1];
        ret = strncpy_s(publisher_topic, topic_arr_len + 1, topic[0], topic_arr_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic \" %s \" to publisher_topic", ret, topic[0]);
            goto err;
        }
        trim(publisher_topic);
        ret = strncat_s(publisher_topic, topic_arr_len + cfg_len + 1, CFG, cfg_len);
        if (ret != 0) {
            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate cfg to publisher_topic: \" %s \" ", ret, publisher_topic);
            goto err;
        }
        LOG_DEBUG("publisher_topic: %s", publisher_topic);
        topic_cfg = getenv(publisher_topic);
        if (topic_cfg == NULL) {
            LOG_ERROR("getenv failed for publisher_topic %s\n", publisher_topic);
            goto err;
        }
        topic_cfg_len = strlen(topic_cfg);
        topic_cfg_arr[topic_cfg_len + 1];

        ret = strncpy_s(topic_cfg_arr, topic_cfg_len + 1, topic_cfg, topic_cfg_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy topic_cfg env value \" %s \" to topic_cfg_arr", ret, topic_cfg);
            goto err;
        }
    } else {
        LOG_ERROR("topic type: %s is not supported", topic_type_arr);
        goto err;
    }

    mode_address = (char **)calloc(topic_cfg_len + 1, sizeof(char*));
    if (mode_address == NULL) {
        LOG_ERROR_0("Calloc failed for mode_address");
        goto err;
    }
    char* ref_ptr = NULL;
    char* str_topic =  NULL;
    for ( i = 0, str_topic = topic_cfg_arr; ; str_topic = NULL, i++) { 
        data = strtok_r(str_topic, ",", &ref_ptr);
        if (data == NULL) {
            break;
        }
        data_len = strlen(data);
        if (mode_address[i] == NULL) {
            mode_address[i] = (char*) calloc(data_len + 1, sizeof(char));
        }
        if (mode_address[i] == NULL) {
            LOG_ERROR_0("Calloc failed for individual mode_address");
            goto err;
        }
        ret = strncpy_s(mode_address[i], data_len + 1, data, data_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d): Failed to copy data \" %s \" to mode_address", ret, data);
            goto err;
        }
    }
    if (mode_address[2] == NULL) {
        LOG_DEBUG_0("socket file not explicitly given by application");
    }
    else {
        LOG_DEBUG_0("socket file explicitly given by application");
    }
    mode = mode_address[0];
    trim(mode);
    address = mode_address[1];
    trim(address);
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "type", mode);
    if (zmq_recv_hwm != NULL) {
        ret = strncpy_s(zmq_hwm, zmq_recv_hwm_len + 1, zmq_recv_hwm, zmq_recv_hwm_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d) :  Failed to copy zmq_recv_hwm \" %s \" ", ret, zmq_recv_hwm);
            goto err;
        }
        cJSON_AddNumberToObject(json, "ZMQ_RECV_HWM", atoi(zmq_hwm));
    }

    host_port = (char **)calloc(strlen(address) + 1, sizeof(char*));
    if (host_port == NULL) {
        LOG_ERROR_0("Calloc failed for host_port");
        goto err;
    }
    if (!strcmp(mode, "zmq_tcp")) {
        i = 0;
        while ((data = strtok_r(address, ":", &address))) {
            data_len = strlen(data);
            if (host_port[i] == NULL) {
                host_port[i] = (char*) malloc(data_len + 1);
            }
            if (host_port[i] == NULL) {
                LOG_ERROR_0("Malloc failed for individual host_port");
                goto err;
            }
            ret = strncpy_s(host_port[i], data_len + 1, data, data_len);
            if (ret != 0) {
                LOG_ERROR("String copy failed (errno: %d) : Failed to copy data \" %s \" to host_port", ret, data);
                goto err;
            }
            i++;
        }
        host = host_port[0];
        trim(host);
        port = host_port[1];
        trim(port);

        __int64_t i_port = atoi(port);
        size_t clients_env_len;
        if (!strcmp(topic_type_arr, PUB)) {
            cJSON* zmq_tcp_config = cJSON_CreateObject();
            cJSON_AddItemToObject(json, "zmq_tcp_publish", zmq_tcp_config);
            cJSON_AddStringToObject(zmq_tcp_config, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_config, "port", i_port);

            if (!dev_mode) {
                clients_env = getenv(CLIENTS_ENV);
                if (clients_env == NULL) {
                    LOG_ERROR("getenv failed for clients_env %s\n", CLIENTS_ENV);
                    goto err;
                }
                clients_env_len = strlen(clients_env);
                char clients[clients_env_len + 1];
                ret = strncpy_s(clients, clients_env_len + 1, clients_env, clients_env_len);
                if (ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d) : Failed to copy clients_env \" %s \" ", ret, clients_env);
                    goto err;
                }
                char* refptr = NULL;
                allowed_clients = (char **)calloc(clients_env_len + 1, sizeof(char*));
                if (allowed_clients == NULL) {
                    LOG_ERROR_0("Calloc failed for allowed_clients");
                    goto err;
                }
                for (i = 0, str = clients; ; str = NULL, i++) {
                    data = strtok_r(str, ",", &refptr);
                    if (data == NULL) {
                       break;
                    }
                    data_len = strlen(data);
                    if (allowed_clients[i] == NULL) {
                        allowed_clients[i] = (char*) calloc(data_len + 1, sizeof(char));
                    }
                    if (allowed_clients[i] == NULL) {
                        LOG_ERROR_0("Calloc failed for individual allowed_clients");
                        goto err;
                    }
                    ret = strncpy_s(allowed_clients[i], data_len + 1, data, data_len);
                    if (ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d) : Failed to copy allowed clients \" %s \" ", ret, data);
                        goto err;
                    }
                    trim(allowed_clients[i]);
                }
                cJSON* all_clients = cJSON_CreateArray();
                for (j=0; j<i; j++) {
                    ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));
                    if (ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                        goto err;
                    }
                    ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, allowed_clients[j], strlen(allowed_clients[j]));
                    if (ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate client name \" %s \" to /Publickeys/", ret, allowed_clients[j]);
                        goto err;
                    }
                    char* client_pub_key = configmgr->get_config(publicKey);

                    if (strcmp(client_pub_key, "")) {
                        cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key));
                    }
                }

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate appname \" %s \" to \"/\"", ret, app_name_env);
                    goto err;
                }
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, "/private_key", strlen("/private_key"));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate /private_key to appname \" %s \" ", ret, app_private_key);
                    goto err;
                }
                const char* server_secret_key = configmgr->get_config(app_private_key);

                cJSON_AddStringToObject(zmq_tcp_config, "server_secret_key", server_secret_key);
                cJSON_AddItemToObject(json, "allowed_clients", all_clients);
            }

        }
        else if (!strcmp(topic_type_arr, SUB)) {
            cJSON* zmq_tcp_config = cJSON_CreateObject();
            cJSON_AddItemToObject(json, pub_topic[1], zmq_tcp_config);
            cJSON_AddStringToObject(zmq_tcp_config, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_config, "port", i_port);

            if (!dev_mode) {
                ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if (ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                    goto err;
                }
                ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, publisher, strlen(publisher));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate publisher's name \" %s \" to /Publickeys/", ret, publisher);
                    goto err;
                }

                const char* server_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(zmq_tcp_config, "server_public_key", server_pub_key);
                ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));
                if (ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d): Failed to copy /Publickeys/", ret);
                    goto err;
                }
                ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app name \" %s \" to /Publickeys/", ret, publicKey);
                    goto err;
                }

                const char* client_pub_key = configmgr->get_config(publicKey);
                cJSON_AddStringToObject(zmq_tcp_config, "client_public_key", client_pub_key);

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app name \" %s \" to /", ret, app_private_key);
                    goto err;
                }
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, "/private_key", strlen("/private_key"));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate /private_key to app name \" %s \" ", ret, app_private_key);
                    goto err;
                }

                const char* client_secret_key = configmgr->get_config(app_private_key);
                cJSON_AddStringToObject(zmq_tcp_config, "client_secret_key", client_secret_key);
            }

        }
        else if (!strcmp(topic_type_arr, SERVER)) {
            cJSON* zmq_tcp_config = cJSON_CreateObject();
            cJSON_AddItemToObject(json, topic[0], zmq_tcp_config);
            cJSON_AddStringToObject(zmq_tcp_config, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_config, "port", i_port);

            if (!dev_mode) {
                clients_env = getenv(CLIENTS_ENV);
                if (clients_env == NULL) {
                    LOG_ERROR("getenv failed for clients_env %s\n", CLIENTS_ENV);
                    goto err;
                }
                clients_env_len = strlen(clients_env);
                char clients[clients_env_len + 1];
                ret = strncpy_s(clients, clients_env_len + 1, clients_env, clients_env_len);
                if (ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d) : Failed to copy clients \" %s \" ", ret, clients_env);
                    goto err;
                }
                char* refptr = NULL;
                allowed_clients = (char **)calloc(clients_env_len + 1, sizeof(char*));
                if (allowed_clients == NULL) {
                    LOG_ERROR_0("Calloc failed for allowed_clients");
                    goto err;
                }
                for (i = 0, str = clients ; ; str = NULL, i++) {
                    data = strtok_r(str, ",", &refptr);
                    if (data == NULL) {
                       break;
                    }
                    data_len = strlen(data);
                    if (allowed_clients[i] == NULL) {
                        allowed_clients[i] = (char*) malloc(data_len + 1);
                    }
                    if (allowed_clients[i] == NULL) {
                        LOG_ERROR_0("Malloc failed for individual allowed_clients");
                        goto err;
                    }
                    ret = strncpy_s(allowed_clients[i], data_len + 1, data, data_len);
                    if (ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d) : Failed to copy allowed clients \" %s \" ", ret, data);
                        goto err;
                    }
                    trim(allowed_clients[i]);
                }

		        cJSON* all_clients = cJSON_CreateArray();
                for (j=0; j<i; j++) {
                    ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));
                    if (ret != 0) {
                        LOG_ERROR("String copy failed (errno: %d): Failed to copy publicKey", ret);
                        goto err;
                    }
                    ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, allowed_clients[j], strlen(allowed_clients[j]));
                    if (ret != 0) {
                        LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of client \" %s \" ", ret, allowed_clients[j]);
                        goto err;
                    }
                    char* client_pub_key = configmgr->get_config(publicKey);

                    if (strcmp(client_pub_key, "")) {
                        cJSON_AddItemToArray(all_clients, cJSON_CreateString(client_pub_key));
                    }
                }

                app_private_key[0] = '/';
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of publisher \" %s \" ", ret, app_name_env);
                    goto err;
                }
                ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, "/private_key", strlen("/private_key"));
                if (ret != 0) {
                    LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of publisher", ret);
                    goto err;
                }

                const char* server_secret_key = configmgr->get_config(app_private_key);

                cJSON_AddStringToObject(zmq_tcp_config, "server_secret_key", server_secret_key);
                cJSON_AddItemToObject(json, "allowed_clients", all_clients);
	        }
	    }
        else if (!strcmp(topic_type_arr, CLIENT)) {
            cJSON* zmq_tcp_config = cJSON_CreateObject();
            cJSON_AddItemToObject(json, topic[0], zmq_tcp_config);
            cJSON_AddStringToObject(zmq_tcp_config, "host", host);
            cJSON_AddNumberToObject(zmq_tcp_config, "port", i_port);

            if (!dev_mode) {
                char* end_point_list_env = NULL;
                char* end_point = NULL;
                char* ref_ptr = NULL;
                size_t end_point_list_env_len;
                end_point_list_env = getenv(REQUEST_EP);
                if (end_point_list_env == NULL) {
                    LOG_ERROR("getenv failed for end_point_list_env %s\n", REQUEST_EP);
                    goto err;
                }
                end_point_list_env_len = strlen(end_point_list_env);
                char end_point_list[end_point_list_env_len + 1];

                ret = strncpy_s(end_point_list, end_point_list_env_len + 1, end_point_list_env, end_point_list_env_len);
                if (ret != 0) {
                    LOG_ERROR("String copy failed (errno: %d): Failed to copy end_point_list_env \" %s \" ", ret, end_point_list_env);
                    goto err;
                }
                for (str = end_point_list; ; str = NULL) {
                    end_point = strtok_r(str , ",", &ref_ptr);
                    if (end_point == NULL) {
                        break;
                    }
                    trim(end_point);
                    if (!strcmp(topic[0], end_point)) {
                        ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));
                        if (ret != 0) {
                            LOG_ERROR("String copy failed (errno: %d): Failed to copy public keys", ret);
                            goto err;
                        }
                        ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                        if (ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate for getting key of client \" %s \" ", ret, app_name_env);
                            goto err;
                        }

                        const char* client_pub_key = configmgr->get_config(publicKey);
                        cJSON_AddStringToObject(zmq_tcp_config, "client_public_key", client_pub_key);
                        ret = strncpy_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, "/Publickeys/", strlen("/Publickeys/"));

                        if (ret != 0) {
                            LOG_ERROR("String copy failed (errno: %d): Failed to copy public keys", ret);
                            goto err;
                        }
                        ret = strncat_s(publicKey, PUBLIC_PRIVATE_KEY_SIZE, topic[0], topic_arr_len);
                        if (ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate topic \" %s \" to public key", ret, topic[0]);
                            goto err;
                        }

                        const char* server_pub_key = configmgr->get_config(publicKey);
                        cJSON_AddStringToObject(zmq_tcp_config, "server_public_key", server_pub_key);

                        app_private_key[0] = '/';
                        ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, app_name_env, strlen(app_name_env));
                        if (ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate app_name_env \" %s \" to app_private_key", ret, app_name_env );
                            goto err;
                        }
                        ret = strncat_s(app_private_key, PUBLIC_PRIVATE_KEY_SIZE, "/private_key", strlen("/private_key"));
                        if (ret != 0) {
                            LOG_ERROR("String concatenation failed (errno: %d): Failed to concatenate private_key to app_private_key", ret);
                            goto err;
                        }
                        const char* client_secret_key = configmgr->get_config(app_private_key);
                        cJSON_AddStringToObject(zmq_tcp_config, "client_secret_key", client_secret_key);
                    }
                }
            }
        } else {
            LOG_ERROR_0("topic is neither PUB nor SUB / neither Server nor Client");
            goto err;
        }
    } else if (!strcmp(mode, "zmq_ipc")) {
        char* socket_file = NULL;
        if (mode_address[2] == NULL) {
            LOG_DEBUG_0("Socket file explicitly not given by application ");
        } else {
            LOG_DEBUG("Socket file given by the application is = %s", mode_address[2]);
            socket_file = mode_address[2];
        }
        cJSON_AddStringToObject(json, "socket_dir", mode_address[1]);

        // If socket_file is given by the application
        if (socket_file != NULL) {
            if (!strcmp(topic_type_arr, PUB)) {
                LOG_DEBUG_0(" topic type is Pub");
                for (size_t i=0; i < num_of_topics; ++i) {
                    cJSON* socket_file_obj = cJSON_CreateObject();
                    cJSON_AddItemToObject(json, topic[i], socket_file_obj);
                    cJSON_AddStringToObject(socket_file_obj, SOCKET_FILE, socket_file);
                }
            } else if (!strcmp(topic_type_arr, SUB)) {
                LOG_DEBUG_0(" topic type is Sub");
                cJSON* socket_file_obj = cJSON_CreateObject();
                cJSON_AddItemToObject(json, pub_topic[1], socket_file_obj);
                cJSON_AddStringToObject(socket_file_obj, SOCKET_FILE, socket_file);
            }
        }

    } else {
        LOG_ERROR("mode: %s is not supported", mode);
        goto err;
    }

    char* config_value = cJSON_Print(json);
    LOG_DEBUG("Env Config is : %s \n", config_value);
    free(config_value);

    config = config_new(
            (void*) json, free_json, get_config_value);
    if (config == NULL) {
        LOG_ERROR_0("Failed to initialize configuration object");
        goto err;
    }

err:
    if (mode_address != NULL) {
        free_mem(mode_address);
    }
    if (host_port != NULL) {
        free_mem(host_port);
    }
    if (allowed_clients != NULL) {
        free_mem(allowed_clients);
    }
    if (pub_topic != NULL) {
        free_mem(pub_topic);
    }

    return config;
}

void trim(char* str_value) {
    int index;
    int i;

    // Trimming leading white spaces
    index = 0;
    while (str_value[index] == ' ' || str_value[index] == '\t' || str_value[index] == '\n') {
        index++;
    }

    i = 0;
    while (str_value[i + index] != '\0') {
        str_value[i] = str_value[i + index];
        i++;
    }
    str_value[i] = '\0'; // Terminate string with NULL

    // Trim trailing white spaces
    i = 0;
    index = -1;
    while (str_value[i] != '\0') {
        if (str_value[i] != ' ' && str_value[i] != '\t' && str_value[i] != '\n') {
            index = i;
        }
        i++;
    }
    // Mark the next character to last non white space character as NULL
    str_value[index + 1] = '\0';
}

void env_config_destroy(env_config_t* env_config) {
    if (env_config != NULL)
        free(env_config);
}


env_config_t* env_config_new() {
    env_config_t *env_config = (env_config_t *)malloc(sizeof(env_config_t));
    if (env_config == NULL) {
        return NULL;
    }
    env_config->get_topics_from_env = get_topics_from_env;
    env_config->get_messagebus_config = get_messagebus_config;
    env_config->get_topics_count = get_topics_count;
    env_config->trim = trim;
    env_config->free_mem = free_mem;
    return env_config;
}
