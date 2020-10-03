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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "eis/config_manager/config_manager.h"

void callback(char* key, char * val){
   printf("callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

void test_callback(char* key, char * val){
   printf("test_callback callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

int main(int argc, char **argv) {
  char *key, *action, *val, *cert_file, *key_file, *ca_file;
  if(argc == 2 && !strcmp(argv[1], "-h")){
     printf("Usage: <program> <cert_file> <key_file> <ca_file> <action> <key> <value> \n\
               cert_file: config manager client certificate in pem format \n \
               key_file: config manager private key in pem format \n \
               ca_file: ca certificate in pem format \n \
               action: action to be performed on key, possible options are get, put, watchkey, watchdir \n \
               key: key to perform the action on \n \
               value: value to set the key to (relevant only for the action 'put')\n");
   }

   if (argc < 6) {
      printf("Check usuage of the program, defaulting to dev mode, action=get and key=/GlobalEnv/ \n");
      key = "/GlobalEnv/";
      action = "get";
      cert_file = "";
      key_file = "";
      ca_file = "";
   }else{
      cert_file = argv[1];
      key_file = argv[2];
      ca_file = argv[3];
      action = argv[4];
      if(!strcmp(action, "put")){
         if(argc < 7){
            printf("Check usuage of the program, provide the value for the action 'put'");
            exit(-1);
         }
      }
      key = argv[5]; 
      val = argv[6];
   }

   config_mgr_t* config_mgr_client = NULL;
  
   int flag = 0;
   config_mgr_client = config_mgr_new("etcd", cert_file, key_file, ca_file);

   if (config_mgr_client == NULL){
      printf("Config manager client creation failed\n");
      return 0;
   }

   if (!strcmp(action, "get")){
      flag = 1;
      char *value = config_mgr_client->get_config(key);
      printf("get_config is called, key:%s value is: %s \n", key, value);
      config_mgr_config_destroy(config_mgr_client);
      return 0;
   }
   if(!strcmp(action, "put")){
      flag = 1;
      int err_status = config_mgr_client->put_config(key, val);
      if (err_status == -1){
         printf("put_config is failed\n");
         return err_status;
      }
      char *value = config_mgr_client->get_config(key);
      printf("get_config is called on the key:%s, value is: %s \n", key, value);
      config_mgr_config_destroy(config_mgr_client);
      return 0;
   }
   if(!strcmp(action, "watchkey")){
      flag = 1;
      config_mgr_client->register_watch_key(key, callback);
   }
   if(!strcmp(action, "watchdir")){
      flag = 1;
      config_mgr_client->register_watch_dir(key, test_callback);
   }
   if(flag == 0){
      printf("Provided action is not supported");
   }
   sleep(35);
   config_mgr_config_destroy(config_mgr_client);
   return 0;
}

