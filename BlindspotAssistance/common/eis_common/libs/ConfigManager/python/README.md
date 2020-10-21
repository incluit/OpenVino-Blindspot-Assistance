# ConfigManager Py Wrappers

ConfigManager provides py wrapper APIs for distributed key-value store to read EIS related config details.

The py example program demonstrates 4 basic functionalities :
1. GetConfig - which returns value of the specified key from config manager.
2. PutConfig - To save a value to config manager if key is already exists, if not create/store a new set of key-value in config manager
3. RegisterKeyWatch - which keeps a watch on the specified key in config manager and triggers the callback whenever the value of the key is changed.
4. RegisterDirwatch - which keeps a watch on the prefix of a specified key in config manager and triggers the callback whenever the value of the key is changed.

**Note**: Currently supported storage types: **etcd**

## To run examples from [ConfigManager/python/examples folder]

### 1. Pre-requisite

* Install etcd service by following the link: [etcdservice](https://computingforgeeks.com/how-to-install-etcd-on-ubuntu-18-04-ubuntu-16-04/)

* Install python-etcd3 which is a python etcd client lib, from sources(as the master supports for watchprefix feature) by following the link: [python-etcd3](https://python-etcd3.readthedocs.io/en/latest/installation.html#from-sources)
### Dependencies

* Install EIS Python Config Manager lib and Python EnvConfig
    
    ```sh
    cd ConfigManager/python
    rm -rf build
    sudo python3.6 setup.py install
    ```

### Testing with security enabled

* To test basic functionalities:

```sh
$ cd ConfigManager/python/examples
$ make configmgrclient key=<provide a key> action=<provide the action to be performed on key, possible options are get, put, watchkey, watchdir> value=<value to set the key to(relevant only for the action 'put')>
```

### Testing with security disabled

* To test basic functionalities:

```sh
$ cd ConfigManager/python/examples
$ make configmgrclient_insecure key=<provide a key> action=<provide the action to be performed on key, possible options are get, put, watchkey, watchdir> value=<value to set the key to(relevant only for the action 'put')>
```
