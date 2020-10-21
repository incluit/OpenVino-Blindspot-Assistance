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
#include <eis/utils/config.h>

// Configuration context object for Go
typedef struct {
	int ref_id;
} go_config_t;


// Wrapper structure for receiving a message
typedef struct {
	msg_envelope_t* msg;
	msgbus_ret_t recv_ret;
} recv_wrapper_t;

// Wrapper for returning a void* with the msgbus_ret_t value for the underlying
// message bus call.
typedef struct {
	void* ret;
	msgbus_ret_t rc;
} void_wrapper_t;

// Extern method in Go to get the configuration value
extern void* go_get_config_value(int ref_id, char* key);

// Extern method for freeing the Go references to the configuration maps
extern void go_free_config(int ref_id);

// Extern method in Go to get configuration array value
extern void* go_get_array_idx(int ref_id, int idx);

// Create a new void_wrapper_t
static inline void_wrapper_t* new_void_wrapper(void* ret, msgbus_ret_t rc) {
	void_wrapper_t* wrap = (void_wrapper_t*) malloc(sizeof(void_wrapper_t));
	if(wrap == NULL) {
		return NULL;
	}
	wrap->ret = ret;
	wrap->rc = rc;
	return wrap;
}

// Free a void_wrapper_t
static inline void destroy_void_wrapper(void_wrapper_t* wrap) {
	free(wrap);
}

// Get configuration value method wrapper to strip constness because does not have const
static inline config_value_t* get_config_value(const void* obj, const char* key) {
	int ref_id = ((go_config_t*) obj)->ref_id;
	return (config_value_t*) go_get_config_value(ref_id, (char*) key);
}

// Get array value at index  method wrapper to strip constness because does not have const
static inline config_value_t* get_array_idx(const void* obj, int idx) {
	int ref_id = ((go_config_t*) obj)->ref_id;
	return (config_value_t*) go_get_array_idx(ref_id, idx);
}

// Free function wrapper for the go_config_t type
static inline void free_config(void* obj) {
	go_config_t* go_config = (go_config_t*) obj;
	go_free_config(go_config->ref_id);
	free(go_config);
}

// Wrapper for creating a new configuration object
static inline void* init_msgbus(int ref_id) {
	go_config_t* go_config = (go_config_t*) malloc(sizeof(go_config_t));
	if(go_config == NULL) {
		return NULL;
	}

	go_config->ref_id = ref_id;

	config_t* config = config_new((void*) go_config, free_config, get_config_value);
	if(config == NULL) {
		free(go_config);
		return NULL;
	}

	void* ctx = msgbus_initialize(config);
	if(ctx == NULL) {
		config_destroy(config);
		return NULL;
	}

	return ctx;
}

// Wrapper for creating a new config_value_t that is an object
static inline config_value_t* new_config_value_object(int ref_id) {
	go_config_t* go_config = (go_config_t*) malloc(sizeof(go_config_t));
	go_config->ref_id = ref_id;
	return config_value_new_object((void*) go_config, get_config_value, free_config);
}

// Wrapper for creating a new config_value_t that is an array
static inline config_value_t* new_config_value_array(int ref_id, int len) {
	go_config_t* go_config = (go_config_t*) malloc(sizeof(go_config_t));
	go_config->ref_id = ref_id;
	return config_value_new_array(
		(void*) go_config, (size_t) len, get_array_idx, free_config);
}

// Wrapper for creating a new publisher
static inline void_wrapper_t* new_publisher(void* ctx, char* topic) {
	publisher_ctx_t* pub_ctx = NULL;
	msgbus_ret_t ret = msgbus_publisher_new(ctx, topic, &pub_ctx);
	if(ret != MSG_SUCCESS) {
		void_wrapper_t* wrap = new_void_wrapper(NULL, ret);
		return wrap;
	}
	void_wrapper_t* wrap = new_void_wrapper((void*) pub_ctx, ret);
	if(wrap == NULL) {
		msgbus_publisher_destroy(ctx, pub_ctx);
		return NULL;
	}
	return wrap;
}

// Wrapper for creating a new subscriber
static inline void_wrapper_t* new_subscriber(void* ctx, char* topic) {
	recv_ctx_t* sub_ctx = NULL;
	msgbus_ret_t ret = msgbus_subscriber_new(ctx, topic, NULL, &sub_ctx);
	if(ret != MSG_SUCCESS) {
		void_wrapper_t* wrap = new_void_wrapper(NULL, ret);
		return wrap;
	}
	void_wrapper_t* wrap = new_void_wrapper((void*) sub_ctx, ret);
	if(wrap == NULL) {
		msgbus_recv_ctx_destroy(ctx, sub_ctx);
		return NULL;
	}
	return wrap;
}

// Wrapper for creating a new service
static inline void_wrapper_t* new_service(void* ctx, char* service_name) {
	recv_ctx_t* service_ctx = NULL;
	msgbus_ret_t ret = msgbus_service_new(ctx, service_name, NULL, &service_ctx);
	if(ret != MSG_SUCCESS) {
		void_wrapper_t* wrap = new_void_wrapper(NULL, ret);
		return wrap;
	}
	void_wrapper_t* wrap = new_void_wrapper((void*) service_ctx, ret);
	if(wrap == NULL) {
		msgbus_recv_ctx_destroy(ctx, service_ctx);
		return NULL;
	}
	return wrap;
}

// Wrapper for getting a service to issue requests to
static inline void_wrapper_t* get_service(void* ctx, char* service_name) {
	recv_ctx_t* service_ctx = NULL;
	msgbus_ret_t ret = msgbus_service_get(ctx, service_name, NULL, &service_ctx);
	if(ret != MSG_SUCCESS) {
		void_wrapper_t* wrap = new_void_wrapper(NULL, ret);
		return wrap;
	}
	void_wrapper_t* wrap = new_void_wrapper((void*) service_ctx, ret);
	if(wrap == NULL) {
		msgbus_recv_ctx_destroy(ctx, service_ctx);
		return NULL;
	}
	return wrap;
}

// msgbus_recv_wait() wrapper
static inline recv_wrapper_t recv_wait(void* ctx, recv_ctx_t* recv_ctx) {
	msg_envelope_t* env = NULL;
	msgbus_ret_t ret = msgbus_recv_wait(ctx, recv_ctx, &env);
	recv_wrapper_t wrap = { .msg = env, .recv_ret = ret };
	return wrap;
}

// msgbus_recv_nowait() wrapper
static inline recv_wrapper_t recv_nowait(void* ctx, recv_ctx_t* recv_ctx) {
	msg_envelope_t* env = NULL;
	msgbus_ret_t ret = msgbus_recv_nowait(ctx, recv_ctx, &env);
	recv_wrapper_t wrap = { .msg = env, .recv_ret = ret };
	return wrap;
}

// msgbus_recv_timedwait() wrapper
static inline recv_wrapper_t recv_timedwait(void* ctx, recv_ctx_t* recv_ctx, int timeout) {
	msg_envelope_t* env = NULL;
	msgbus_ret_t ret = msgbus_recv_timedwait(ctx, recv_ctx, timeout, &env);
	recv_wrapper_t wrap = { .msg = env, .recv_ret = ret };
	return wrap;
}
*/
import "C"
import (
	types "EISMessageBus/pkg/types"
	"encoding/json"
	"errors"
	"reflect"
	"sync"
	"unsafe"
)

//export go_get_config_value
func go_get_config_value(refId C.int, cKey *C.char) unsafe.Pointer {
	config := lookup((int)(refId))
	key := C.GoString(cKey)
	value, err := config.GetConfigValue(key)
	if err != nil {
		return nil
	}
	return value
}

//export go_get_array_idx
func go_get_array_idx(refId C.int, idx C.int) unsafe.Pointer {
	config := lookup((int)(refId))
	value, err := config.GetArrayIndex((int)(idx))
	if err != nil {
		return nil
	}
	return value
}

//export go_free_config
func go_free_config(refId C.int) {
	unregister((int)(refId))
}

// Message bus context object
type MsgbusContext struct {
	msgbusCtx unsafe.Pointer
}

// Publisher context wrapper
type PublisherContext struct {
	pubCtx unsafe.Pointer
}

// Receive context wrapper
type ReceiveContext struct {
	recvCtx unsafe.Pointer
}

// Internal configuration object wrapper
type configContext struct {
	// Only used if the configuration context object is an object
	config map[string]interface{}

	// Only used if the configuration context object is an array
	array []interface{}
}

// Helper function for creating a new configuration context structure
func newConfigContext(config map[string]interface{}, array []interface{}) *configContext {
	ctx := new(configContext)
	ctx.config = config
	ctx.array = array
	return ctx
}

// Get configuration value method
func (cfg *configContext) GetConfigValue(key string) (unsafe.Pointer, error) {
	// Verify that we are dealing with an object
	if cfg.config == nil {
		return nil, errors.New("Config value is an array, not an object")
	}

	// Retrieve value from the configuration map object
	value, ok := cfg.config[key]
	if !ok {
		return nil, errors.New("Config value does not exist")
	}

	// Convert value into a config_value_t
	return goToConfigValue(value)
}

// Get configuration value of an array configuration value
func (cfg *configContext) GetArrayIndex(idx int) (unsafe.Pointer, error) {
	// Verify that we are dealing with an array config value
	if cfg.array == nil {
		return nil, errors.New("Config value is an object, not an array")
	}

	// Verify that the index exists (since go has no sparse arrays)
	if len(cfg.array) <= idx {
		return nil, errors.New("Value at the array index does not exist")
	}

	// Get the value from the array
	value := cfg.array[idx]

	// Convert to config_value_t value if possible
	return goToConfigValue(value)
}

// Method to iniitalize a config_value_t C structure from a Go interface.
func goToConfigValue(value interface{}) (unsafe.Pointer, error) {
	var configValue unsafe.Pointer

	if jsonNumber, ok := value.(json.Number); ok {
		integer, err := jsonNumber.Int64()
		if err != nil {
			floating, _ := jsonNumber.Float64()
			configValue = unsafe.Pointer(C.config_value_new_floating(C.double(floating)))
		} else {
			configValue = unsafe.Pointer(C.config_value_new_integer(C.long(integer)))
		}
	} else {
		switch v := reflect.ValueOf(value); v.Kind() {
		case reflect.String:
			configValue = unsafe.Pointer(C.config_value_new_string(C.CString(value.(string))))
		case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
			configValue = unsafe.Pointer(C.config_value_new_integer(C.long(value.(int64))))
		case reflect.Float32, reflect.Float64:
			configValue = unsafe.Pointer(C.config_value_new_floating(C.double(value.(float64))))
		case reflect.Bool:
			configValue = unsafe.Pointer(C.config_value_new_boolean(C.bool(value.(bool))))
		case reflect.Map:
			config := newConfigContext(value.(map[string]interface{}), nil)
			refId := register(config)
			configValue = unsafe.Pointer(C.new_config_value_object(C.int(refId)))
			if configValue == nil {
				unregister(refId)
			}
		case reflect.Slice:
			arr := value.([]interface{})
			config := newConfigContext(nil, arr)
			refId := register(config)
			configValue = unsafe.Pointer(C.new_config_value_array(C.int(refId), C.int(len(arr))))
			if configValue == nil {
				unregister(refId)
			}
		default:
			return nil, errors.New("Unknown type in data map")
		}
	}

	if configValue == nil {
		return nil, errors.New("Failed to initialize configuration value")
	}

	return unsafe.Pointer(configValue), nil
}

// Initialize a new message bus context
func NewMsgbusContext(config map[string]interface{}) (*MsgbusContext, error) {
	conf := newConfigContext(config, nil)
	refId := register(conf)

	ctx := C.init_msgbus(C.int(refId))
	if ctx == nil {
		return nil, errors.New("Failed to initialize message bus context")
	}

	msgbusContext := new(MsgbusContext)
	msgbusContext.msgbusCtx = ctx

	return msgbusContext, nil
}

// Destroy the message bus context. Note that any calls to the context after
// calling this method will fail.
func (ctx *MsgbusContext) Destroy() {
	if ctx.msgbusCtx != nil {
		C.msgbus_destroy(ctx.msgbusCtx)
		ctx.msgbusCtx = nil
	}
}

// Create a new publisher context on the message bus context for the given topic.
func (ctx *MsgbusContext) NewPublisher(topic string) (*PublisherContext, error) {
	wrap := C.new_publisher(ctx.msgbusCtx, C.CString(topic))
	defer C.destroy_void_wrapper(wrap)
	if wrap.rc != C.MSG_SUCCESS {
		return nil, errors.New("Failed to initialize publisher")
	}
	pubCtx := new(PublisherContext)
	pubCtx.pubCtx = wrap.ret
	return pubCtx, nil
}

// Publish the given message on the publisher context.
func (ctx *MsgbusContext) Publish(pubCtx *PublisherContext, msg interface{}) error {
	if pubCtx.pubCtx == nil {
		return errors.New("Publisher context has already been destroyed")
	}

	env, err := GoToMsgEnvelope(msg)
	if err != nil {
		return err
	}
	defer MsgEnvelopeDestroy(env)

	c := (*C.publisher_ctx_t)(pubCtx.pubCtx)
	ret := C.msgbus_publisher_publish(ctx.msgbusCtx, c, (*C.msg_envelope_t)(env))
	if ret != C.MSG_SUCCESS {
		return errors.New("Failed to publish message")
	}

	return nil
}

// Destroy the given publisher context.
func (ctx *MsgbusContext) DestroyPublisher(pubCtx *PublisherContext) {
	if pubCtx.pubCtx != nil {
		C.msgbus_publisher_destroy(ctx.msgbusCtx, (*C.publisher_ctx_t)(pubCtx.pubCtx))
		pubCtx.pubCtx = nil
	}
}

// Subscribe to the given topic.
func (ctx *MsgbusContext) NewSubscriber(topic string) (*ReceiveContext, error) {
	wrap := C.new_subscriber(ctx.msgbusCtx, C.CString(topic))
	defer C.destroy_void_wrapper(wrap)
	if wrap.rc != C.MSG_SUCCESS {
		if wrap.rc == C.MSG_ERR_AUTH_FAILED {
			return nil, errors.New("Authentication failed")
		}
		return nil, errors.New("Subscription failed")
	}

	recvCtx := new(ReceiveContext)
	recvCtx.recvCtx = wrap.ret

	return recvCtx, nil
}

// Destroy the given ReceiveContext.
func (ctx *MsgbusContext) DestroyRecvCtx(recvCtx *ReceiveContext) {
	if recvCtx.recvCtx != nil {
		C.msgbus_recv_ctx_destroy(ctx.msgbusCtx, (*C.recv_ctx_t)(recvCtx.recvCtx))
		recvCtx.recvCtx = nil
	}
}

// Receive a message from the message bus on the given receive context. This receive
// method will block indefinitley.
func (ctx *MsgbusContext) ReceiveWait(recvCtx *ReceiveContext) (*types.MsgEnvelope, error) {
	recvRet := C.recv_wait(ctx.msgbusCtx, (*C.recv_ctx_t)(recvCtx.recvCtx))
	if recvRet.recv_ret != C.MSG_SUCCESS {
		if recvRet.recv_ret == C.MSG_ERR_EINTR {
			return nil, errors.New("Receive interrupted")
		}
		if recvRet.recv_ret == C.MSG_ERR_DISCONNECTED {
			return nil, errors.New("Disconnected")
		}
		if recvRet.recv_ret == C.MSG_ERR_AUTH_FAILED {
			return nil, errors.New("Authentication Failed")
		}
		return nil, errors.New("Failed to receive message")
	}

	defer C.msgbus_msg_envelope_destroy(recvRet.msg)

	msg, err := MsgEnvelopeToGo(unsafe.Pointer(recvRet.msg))
	if err != nil {
		return nil, err
	}

	return msg, nil
}

// Receive a message with no wait. Note that if there is no message available then
// both return values will be nil.
func (ctx *MsgbusContext) ReceiveNoWait(recvCtx *ReceiveContext) (*types.MsgEnvelope, error) {
	recvRet := C.recv_nowait(ctx.msgbusCtx, (*C.recv_ctx_t)(recvCtx.recvCtx))
	if recvRet.recv_ret == C.MSG_RECV_NO_MESSAGE {
		return nil, nil
	} else if recvRet.recv_ret != C.MSG_SUCCESS {
		if recvRet.recv_ret == C.MSG_ERR_EINTR {
			return nil, errors.New("Receive interrupted")
		}
		if recvRet.recv_ret == C.MSG_ERR_DISCONNECTED {
			return nil, errors.New("Disconnected")
		}
		if recvRet.recv_ret == C.MSG_ERR_AUTH_FAILED {
			return nil, errors.New("Authentication Failed")
		}
		return nil, errors.New("Failed to receive message")
	}

	defer C.msgbus_msg_envelope_destroy(recvRet.msg)

	msg, err := MsgEnvelopeToGo(unsafe.Pointer(recvRet.msg))
	if err != nil {
		return nil, err
	}

	return msg, nil
}

// Receive a message with the given timeout (in milliseconds). Note that if the timeout is reached, then
// both return values will be nil.
func (ctx *MsgbusContext) ReceiveTimedWait(recvCtx *ReceiveContext, timeout int) (*types.MsgEnvelope, error) {
	recvRet := C.recv_timedwait(ctx.msgbusCtx, (*C.recv_ctx_t)(recvCtx.recvCtx), C.int(timeout))
	if recvRet.recv_ret == C.MSG_RECV_NO_MESSAGE {
		return nil, nil
	} else if recvRet.recv_ret != C.MSG_SUCCESS {
		if recvRet.recv_ret == C.MSG_ERR_EINTR {
			return nil, errors.New("Receive interrupted")
		}
		if recvRet.recv_ret == C.MSG_ERR_DISCONNECTED {
			return nil, errors.New("Disconnected")
		}
		if recvRet.recv_ret == C.MSG_ERR_AUTH_FAILED {
			return nil, errors.New("Authentication Failed")
		}
		return nil, errors.New("Failed to receive message")
	}

	defer C.msgbus_msg_envelope_destroy(recvRet.msg)

	msg, err := MsgEnvelopeToGo(unsafe.Pointer(recvRet.msg))
	if err != nil {
		return nil, err
	}

	return msg, nil
}

// Create a new service to receive requests and send responses over.
func (ctx *MsgbusContext) NewService(serviceName string) (*ReceiveContext, error) {
	wrap := C.new_service(ctx.msgbusCtx, C.CString(serviceName))
	defer C.destroy_void_wrapper(wrap)
	if wrap.rc != C.MSG_SUCCESS {
		return nil, errors.New("Failed to initialize a new service")
	}

	recvCtx := new(ReceiveContext)
	recvCtx.recvCtx = wrap.ret

	return recvCtx, nil
}

// Get the context of a service to issue requests to and receive responses from.
func (ctx *MsgbusContext) GetService(serviceName string) (*ReceiveContext, error) {
	wrap := C.get_service(ctx.msgbusCtx, C.CString(serviceName))
	defer C.destroy_void_wrapper(wrap)
	if wrap.rc != C.MSG_SUCCESS {
		if wrap.rc == C.MSG_ERR_AUTH_FAILED {
			return nil, errors.New("Authentication failed")
		}
		return nil, errors.New("Failed to get service")
	}

	recvCtx := new(ReceiveContext)
	recvCtx.recvCtx = wrap.ret

	return recvCtx, nil
}

// Send a request to a service.
func (ctx *MsgbusContext) Request(serviceCtx *ReceiveContext, request interface{}) error {
	if serviceCtx.recvCtx == nil {
		return errors.New("Service context already destroyed")
	}

	env, err := GoToMsgEnvelope(request)
	if err != nil {
		return err
	}
	defer MsgEnvelopeDestroy(env)

	c := (*C.recv_ctx_t)(serviceCtx.recvCtx)
	ret := C.msgbus_request(ctx.msgbusCtx, c, (*C.msg_envelope_t)(env))
	if ret != C.MSG_SUCCESS {
		return errors.New("Failed to send request")
	}

	return nil
}

// Send response to a request
func (ctx *MsgbusContext) Response(serviceCtx *ReceiveContext, response interface{}) error {
	if serviceCtx.recvCtx == nil {
		return errors.New("Service context already destroyed")
	}

	env, err := GoToMsgEnvelope(response)
	if err != nil {
		return err
	}
	defer MsgEnvelopeDestroy(env)

	c := (*C.recv_ctx_t)(serviceCtx.recvCtx)
	ret := C.msgbus_response(ctx.msgbusCtx, c, (*C.msg_envelope_t)(env))
	if ret != C.MSG_SUCCESS {
		return errors.New("Failed to send response")
	}

	return nil
}

// Methods used for retrieving configuration objects in C
var mu sync.Mutex
var index int
var configRefs = make(map[int]*configContext)

func register(config *configContext) int {
	mu.Lock()
	defer mu.Unlock()
	index++
	for configRefs[index] != nil {
		index++
	}
	configRefs[index] = config
	return index
}

func lookup(i int) *configContext {
	mu.Lock()
	defer mu.Unlock()
	return configRefs[i]
}

func unregister(i int) {
	mu.Lock()
	defer mu.Unlock()
	delete(configRefs, i)
}
