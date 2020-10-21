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

/*
#cgo CFLAGS: -g -Wall
#cgo pkg-config: libeismsgbus libeismsgenv libeisutils

#include <stdlib.h>
#include <eis/msgbus/msgbus.h>
#include <eis/utils/logger.h>

// The code below is for helping deal with the eccentricities of cgo

typedef struct {
	int num_parts;
	msg_envelope_serialized_part_t* parts;
} parts_wrapper_t;

parts_wrapper_t* get_parts(msg_envelope_t* msg) {
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if(num_parts <= 0) {
		return NULL;
	} else {
		parts_wrapper_t* wrap = (parts_wrapper_t*) malloc(sizeof(parts_wrapper_t));
		wrap->num_parts = num_parts;
		wrap->parts = parts;
		return wrap;
	}
}

void free_parts_wrapper(parts_wrapper_t* wrap) {
	msgbus_msg_envelope_serialize_destroy(wrap->parts, wrap->num_parts);
	free(wrap);
}

char* get_part_bytes(parts_wrapper_t* wrap, int idx) {
	return (char*) wrap->parts[idx].bytes;
}

int get_part_len(parts_wrapper_t* wrap, int idx) {
	return (int) wrap->parts[idx].len;
}
*/
import "C"
import (
	types "EISMessageBus/pkg/types"
	"encoding/json"
	"errors"
	"reflect"
	"unsafe"
)

// Global for checking if slice is a []byte slice
var typeOfBytes = reflect.TypeOf([]byte(nil))

// Convert Go message envelope representation to C message envelope representation
func GoToMsgEnvelope(env interface{}) (unsafe.Pointer, error) {
	var msg unsafe.Pointer
	envValue := reflect.ValueOf(env)
	envKind := envValue.Kind()

	if envKind == reflect.Map {
		jsonMsg := C.msgbus_msg_envelope_new(C.CT_JSON)
		msg = unsafe.Pointer(jsonMsg)
		err := addMapToMsgEnvelope(msg, env.(map[string]interface{}))
		if err != nil {
			C.msgbus_msg_envelope_destroy(jsonMsg)
			return nil, err
		}
	} else if envKind == reflect.Slice {
		if envValue.Type() == typeOfBytes {
			blobMsg := C.msgbus_msg_envelope_new(C.CT_BLOB)
			msg = unsafe.Pointer(blobMsg)
			err := addBytesToMsgEnvelope(msg, env.([]byte))
			if err != nil {
				C.msgbus_msg_envelope_destroy(blobMsg)
				return nil, err
			}
		} else {
			slice := env.([]interface{})
			if len(slice) != 2 {
				return nil, errors.New("Slice can only contain 2 elements")
			}

			first := slice[0]
			firstValue := reflect.ValueOf(first)
			firstKind := firstValue.Kind()

			second := slice[1]
			secondValue := reflect.ValueOf(second)
			secondKind := secondValue.Kind()

			if firstKind == secondKind {
				return nil, errors.New("Elements must have only one map[string]interface{} and one []byte")
			}

			multiMsg := C.msgbus_msg_envelope_new(C.CT_JSON)
			msg = unsafe.Pointer(multiMsg)

			if firstKind == reflect.Map {
				err := addMapToMsgEnvelope(msg, first.(map[string]interface{}))
				if err != nil {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, err
				}
			} else if firstKind == reflect.Slice {
				if firstValue.Type() != typeOfBytes {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, errors.New("Slice must be []byte")
				}

				err := addBytesToMsgEnvelope(msg, first.([]byte))
				if err != nil {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, err
				}
			} else {
				C.msgbus_msg_envelope_destroy(multiMsg)
				return nil, errors.New("Unknown data type, must be map[string]interface{} or []byte")
			}

			if secondKind == reflect.Map {
				err := addMapToMsgEnvelope(msg, second.(map[string]interface{}))
				if err != nil {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, err
				}
			} else if secondKind == reflect.Slice {
				if secondValue.Type() != typeOfBytes {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, errors.New("Slice must be []byte")
				}

				err := addBytesToMsgEnvelope(msg, second.([]byte))
				if err != nil {
					C.msgbus_msg_envelope_destroy(multiMsg)
					return nil, err
				}
			} else {
				C.msgbus_msg_envelope_destroy(multiMsg)
				return nil, errors.New("Unknown data type, must be map[string]interface{} or []byte")
			}
		}
	}

	return msg, nil
}

func MsgEnvelopeDestroy(msg unsafe.Pointer) {
	if msg != nil {
		C.msgbus_msg_envelope_destroy((*C.msg_envelope_t)(msg))
	}
}

func goToMsgEnvElemBody(data interface{}) (unsafe.Pointer, error) {
	var ret C.msgbus_ret_t
	var elem unsafe.Pointer

	if data == nil {
		body := C.msgbus_msg_envelope_new_none()
		if body == nil {
			return nil, errors.New("Failed to initialize none msg envelope element")
		}

		elem = unsafe.Pointer(body)
		return elem, nil
	}

	switch v := reflect.ValueOf(data); v.Kind() {
	case reflect.String:
		body := C.msgbus_msg_envelope_new_string(C.CString(data.(string)))
		if body == nil {
			return nil, errors.New("Error adding msg envelope element")
		}
		elem = unsafe.Pointer(body)
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		body := C.msgbus_msg_envelope_new_integer(C.long(data.(int)))
		if body == nil {
			return nil, errors.New("Error adding msg envelope element")
		}
		elem = unsafe.Pointer(body)
	case reflect.Float32, reflect.Float64:
		body := C.msgbus_msg_envelope_new_floating(C.double(data.(float64)))
		if body == nil {
			return nil, errors.New("Error adding msg envelope element")
		}
		elem = unsafe.Pointer(body)
	case reflect.Bool:
		body := C.msgbus_msg_envelope_new_bool(C.bool(data.(bool)))
		if body == nil {
			return nil, errors.New("Error adding msg envelope element")
		}
		elem = unsafe.Pointer(body)
	case reflect.Map:
		body := C.msgbus_msg_envelope_new_object()
		if body == nil {
			return nil, errors.New("Error initializing msg envelope object element")
		}

		map_data := data.(map[string]interface{})
		for key, value := range map_data {
			subelem, err := goToMsgEnvElemBody(value)
			if err != nil {
				C.msgbus_msg_envelope_elem_destroy(body)
				return nil, errors.New("Failed to initialize object sub element")
			}

			ret = C.msgbus_msg_envelope_elem_object_put(body, C.CString(key), (*C.msg_envelope_elem_body_t)(subelem))
			if ret != C.MSG_SUCCESS {
				C.msgbus_msg_envelope_elem_destroy((*C.msg_envelope_elem_body_t)(subelem))
				C.msgbus_msg_envelope_elem_destroy(body)
				return nil, errors.New("Failed to put sub-element into object element")
			}
		}

		elem = unsafe.Pointer(body)
	case reflect.Slice:
		body := C.msgbus_msg_envelope_new_array()
		if body == nil {
			return nil, errors.New("Error initializing msg envelope object element")
		}

		slice_data := data.([]interface{})
		for _, value := range slice_data {
			subelem, err := goToMsgEnvElemBody(value)
			if err != nil {
				C.msgbus_msg_envelope_elem_destroy(body)
				return nil, errors.New("Failed to initialize array sub element")
			}

			ret = C.msgbus_msg_envelope_elem_array_add(body, (*C.msg_envelope_elem_body_t)(subelem))
			if ret != C.MSG_SUCCESS {
				C.msgbus_msg_envelope_elem_destroy((*C.msg_envelope_elem_body_t)(subelem))
				C.msgbus_msg_envelope_elem_destroy(body)
				return nil, errors.New("Failed to put sub-element into array element")
			}
		}

		elem = unsafe.Pointer(body)
	default:
		return nil, errors.New("Unknown type in data map")
	}

	return elem, nil
}

func addMapToMsgEnvelope(env unsafe.Pointer, data map[string]interface{}) error {
	var ret C.msgbus_ret_t
	msg := (*C.msg_envelope_t)(env)

	for key, value := range data {
		elem, err := goToMsgEnvElemBody(value)
		if err != nil {
			return errors.New("Failed to convert golang value to msg envelope element")
		}

		body := (*C.msg_envelope_elem_body_t)(elem)
		ret = C.msgbus_msg_envelope_put(msg, C.CString(key), body)
		if ret != C.MSG_SUCCESS {
			C.msgbus_msg_envelope_elem_destroy(body)
			return errors.New("Failed to add msg envelope element")
		}
	}

	return nil
}

func addBytesToMsgEnvelope(msg unsafe.Pointer, data []byte) error {
	env := (*C.msg_envelope_t)(msg)

	elem := C.msgbus_msg_envelope_new_blob(C.CString(string(data)), C.size_t(len(data)))
	if elem == nil {
		return errors.New("Failed to initialize blob")
	}

	ret := C.msgbus_msg_envelope_put(env, nil, elem)
	if ret != C.MSG_SUCCESS {
		C.msgbus_msg_envelope_elem_destroy(elem)
		return errors.New("Failed to add blob element")
	}

	return nil
}

func MsgEnvelopeToGo(msg unsafe.Pointer) (*types.MsgEnvelope, error) {
	res := new(types.MsgEnvelope)
	env := (*C.msg_envelope_t)(msg)
	parts := C.get_parts(env)
	if parts == nil {
		return nil, errors.New("Failed to serialize message envelope parts")
	}

	defer C.free_parts_wrapper(parts)

	if env.name != nil {
		res.Name = C.GoString(env.name)
	}
	if env.content_type == C.CT_BLOB {
		res.Blob = C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 0)), C.get_part_len(parts, 0))
		res.Data = nil
	} else {
		n := C.get_part_len(parts, 0)
		jsonBytes := C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 0)), n)

		if env.blob != nil {
			res.Blob = C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 1)), C.get_part_len(parts, 1))
		} else {
			res.Blob = nil
		}

		err := json.Unmarshal(jsonBytes, &res.Data)
		if err != nil {
			return nil, err
		}
	}

	return res, nil
}
