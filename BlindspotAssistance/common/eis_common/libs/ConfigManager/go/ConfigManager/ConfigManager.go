/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Package configmanager to chose datastore client object
package configmanager

import (
	"strings"
	"flag"

	"github.com/golang/glog"
)

type config struct {
	certFile  string
	keyFile   string
	trustFile string
}

// ConfigMgr interface
type ConfigMgr interface {
	GetConfig(key string) (string, error)

	PutConfig(key string, value string) error

	RegisterDirWatch(key string, onChangeCallback OnChangeCallback)

	RegisterKeyWatch(key string, onChangeCallback OnChangeCallback)
}

// OnChangeCallback callback
type OnChangeCallback func(key string, newValue string)

//Init function to initialize config manager
func Init(storageType string, config map[string]string) ConfigMgr {
	flag.Parse()
	flag.Lookup("logtostderr").Value.Set("true")
	defer glog.Flush()

	glog.Infof("initializing configuration manager...")
	return GetConfigClient(storageType, config)
}

//GetConfigClient function returns respective config client instance
func GetConfigClient(storageType string, conf map[string]string) ConfigMgr {
	var config config

	if strings.ToLower(storageType) == "etcd" {
		config.certFile = conf["certFile"]
		config.keyFile = conf["keyFile"]
		config.trustFile = conf["trustFile"]
		etcdclient, err := NewEtcdClient(config)
		if err != nil {
			glog.Errorf("Etcd client initialization failed!! Error: %v", err)
			return nil
		}
		return etcdclient
	}
	return nil
}
