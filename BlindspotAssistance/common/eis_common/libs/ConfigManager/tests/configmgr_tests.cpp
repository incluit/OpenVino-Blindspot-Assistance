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
 * @brief Config Manager GTests unit tests
 * @author Varalakshmi KA (varalakshmi.ka@intel.com)
 */

#include <gtest/gtest.h>
#include <string.h>
#include <unistd.h>
#include "eis/config_manager/config_manager.h"

#define COMMAND_LENGTH 40
#define PUT_VAL_CMD 30
#define TEST_PUT_VAL_CMD 32

static char* cert_file = NULL;
static char* key_file = NULL;
static char* ca_file = NULL;
static const char* app_name = "";
static char* root_cert = NULL;
static char* root_key = NULL;

static int watch_key_cb = 0;
static int watch_dir_cb = 0;
static int dev_mode = 0;
static char* command = "";

void watch_key_callback(char* key, char* value){
    std::cout << "watch key callback....." << std::endl;
    watch_key_cb++;
}

void watch_dir_callback(char* key, char* value){
    std::cout << "watch dir callback....." << std::endl;
    watch_dir_cb++;
}


void init(){
    if(ca_file == NULL && cert_file == NULL && key_file == NULL){
        dev_mode = 1;
        return;
    }

    
    command = (char*)malloc((strlen(ca_file) + strlen(root_cert) + strlen(root_key) + COMMAND_LENGTH) * sizeof(char));
    sprintf(command, " --cacert=%s --cert=%s --key=%s", ca_file, root_cert, root_key);
}

config_mgr_t* get_config_mgr_client(char *storage_type){
    config_mgr_t *config_mgr_client = config_mgr_new(storage_type, cert_file, key_file, ca_file);
    return config_mgr_client;
}

// Return the key in /[AppName]/key format
char* get_app_key(char* key){
    char* app_key = (char*)malloc((strlen(app_name) + strlen(key) + 3) * sizeof(char));
    strcpy(app_key, "/");
    if(strcmp(app_name,"")){
        // output of the below sprintf() will be: '/<app_name/' 
        sprintf(app_key, "%s%s/", app_key, app_name);
    }
    strcat(app_key, key);
    return app_key;
}

TEST(configmgr_test, configmgr_init) {
    std::cout << "Test case: create configmgr instance.." << std::endl;
    config_mgr_t* config_mgr_client = get_config_mgr_client((char*)"etcd");
    ASSERT_NE(nullptr, config_mgr_client);
    config_mgr_config_destroy(config_mgr_client);
}

TEST(configmgr_test, configmgr_get_config) {
    std::cout << "Test case: get_config()..." << std::endl;
    config_mgr_t* config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }

    char* put_cmd = (char*)malloc((strlen(command) + COMMAND_LENGTH) * sizeof(char));
    char* key = get_app_key((char*)"test");
    sprintf(put_cmd, "./etcdctl put %s test12356", key);

    if(dev_mode == 0){    
        strcat(put_cmd, command);
    }
    system(put_cmd);
    std::cout << "get_config() API on key:" << key << std::endl;

    char* value = config_mgr_client->get_config(key);
    free(put_cmd);
    free(key);
    config_mgr_config_destroy(config_mgr_client);
    ASSERT_STREQ("test12356", value);
}

TEST(configmgr_test, configmgr_put_config) {
    std::cout << "Test case: put_config()..." << std::endl;
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    
    char* key = get_app_key((char*)"datastore");
    std::cout << "put_config() API on key:" << key << std::endl;

    char* val = "UnitTesting put_config api";
    int err_status = config_mgr_client->put_config(key, val);

    if (err_status == -1){
        FAIL() << "put_config() API is failed";
    }

    char *value = config_mgr_client->get_config(key);

    free(key);
    config_mgr_config_destroy(config_mgr_client);
    ASSERT_STREQ(val, value);
}

TEST(configmgr_test, configmgr_register_watch_key) {
    std::cout << "Test case: register_watch_key()..." << std::endl;
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }

    char* key = get_app_key((char*)"watch_key_test");
    char* put_value_cmd = (char*)malloc((strlen(key) + strlen(command) + PUT_VAL_CMD + 20) * sizeof(char));
    char* test_put_cmd = (char*)malloc((strlen(key) + strlen(command) + TEST_PUT_VAL_CMD + 20)* sizeof(char));
    sprintf(put_value_cmd, "./etcdctl put %s test123", key);
    sprintf(test_put_cmd, "./etcdctl put %s test123456", key);
    
    if(dev_mode == 0){
        strcat(put_value_cmd, command);
        strcat(test_put_cmd, command);
    }

    system(put_value_cmd);
    config_mgr_client->register_watch_key(key, watch_key_callback);
    std::cout << "register_watch_key() API on key:" << key << std::endl;
    
    sleep(2);

    system(test_put_cmd);

    config_mgr_config_destroy(config_mgr_client);
    free(key);
    free(put_value_cmd);
    free(test_put_cmd);

    ASSERT_EQ(1, watch_key_cb);
}

TEST(configmgr_test, configmgr_register_watch_dir) {
    std::cout << "Test case: register_dir_key()..." << std::endl;
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }

    char* key = get_app_key((char*)"watch_dir_test");
    char* put_value_cmd = (char*)malloc((strlen(key) + strlen(command) + PUT_VAL_CMD + 20) * sizeof(char));
    char* test_put_cmd = (char*)malloc((strlen(key) + strlen(command) + TEST_PUT_VAL_CMD + 20)* sizeof(char));
    sprintf(put_value_cmd, "./etcdctl put %s test123", key);
    sprintf(test_put_cmd, "./etcdctl put %s test123456", key);
    
    if(dev_mode == 0){
        strcat(put_value_cmd, command);
        strcat(test_put_cmd, command);
    }

    system(put_value_cmd);
    char* watch_dir = get_app_key((char*)"watch_dir");

    config_mgr_client->register_watch_dir(watch_dir, watch_dir_callback);
    std::cout << "register_watch_dir() API on prefix:" << watch_dir << std::endl;
    
    sleep(2);

    system(test_put_cmd);

    config_mgr_config_destroy(config_mgr_client);
    free(key);
    free(watch_dir);
    free(put_value_cmd);
    free(test_put_cmd);

    ASSERT_EQ(1, watch_dir_cb);
}

TEST(configmgr_test, configmgr_init_fail) {
    std::cout << "Test case: fail to create configmanager instance..." << std::endl;
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"test");
    ASSERT_EQ(nullptr, config_mgr_client);
    config_mgr_config_destroy(config_mgr_client);
}


TEST(configmgr_test, configmgr_get_config_fail) {
    std::cout << "Test case: fail to get_config()..." << std::endl;
    config_mgr_t* config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }

    char* put_cmd = (char*)malloc((strlen(command) + COMMAND_LENGTH) * sizeof(char));
    char* key = "/TestApp/test";
    sprintf(put_cmd, "./etcdctl put %s test12", key);

    if(dev_mode == 0){    
        strcat(put_cmd, command);
    }
    system(put_cmd);
    std::cout << "get_config() API to Fail on key:" << key << std::endl;

    char* value = config_mgr_client->get_config(key);
    free(put_cmd);
    config_mgr_config_destroy(config_mgr_client);
    if(dev_mode == 0){
        ASSERT_STREQ(nullptr, value);
    }
}

TEST(configmgr_test, configmgr_put_config_fail) {
    std::cout << "Test case: fail to put_config()..." << std::endl;
    config_mgr_t *config_mgr_client = get_config_mgr_client((char*)"etcd");
    if (config_mgr_client == NULL){
        FAIL() << "Failed to create config manager client";
    }
    
    char* key = get_app_key((char*)"datasto");
    std::cout << "put_config() API to Fail on key:" << key << std::endl;

    int err_status = config_mgr_client->put_config(key, (char*)"UnitTesting put_config api fail");
    free(key);

    config_mgr_config_destroy(config_mgr_client);
    if(dev_mode == 0){
        ASSERT_EQ(-1, err_status);
    }else{
        ASSERT_EQ(0, err_status);
    }
}

int main(int argc, char **argv) {
    if(argc == 7){
        app_name = argv[1];
        cert_file = argv[2];
        key_file = argv[3];
        ca_file = argv[4];
        root_cert = argv[5];
        root_key = argv[6];
        printf("Unit tests are running in Prod mode....\n");
    }else if(argc == 2){
        if(!strcmp(argv[1], "-h")){
            printf("Usage In Dev mode: <program> <app_name>(Optional)\n");
            printf("Usage In Prod mode: <program> <app_name> <app_cert_file> <app_key_file> <ca_file> <root_cert> <root_key>\napp_name: service name \napp_cert_file(In Prod mode): config manager client certificate in pem format \napp_key_file(In Prod mode): config manager private key in pem format \nca_file(In Prod mode): ca certificate in pem format \nroot_cert(In prod  mode):config manager root certificate in pem format\nroot_key(In Prod mode): config manager root key in pem format\n");
            return 0;
        }
        app_name = argv[1];
        printf("Unit tests are running in Dev mode....\n");
    }else if(argc == 1){
        printf("Unit tests are running in Dev mode....\n");
    }else{
        printf("Wrong number of args are passed, Check the usuage: <program> -h\n");
    }

    init();
    ::testing::InitGoogleTest(&argc, argv);
    int test_run_status = RUN_ALL_TESTS();
    if(strcmp(command, "")){
        free(command);
    }
    return test_run_status;
}
