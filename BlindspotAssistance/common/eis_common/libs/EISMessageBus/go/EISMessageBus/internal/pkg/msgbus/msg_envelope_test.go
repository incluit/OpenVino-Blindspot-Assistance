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

package msgbus

import (
	// types "EISMessageBus/pkg/types"
	"reflect"
	"testing"
)

func TestMapToMsgEnvelope(t *testing.T) {
	m := map[string]interface{}{
		"str":   "hello",
		"int":   2.0,
		"float": 55.5,
		"bool":  true,
		"obj": map[string]interface{}{
			"nest": map[string]interface{}{
				"test": "hello",
			},
			"hello": "world",
		},
		"arr":   []interface{}{"test", 123.0},
		"empty": nil,
	}

	msg, err := GoToMsgEnvelope(m)
	if err != nil {
		t.Errorf("%v", err)
		return
	}
	defer MsgEnvelopeDestroy(msg)

	res, err := MsgEnvelopeToGo(msg)
	if err != nil {
		t.Errorf("Failed to convert from MsgEnvelope to Go object")
		return
	}

	eq := reflect.DeepEqual(res.Data, m)
	if !eq {
		t.Errorf("Deserialized message to Go object failed:\n\tres.: %v\n\torig: %v", res.Data, m)
		return
	}
}

func TestBytesToMsgEnvelope(t *testing.T) {
	bytes := []byte("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09")

	msg, err := GoToMsgEnvelope(bytes)
	if err != nil {
		t.Errorf("%v", err)
		return
	}
	defer MsgEnvelopeDestroy(msg)

	res, err := MsgEnvelopeToGo(msg)
	if err != nil {
		t.Errorf("Failed to convert from MsgEnvelope to Go object")
		return
	}

	eq := reflect.DeepEqual(res.Blob, bytes)
	if !eq {
		t.Errorf("Deserialized message to Go object failed:\n\tres.: %v\n\torig: %v", res.Blob, bytes)
		return
	}
}

func TestMapBytesToMsgEnvelope(t *testing.T) {
	m := map[string]interface{}{"str": "hello", "int": 2.0, "float": 55.5, "bool": true}
	bytes := []byte("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09")
	slice := make([]interface{}, 2)
	slice[0] = m
	slice[1] = bytes

	msg, err := GoToMsgEnvelope(slice)
	if err != nil {
		t.Errorf("%v", err)
		return
	}
	defer MsgEnvelopeDestroy(msg)

	res, err := MsgEnvelopeToGo(msg)
	if err != nil {
		t.Errorf("Failed to convert from MsgEnvelope to Go object")
		return
	}

	eq := reflect.DeepEqual(bytes, res.Blob)
	if !eq {
		t.Errorf("Deserialized message to Go object failed:\n\tres: %v\n\torig: %v", res.Blob, bytes)
		return
	}

	eq = reflect.DeepEqual(m, res.Data)
	if !eq {
		t.Errorf("Deserialized message to Go object failed:\n\tres: %v\n\torig: %v", res.Data, m)
		return
	}
}
