# ConfigManager Go Wrappers

ConfigManager provides go wrapper APIs for distributed key-value store to read EIS related config details.

The go example program demonstrates 4 basic functionalities :
* GetConfig - which returns value of the specified key from config manager
* PutConfig - To save a value to config manager if key already exists, if not create/store a new set of key-value to config manager
* RegisterKeyWatch - which keeps a watch on the specified key and triggers the callback whenever the value of the key is changed.
* RegisterDirwatch - which keeps a watch on the prefix of a specified key and triggers the callback whenever the value of the key is changed.

**Note**: Currently supported storage types: **etcd**

## How to install EIS Go EnvConfig

* There are 2 ways to install EIS Go EnvConfig
1. Execute `cmake -DWITH_GO_ENV_CONFIG=ON ..` and `make` commands from `build` directory of `common/libs/ConfigManager`, follow README [C/README](../../C/README.md) for more information.
2. Without installing `EIS Go EnvConfig` through cmake 

```sh
$ cp -r go/EnvConfig/ $GOPATH/src
# OR
$ ln -s go/EnvConfig/ $GOPATH/src
```

> **NOTE:** The command above assumes that you are currently in the
> ConfigManger source root `common/libs/ConfigManager` directory.
  

### 1. Pre-requisite

* Install etcd service by following the link: [etcd service](https://computingforgeeks.com/how-to-install-etcd-on-ubuntu-18-04-ubuntu-16-04/)

* Install go-etcd3, go-etcd3 is a go etcd client lib, install this by running go get go.etcd.io/etcd/clientv3 from $GOPATH.

  **Note**: Currently EIS is tested on commit ID: 0c787e26bcd102c3bb14050ac893b07ba9ea029f

* Install EIS Go ConfigManager: There are 2 ways to install Go ConfigManager
1. Execute `cmake` and `make` commands from `build` directory of `C ConfigManager`, follow README [C/README](../../C/README) for more information.
2. Without installing `EIS C ConfigManager`

```sh
$ cp -r go/ConfigManager/ $GOPATH/src
# OR
$ ln -s go/ConfigManager/ $GOPATH/src
```

> **NOTE:** The command above assumes that you are currently in the
> ConfigManger source root directory.

## How to run examples from [examples](examples) folder

### 1. Testing with security enabled

* To test basic functionalities:

```sh
make configmgrclient key=<provide a key> action=<provide the action to be performed on key, possible options are get, put, watchkey, watchdir> value=<value to set the key to(relevant only for the action 'put')>
```

### 1. Testing with security disabled

* To test basic functionalities:

```sh
make configmgrclient_insecure key=<provide a key> action=<provide the action to be performed on key, possible options are get, put, watchkey, watchdir> value=<value to set the key to(applies only for the action 'put')>
```
