/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package util

import (
	"io/ioutil"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"time"

	"github.com/golang/glog"
	"github.com/xeipuuv/gojsonschema"
)

// CheckPortAvailability - checks for port availability on hostname
func CheckPortAvailability(hostname, port string) bool {
	maxRetries := 1000
	retryCount := 0

	portUp := false
	glog.Infof("Waiting for Port: %s on hostname: %s ", port, hostname)
	for retryCount < maxRetries {
		conn, _ := net.DialTimeout("tcp", net.JoinHostPort(hostname, port), (5 * time.Second))
		if conn != nil {
			glog.Infof("Port: %s on hostname: %s is up.", port, hostname)
			conn.Close()
			portUp = true
			break
		}
		time.Sleep(100 * time.Millisecond)
		retryCount++
	}
	return portUp
}

// CheckPortOccupied - checks for port being occupied already
func CheckPortOccupied(hostname, port string) bool {

	portUp := false
	glog.Infof("Checking for Port: %s on hostname: %s ", port, hostname)

	conn, _ := net.DialTimeout("tcp", net.JoinHostPort(hostname, port), (5 * time.Second))
	if conn != nil {
		glog.Infof("Port: %s on hostname: %s is up.", port, hostname)
		conn.Close()
		portUp = true
	}

	return portUp
}

// WriteCertFile - A wrapper to write certifiates for different module
func WriteCertFile(fileList []string, Certs map[string]interface{}) error {
	for _, filePath := range fileList {
		fileName := filepath.Base(filePath)
		if data, ok := Certs[fileName].([]byte); ok {
			err := ioutil.WriteFile(filePath, data, 0777)
			if err != nil {
				glog.Errorf("Not able to write to secret file: %v, error: %v", filePath, err)
				return err
			}
		}
	}
	return nil
}

//DeleteCertFile - wrapper to remove certificates
func DeleteCertFile(fileList []string) error {
	for _, filePath := range fileList {
		err := os.Remove(filePath)
		if err != nil {
			glog.Infof("Failed to revmove influxdb certs: %v", err.Error())
			return err
		}
	}
	return nil
}

func GetCryptoMap(appName string) map[string]string {

	conf := map[string]string{
		"certFile":  "",
		"keyFile":   "",
		"trustFile": "",
	}

	devMode, _ := strconv.ParseBool(os.Getenv("DEV_MODE"))

	if devMode != true {
		conf["certFile"] = "/run/secrets/etcd_" + appName + "_cert"
		conf["keyFile"] = "/run/secrets/etcd_" + appName + "_key"
		conf["trustFile"] = "/run/secrets/ca_etcd"

		configmgr_cert := os.Getenv("CONFIGMGR_CERT")
		configmgr_key := os.Getenv("CONFIGMGR_KEY")
		configmgr_cacert := os.Getenv("CONFIGMGR_CACERT")
		if ( configmgr_cert != "") && (configmgr_key != "") && (configmgr_cacert != "") {
			conf["certFile"] = configmgr_cert
			conf["keyFile"] = configmgr_key
			conf["trustFile"] = configmgr_cacert
		}
	}

	return conf
}

// ValidateJSON - JSON validator function
func ValidateJSON(schema string, config string) bool {
	schemaJSON := gojsonschema.NewStringLoader(schema)
	validJSON := gojsonschema.NewStringLoader(config)

	result, err := gojsonschema.Validate(schemaJSON, validJSON)
	if err != nil {
		glog.Error("Error during JSON validation %v", err)
		return false
	}

	if !result.Valid() {
		for _, desc := range result.Errors() {
			glog.Errorf("- %s\n", desc)
		}
		glog.Error("JSON schema validation failed !")
		return false
	}

	glog.Info("JSON schema validation passed !")
	return true

}