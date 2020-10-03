/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package influxdbutil

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"

	"github.com/golang/glog"
	"github.com/influxdata/influxdb/client/v2"
)

// CreateHTTPClient returns a http client connected to Influx DB
func CreateHTTPClient(host string, port string, userName string, passwd string, devMode bool) (client.Client, error) {
	var buff bytes.Buffer

	var err error
	var httpClient client.Client

	if !devMode {
		fmt.Fprintf(&buff, "https://%s:%s", host, port)
		const (
			RootCA = "/tmp/influxdb/ssl/ca_certificate.pem"
		)
		certPool := x509.NewCertPool()
		ca, err := ioutil.ReadFile(RootCA)

		if err != nil {
			glog.Errorf("Failed to Read CA Certificate : %s", err)
			return nil, err
		}
		if ok := certPool.AppendCertsFromPEM(ca); !ok {
			glog.Errorf("Failed to Append Certificate")
			return nil, nil
		}
		httpClient, err = client.NewHTTPClient(client.HTTPConfig{
			Addr:               buff.String(),
			Username:           userName,
			Password:           passwd,
			InsecureSkipVerify: false,
			TLSConfig: &tls.Config{
				RootCAs: certPool}})
	} else {
		fmt.Fprintf(&buff, "http://%s:%s", host, port)
		httpClient, err = client.NewHTTPClient(client.HTTPConfig{
			Addr:     buff.String(),
			Username: userName,
			Password: passwd})
	}

	return httpClient, err
}

// DropAllSubscriptions drops all the subscriptions in InfluxDB
func DropAllSubscriptions(cli client.Client, dbName string) (*client.Response, error) {
	var buff bytes.Buffer

	fmt.Fprintf(&buff, "show subscriptions")
	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	if err != nil {
		glog.Errorf("Failed to query subscriptions : %s", err)
		return response, err
	}

	res := response.Results
	if res == nil || res[0].Series == nil {
		return response, err
	}

	for _, row := range res[0].Series[0].Values {
		val := row[1].(string)
		buff.Reset()
		fmt.Fprintf(&buff, "drop subscription \"%s\" ON \"%s\".\"autogen\"", val, dbName)
		q := client.NewQuery(buff.String(), "", "")
		response, err := cli.Query(q)
		if err != nil {
			glog.Errorf("Failed to delete subscription : %s", val)
			return response, err
		}
	}
	return response, err
}

// CreateSubscription creates the subscription in InfluxDB
func CreateSubscription(cli client.Client, subName string, dbName string, httpHost string, httpPort string, devMode bool) (*client.Response, error) {
	var buff bytes.Buffer

	if devMode {
		fmt.Fprintf(&buff, "create subscription %s ON \"%s\".\"autogen\" DESTINATIONS ALL 'http://%s:%s'", subName, dbName, httpHost, httpPort)
	} else {
		fmt.Fprintf(&buff, "create subscription %s ON \"%s\".\"autogen\" DESTINATIONS ALL 'https://%s:%s'", subName, dbName, httpHost, httpPort)
	}

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// CreateUser creates a new user in InfluxDB
func CreateUser(cli client.Client, UserName string, Password string) (*client.Response, error) {
	var buff bytes.Buffer
	fmt.Fprintf(&buff, "CREATE USER %s WITH PASSWORD '%s'", UserName, Password)
	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// CreateAdminUser creates a new user in InfluxDB
func CreateAdminUser(cli client.Client, UserName string, Password string, dbName string) (*client.Response, error) {
	var buff bytes.Buffer
	fmt.Fprintf(&buff, "CREATE USER %s WITH PASSWORD '%s' WITH ALL PRIVILEGES", UserName, Password)
	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// GrantAllPermissions grants all permissions on a to a user in InfluxDB
func GrantAllPermissions(cli client.Client, dbName string, UserName string) (*client.Response, error) {
	var buff bytes.Buffer
	fmt.Fprintf(&buff, "GRANT ALL ON %s TO %s", dbName, UserName)
	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}

// CreateDatabase creates the InfluxDB database
func CreateDatabase(cli client.Client, dbName string, retentionPolicy string) (*client.Response, error) {
	var buff bytes.Buffer

	if retentionPolicy != "" {
		fmt.Fprintf(&buff, "create database %s with duration %s", dbName, retentionPolicy)
	} else {
		fmt.Fprintf(&buff, "create database %s", dbName)
	}

	q := client.NewQuery(buff.String(), "", "")
	response, err := cli.Query(q)
	return response, err
}
