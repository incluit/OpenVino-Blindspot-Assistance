/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

/*
typedef void (*callback_fcn)(char* key, char* value);
static inline void cgoCallBack(callback_fcn cb, char *key, char *value){
	cb(key, value);

}
*/
import "C"

import (
	"os"
	"strings"
	"sync"

	"github.com/golang/glog"

	configmgr "ConfigManager"
)

var mu sync.Mutex

var confMgr configmgr.ConfigMgr

// register callbacks by mapping usercallbacks to a specific config manager key
var registerCallbacks = make(map[string][]C.callback_fcn)

func registerCallback(key string, userCallback C.callback_fcn) string {
	flag := 0

	// Check if ETCD_PREFIX is set, if so register callback for the prepended prefixed key
	etcdPrefix := os.Getenv("ETCD_PREFIX")
	if etcdPrefix != "" {
		key = etcdPrefix + key
	}

	glog.Infof("Register user callback on key %v", key)
	for k, v := range registerCallbacks {
		// config manager key already exists hence add user callbacks to the list
		if k == key {
			flag = 1
			mu.Lock()
			v = append(v, userCallback)
			registerCallbacks[key] = v
			mu.Unlock()
			break
		}
	}
	if flag == 0 {
		// config manager key doen't exist hence create a new config manager key and user callback pair
		mu.Lock()
		registerCallbacks[key] = []C.callback_fcn{userCallback}
		mu.Unlock()
	}
	return key
}

func watchKeyCallback(key string, value string) {
	ckey := C.CString(key)
	cvalue := C.CString(value)
	callbacks := registerCallbacks[key]
	for _, userCb := range callbacks {
		mu.Lock()
		C.cgoCallBack(userCb, ckey, cvalue)
		mu.Unlock()
	}
}

func watchDirCallback(key string, value string) {
	ckey := C.CString(key)
	cvalue := C.CString(value)
	for k := range registerCallbacks {
		if (strings.HasPrefix(key, k)) && (key != k) {
			callbacks := registerCallbacks[k]
			for _, userCb := range callbacks {
				mu.Lock()
				C.cgoCallBack(userCb, ckey, cvalue)
				mu.Unlock()
			}
		}
	}

}

//export initialize
func initialize(CStorageType *C.char, CCertFile *C.char, CKeyFile *C.char, CTrustFile *C.char) int {
	storageType := C.GoString(CStorageType)
	config := map[string]string{
		"certFile":  C.GoString(CCertFile),
		"keyFile":   C.GoString(CKeyFile),
		"trustFile": C.GoString(CTrustFile),
	}
	confMgr = configmgr.Init(storageType, config)
	if confMgr == nil {
		glog.Errorf("Config manager initializtion failed")
		return -1
	}
	return 0
}

//export getConfig
func getConfig(keyy *C.char) *C.char {
	key := C.GoString(keyy)
	value, err := confMgr.GetConfig(key)
	if err != nil {
		glog.Errorf("getConfig failed for the key %s with Error: %v", key, err)
		return nil
	}
	glog.V(1).Infof("GetConfig is called and the value of the key %s is: %s", key, value)
	return C.CString(value)
}

//export putConfig
func putConfig(ckey *C.char, cvalue *C.char) int {
	key := C.GoString(ckey)
	value := C.GoString(cvalue)
	errStatus := confMgr.PutConfig(key, value)
	if errStatus != nil {
		glog.Errorf("Config manager PutConfig failed")
		return -1
	}
	return 0
}

//export registerWatchKey
func registerWatchKey(CKey *C.char, userCallback C.callback_fcn) {
	key := C.GoString(CKey)
	for k := range registerCallbacks {

		if k == key {
			glog.V(1).Infof("The key %s is registered for watch key event", key)
			registerCallback(key, userCallback)
			return
		}
	}
	registeredKey := registerCallback(key, userCallback)
	glog.Infof("Register the key: %s for watch key", registeredKey)
	confMgr.RegisterKeyWatch(key, watchKeyCallback)
}

//export registerWatchDir
func registerWatchDir(Ckey *C.char, userCallback C.callback_fcn) {
	key := C.GoString(Ckey)
	for k := range registerCallbacks {
		if k == key {
			glog.Infof("The key %s is already registered for watch key event", key)
			registerCallback(key, userCallback)
			return
		}
	}
	registeredKey := registerCallback(key, userCallback)
	glog.Infof("Register the key: %s for watch prefix", registeredKey)
	confMgr.RegisterDirWatch(key, watchDirCallback)
}

func main() {

}
