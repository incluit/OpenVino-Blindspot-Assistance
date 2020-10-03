/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package eismsgbus

import (
	"bytes"
	"encoding/json"
	"errors"
	"io/ioutil"
	"os"
)

// Helper function for parsing a JSON configuration for the message bus from the given byte blob.
//
// Note: This method automatically takes care of using a JSON decode having set the UseNumber() flag.
func ParseJsonConfig(jsonBytes []byte) (map[string]interface{}, error) {
	var out map[string]interface{}

	// Initialize decoder
	decoder := json.NewDecoder(bytes.NewBuffer(jsonBytes))
	decoder.UseNumber()

	// Decode the JSON blob
	err := decoder.Decode(&out)
	if err != nil {
		return nil, err
	}

	return out, nil
}

// Helper function for reading a JSON configuration for the message bus from a JSON
// configuration file.
//
// Note: This method automatically takes care of using a JSON decode having set the UseNumber() flag.
func ReadJsonConfig(fileName string) (map[string]interface{}, error) {
	// Open JSON file
	if fileName == "" {
		return nil, errors.New("fileName cannot be empty")
	}
	jsonFile, err := os.Open(fileName)
	if err != nil {
		return nil, err
	}
	defer jsonFile.Close()
	// Read all bytes from the file
	jsonBytes, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		return nil, err
	}
	return ParseJsonConfig(jsonBytes)
}
