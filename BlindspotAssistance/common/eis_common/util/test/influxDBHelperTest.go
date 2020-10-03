/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

import (
	"flag"

	"github.com/golang/glog"

	util "IEdgeInsights/common/util/influxdb"
)

const (
	//InfluxDB config
	host     = "localhost"
	port     = "8086"
	userName = "root"
	password = "root"
	dbName   = "testDB"
	subName  = "testSubName"

	//UDP server config
	strmMgrTCPServHost = "localhost"
	strmMgrTCPServPort = "61971"
)

func main() {
	flag.Parse()
	flag.Set("logtostderr", "true")

	defer glog.Flush()

	glog.Infoln("Creating InfluxDB HTTP client...")

	client, err := util.CreateHTTPClient(host, port, userName, password)

	if err != nil {
		glog.Errorf("Error creating InfluxDB client: %v", err)
	}

	glog.Infof("Creating database: %s\n", dbName)
	response, err := util.CreateDatabase(client, dbName)

	if err == nil && response.Error() == nil {
		glog.Infof("Response: %v", response.Results)
	} else {
		glog.Errorf("Error: %v while creating database: %s", err, dbName)
	}

	glog.Infof("Creating subscription: %s on db: %s", subName, dbName)
	response, err = util.CreateSubscription(client, subName, dbName,
		strmMgrTCPServHost, strmMgrTCPServPort)

	if err == nil && response.Error() == nil {
		glog.Infof("Response: %v", response.Results)
	} else {
		glog.Errorf("CreateSubscription error: %v", err)
		glog.Errorf("CreateSubscription response error: %v", response.Error())
	}

}
