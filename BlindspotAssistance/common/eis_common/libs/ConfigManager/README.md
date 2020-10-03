# ConfigManager
ConfigManager provides C/Python/Golang APIs for:
- distributed key-value store like `etcd` to read and watch on EIS related config details.
- To read EIS environment info needed to build EIS MessageBus config


# C ConfigManager

## Dependency Installation

The EISUtils depends on CMake version 3.11+. For Ubuntu 18.04 this is not
the default version installed via `apt-get`. To install the correct version
of CMake, execute the following commands:

```sh
# Remove old CMake version
$ sudo apt -y purge cmake
$ sudo apt -y autoremove

# Download CMake
$ wget https://cmake.org/files/v3.15/cmake-3.15.0-Linux-x86_64.sh

# Installation CMake
$ sudo mkdir /opt/cmake
$ sudo cmake-3.15.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license

# Make the command available to all users
$ sudo update-alternatives --install /usr/bin/cmake cmake /opt/cmake/bin/cmake 1 --force
```

To install the dependencies for the ConfigManager execute the following command:

```sh
$ sudo -E ./install.sh
```

Additionally, ConfigManager also has dependency on EISUtils library. Follow the documentation of EISUtils to install it.
* [IntelSafeString](../IntelSafeString/README.md)
* [EISUtils](../../util/c/README.md)

## Compilation
The EIS Config Manager utilizes CMake as the build tool for compiling the C/C++
library. The simplest sequence of commands for building the library are
shown below.

```sh
$ cd build
$ cmake ..
$ make
```

Set `WITH_PYTHON=ON` to compile Config Manager and EnvConfig in Python.

```
$ cmake -DWITH_PYTHON=ON ..
```

If you wish to compile the EIS Config Manager along with C examples
```sh
$ cmake -DWITH_EXAMPLES=ON ..
```


If you wish to compile the EIS Config Manager in debug mode, then you can set
the `CMAKE_BUILD_TYPE` to `Debug` when executing the `cmake` command (as shown
below).


```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```
## Installation

If you wish to install the EIS Config Manager on your system, execute the
following command after building the library:

```sh
$ sudo make install
```

By default, this command will install the EIS Config Manager C library into
`/usr/local/lib/`. On some platforms this is not included in the `LD_LIBRARY_PATH`
by default. As a result, you must add this directory to you `LD_LIBRARY_PATH`,
otherwise you will encounter issues using the Config Manager. This can
be accomplished with the following `export`:

```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
```
> **NOTE:** You can also specify a different library prefix to CMake through
> the `CMAKE_INSTALL_PREFIX` flag.

## Running Unit Tests

> **NOTE:**

* The unit tests will only be compiled if the `WITH_TESTS=ON` option is specified when running CMake.

* Provisioning EIS should be done to start etcd server in dev/prod mode and to generate application specific certificates(only in prod mode).

Run the following commands from the `build/tests` folder to cover the unit
tests. Make sure etcd daemon should be running, `etcdctl` is available in the folder `build/tests` and install `etcdctl` by following [https://github.com/etcd-io/etcd/releases](https://github.com/etcd-io/etcd/releases).Config manager(go etcd) APIs have been tested on commit `0c787e26bcd102c3bb14050ac893b07ba9ea029f`

* To run EnvConfig unit tests
```
$ ./env-config-tests
```

* TO run ConfigManager unit tests in Dev mode

```sh
$ export ETCDCTL_API=3
$ ./configmgr-tests <AppName>(AppName is optional in Devmode)
```

* Running Config Manager unit tests in Prod mode

```sh
To check usuage,
$ ./configmgr-tests -h

$ ./configmgr-tests <AppName> <app_cert_file> <app_key_file> <ca_file> <root_cert> <root_key>

Here,
AppName: service name
app_cert_file: config manager client certificate in pem format
app_key_file: config manager private key in pem format
ca_file: ca certificate in pem format
root_cert: config manager root certificate in pem format
root_key: config manager root key in pem format
```



## C APIs

### APIs for reading from distributed key-value store

1. Creating a new config manager client

    `config_mgr_t* config_mgr_client = config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert)`

    
    **API documentation:**

    `config_mgr_t* config_mgr_new(char *storage_type, char *cert_file, char *key_file, char *ca_cert);`
    ```
        config_mgr_new function to creates a new config manager client
        @param storage_type      - Type of key-value storage, Eg. etcd
        @param cert_file         - config manager client cert file
        @param key_file          - config manager client key file
        @param ca_cert           - config manager client ca cert
        @return config_mgt_t     - config_mgr_t object on success, NULL on failure
    ```

2. Accessing value of a key stored in distributed store like etcd

    `char* value = config_mgr_client->get_config("/VideoIngestion/config");`

    **API documentation:**

    `char* get_config(char *key)`
    ```
        get_config function gets the value of a key from config manager
        @param key      - key to be queried on from config manager
        @return char*   - values returned from config manager based on key,
                          "-1" as a value on any failure
    ```

3. To save a value to etcd if key already exists, if not create a new set of key-value in etcd

    `int value = config_mgr_client->put_config("/VideoIngestion/datastore", "data");`

    **API documentation:**

    `int put_config(char *key, char *value)`
    ```
        put_config function stores the value of a key to config manager
        @param key      - key in  config manager to set
        @param value    - value to set the key to
        @return int     - 0 on success, -1 on failure
    ```
    **NOTE:**
    1. As per default EIS configuration, put is allowed only on `/[Appname]/datastore`
    2. If running in prod mode, make sure the same application which is used to create config manager instance has to be used on put_config i.e the key to save in config manager would be `/[AppName]/datastore`
    Eg: If `Visualizer` has been used to create config_manager instance, put_config can only save value on `/Visualizer/datastore`

4. Registers user callback function to keep a watch on key based on it's prefix

    `config_mgr_client->register_watch_dir("/VideoIngestion/config", user_callback);`

    **API documentation:**

    `void register_watch_dir(char *key, (*register_watch_dir_cb)(char* key, char* value) user_callback)`
    
    `Here user_callback is callback function to be passed as an argument and example is given below`

    `void user_callback(char* key, char *updated_value_on_watch){}`

    ```
        register_watch_dir function registers to a callback and keeps a watch on the prefix of a specified key
        @param key                                                 - prefix of a key to keep a watch on
        @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event
                                                                     with updated value on the respective key
    ```
5. Registers user callback function to keep a watch on specified key

    `config_mgr_client->register_watch_key("/VideoIngestion/config", user_callback);`

    **API documentation:**

    `void register_watch_key(char *key, (*register_watch_key_cb)(char* key, char* value) user_callback)`

    `Here user_callback is callback function to be passed as an argument and example is given below`

    `void user_callback(char* key, char *updated_value_on_watch){}`

    ```
        register_watch_key function registers to a callback and keeps a watch on a specified key
        @param key                                                 - key to keep a watch on
        @param (*register_watch_key_cb)(char* key, char* value)    - user callback to be called on watch event
                                                                     with updated value on the respective key
    ```
6. Destroy config manager

    `config_mgr_config_destroy(config_mgr_client);`

    **API documentation:**
    `void config_mgr_config_destroy(config_mgr_t *config_mgr_config)`

    ```
        config_mgr_config_destroy function to delete the config_mgr_client instance
        @param config_mgt_t     -   config_mgr_client object
    ```

To refer C examples follow config_manager.c in [examples/](examples/)

**Steps to run example file:**

1. Navigate to ConfigManager/c
2. mkdir build
3. cd build
4. cmake -DWITH_EXAMPLES=ON ..
5. make
6. cd examples
7. `./config_manager`

**NOTE:** In order to run config_manager example in prod_mode, ensure provisioning is done. Also, run `./config_manager -h` from examples folder for its usage.

# Python APIs

1. **get_config_client(storage_type, config)**
        
   ```
   Returns config manager client based on storage_type
   
   :param storage_type: Type of key-value storage, Eg. etcd
   :param config: config of type Dict with certFile, keyFile and trustFile
   ```

To refer Python [examples](python/examples) on etcd client wrapper follow the README [python/README](python/README.md)

# Go APIs

1. **Init(storageType string, config map[string]string)**
        
    ```
    Initializes config manager and returns config manager client instance by calling 
    GetConfigClient API, With the client instance, respective storage_type apis can be called
    
    :param storage_type: Type of key-value storage, Eg. etcd
    :param config: config of type Dict with certFile, keyFile and trustFile
    :return config manager client if creation of client is successful, nil on failure
    ```

2. **GetConfigClient(storageType string, conf map[string]string)**
        
    ```
    Returns config manager client based on storage_type
    
    :param storage_type: Type of key-value storage, Eg. etcd
    :param config: config of type Dict with certFile, keyFile and trustFile
    ```
        
To refer GO examples on etcd client wrapper follow [go/README](go/README)

### APIs for reading environment info to build EIS MessageBus config structure

1. Creating a new env config client

    `env_config_t* env_config_client = env_config_new();`

    **API documentation:**

    `env_config_t* env_config_new();`
    ```
        env_config_new function to creates a new env config client
        @return env_config_t     - env_config object
    ```

2. Getting publish/subscribe topics from env

    `char** pub_topics = env_config_client->get_topics_from_env("pub");`
    `char** sub_topics = env_config_client->get_topics_from_env("sub");`

    **API documentation:**

    `char** get_topics_from_env(const char* topic_type);`
    ```
        get_topics_from_env function gives the topics with respect to publisher/subscriber.
        @param topic_type - "pub" for publisher and "sub" for subscriber.
        @return topics    - topics returned from env config based on topic type
    ```

3. Getting the number of topics in "SubTopics" or "PubTopics" environmental variable.

   `size_t num_of_topics_pub = env_config_client->get_topics_count(pub_topics);`
   `size_t num_of_topics_sub = env_config_client->get_topics_count(sub_topics);`

   **API documentation:**

   `size_t get_topics_count(char* topics[]);`
   ```
        get_topics_count function gives the number of topics in "SubTopics" or "PubTopics" environmental variable.

        @param topics[]       -  An array of C strings

        @return size_t      -    Number of topics
    ```

4. Getting EIS Message Bus config from ENV variables and config manager.

    `config_t* msgbus_config_pub = env_config_client->get_messagebus_config(config_mgr_client, pub_topics[], num_of_topics, "pub");`
    `config_t* msgbus_config_sub = env_config_client->get_messagebus_config(config_mgr_client, sub_topics[], num_of_topics, "sub");`

    **API documentation:**

    `config_t* get_messagebus_config(const config_mgr_t* configmgr, const char* topic[], size_t num_of_topics, const char* topic_type);`
    ```
        get_messagebus_config function gives the configuration that needs in connecting to EIS messagebus. In case of supporting multiple topics, the application will be responsible for providing the context for TCP & IPC connection. Example, in case of IPC the connection parameters which include socket directory & socket file are provided by the application.

        @param configmgr       - Config Manager object
        @param topic           - Array of Topics for which msg bus config needs                          to be constructed
                                 In case the topic is being published, it will be the stream name like `camera1_stream`
                                 and in case the topic is being subscribed, it will be of the format
                                 `[Publisher_AppName]/[stream_name]`.
                                 Eg: `VideoIngestion/camera1_stream`
        @param num_of_topics   - Number of topics
        @param topic_type      - TopicType for which msg bus config needs to be constructed
        @return config_t*      - JSON msg bus config of type config_t
    ```

**NOTE:** `get_messagebus_config()` api supports multiple ipc socket file configuration. 

With respect to multiple tcp configuration, even though api receives list of topics, it returns config of only first topic.
So, if an application needs env config for multiple topics, then this api should be called multiple times with the required topic.

For Example: In `config_t* msgbus_config_pub = env_config_client->get_messagebus_config(config_mgr_client, pub_topics[], num_of_topics, "pub");`
let's say pub_topics[] has **"camera1_stream,camera2_stream,camera3_stream"**. Calling this api with mentioned pub_topics, env_config is received by caller application for only **"camera1_stream"**.

If config is needed for other topics, that is **camera2_stream and camera 3_stream**, then caller application should make sure that **camera2_stream or camera3_stream** should be the first element in **pub_topics[]** array respectively based on whichever config is needed.

Similarly for **sub_topics[]** tcp config as well.


5. Memory de-allocation of char** varialbe.

    `char** topics = env_config_client->get_topics_from_env("sub");`
    `free_mem(topics);`

    **API documentation:**

    `void free_mem(char** arr);`
    ```
        free_mem function is to de-allocate the memory of char**.
        @param arr - char** variable that needs memory deallocation
    ```

6. Destroy Env Config

    `env_config_destroy(env_config_client);`

    **API documentation:**

    `void env_config_destroy(env_config_t* env_config_client);`
    ```
        env_config_destroy function to delete the env_config_client instance
        @param env_config_t     -   env_config_client object
    ```

To refer C examples follow env_config_example.c in [examples/](examples/)

**Steps to run example file:**

1. Navigate to ConfigManager/c
2. Run the below command from ConfigManager/c, if provisioning is done in prod mode.

**NOTE:** Ensure provisioning is done, in order to run env_config example in prod_mode.

```
sudo cp ../../../build/provision/Certificates/ca/* ../../../build/provision/Certificates/VideoIngestion/* ./examples/Sample_certs/.
  ```

3. mkdir build
4. cd build
5. cmake -DWITH_EXAMPLES=ON ..
6. make
7. cd examples
8. For dev mode either run `./env_config` or just `./env_config 0`.
9. For prod mode run `./env_config 1`
