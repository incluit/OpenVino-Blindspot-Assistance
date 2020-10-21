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
 * @brief Config Manager example
 */


#include <eis/config_manager/config_manager.h>
#include "eis/config_manager/env_config.h"

#define PUB "pub"
#define SUB "sub"

void usage(const char* name) {
    printf("Usage: %s [Optional argument prod_mode=1 (By default dev_mode is enabled)]\n", name);
}

int main(int argc, char** argv){
    bool dev_mode = true;
    if(argc >= 3) {
        usage(argv[0]);
        return -1;
    } else if(argc == 2) {
        if(!strcmp(argv[1], "1")) {
            dev_mode = false;
        } else if(!strcmp(argv[1], "0")) {
            dev_mode = true;
        }
    }

    config_mgr_t* config_mgr_client;

    if(!dev_mode) {// if prod mode
        char pub_cert_file[100] = "../../examples/Sample_certs/VideoIngestion_client_certificate.pem";

        char pri_key_file[100] = "../../examples/Sample_certs/VideoIngestion_client_key.pem";

        char trust_file[] = "../../examples/Sample_certs/ca_certificate.pem";
        config_mgr_client = config_mgr_new("etcd", pub_cert_file, pri_key_file, trust_file);
    } else { // if dev mode
        config_mgr_client = config_mgr_new("etcd", "", "", "");
    }

    if(config_mgr_client == NULL) {
       printf("Config manager client creation failed\n");
       return 0;
    }

    setenv("PubTopics","camera1_stream,camera2_stream,camera3_stream ",1);
    setenv("SubTopics","Video/camera1_stream", 1);

    setenv("DEV_MODE", "true", 1);
    setenv("AppName", "publisher", 1);
    setenv("Clients", "publisher,VideoAnalytics", 1);
    //TODO: Edit this ENV_Config example to take the command line arguments
    // (argc,argv) to have a flexibility to demonstrate the TCP , IPC, multi-topics
    // and also the default behavior.
    //For now the below setenv commented line is maintained so that when user
    // wants to test using this example they can comment & uncomment  it based
    // on if multi-topic IPC support is needed or not and also if TCP is needed.

    //setenv("camera1_stream_cfg", "zmq_tcp,127.0.0.1:65013", 1);
    setenv("camera1_stream_cfg", "zmq_ipc, ./.socks, multi-topic.sock", 1);
    //setenv("camera1_stream_cfg", "zmq_ipc, ./.socks", 1);

    env_config_t* env_config_client = env_config_new();

    char** pub_topics = env_config_client->get_topics_from_env(PUB);

    size_t num_of_topics_pub = env_config_client->get_topics_count(pub_topics);
    printf("\n Number of publisher topics=%d\n",num_of_topics_pub);

    config_t* pub_config = env_config_client->get_messagebus_config(config_mgr_client, pub_topics, num_of_topics_pub, PUB);
    if(pub_config == NULL) {
        printf("Failed to get publisher message bus config\n");
    }
    else {
        printf("Getting Message bus publisher config is success !!\n");
    }

    // Same comment above applied here. This line to be commented or uncommented
    // based on if multi-topic support is needed or not.
    setenv("camera1_stream_cfg", "zmq_ipc, ./.socks, multi-topic.sock", 1);
    //setenv("camera1_stream_cfg", "zmq_ipc, ./.socks", 1);

    char** sub_topics = env_config_client->get_topics_from_env(SUB);
    size_t num_of_topics_sub = env_config_client->get_topics_count(sub_topics);
    printf("Number of subscriber topics=%d\n",num_of_topics_sub);
    config_t* sub_config = env_config_client->get_messagebus_config(config_mgr_client, sub_topics, num_of_topics_pub, SUB);
    if(sub_config == NULL) {
        printf("Failed to get subscriber message bus config\n");
    }
    else{
        printf("Getting Message bus subscriber config is success !!\n");
    }

    env_config_destroy(env_config_client);
    return 0;

}