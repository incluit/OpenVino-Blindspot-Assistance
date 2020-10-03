/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	"flag"
	"fmt"
	"log"
	"time"

	configmgr "ConfigManager"
)

func callback(key string, value string) {
	fmt.Println("\ncallback is called")
	fmt.Printf("watch function is watching the key: %s \nnew value is: %s\n", key, value)
}

func main() {

	certFile := flag.String("certFile", "", "provide client certificate file")
	privateFile := flag.String("privateFile", "", "provide client private key file")
	trustFile := flag.String("trustFile", "", "provide ca cert file")
	key := flag.String("key", "", "provide etcd key")
	action := flag.String("action", "", "provide the action to be performed on etcd key Eg: get|put|watchkey|watchdir|put")
	val := flag.String("val", "", "value to be set on the provided key(applies only for the action 'put')")

	flag.Parse()

	config := map[string]string{
		"certFile":  *certFile,
		"keyFile":   *privateFile,
		"trustFile": *trustFile,
	}
	configMgrClient := configmgr.Init("etcd", config)
	if configMgrClient == nil {
		log.Fatal("Creation of config manager client is failed")
	}

	if *action == "get" {
		value, err := configMgrClient.GetConfig(*key)
		fmt.Printf("value is %s", value)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Printf("GetConfig is called and the value is: %s", value)
		return
	} else if *action == "watchkey" {
		fmt.Printf("Watching on the key %s", *key)
		configMgrClient.RegisterKeyWatch(*key, callback)
	} else if *action == "watchdir" {
		fmt.Printf("Watching on the dir %s", *key)
		configMgrClient.RegisterDirWatch(*key, callback)
	} else if *action == "put" {
		fmt.Printf("Setting the key:%s with the value:%s", *key, *val)
		configMgrClient.PutConfig(*key, *val)
		return
	} else {
		fmt.Println("Provided action is not supported")
		return
	}

	for {
		time.Sleep(1)
	}

}
