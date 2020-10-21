# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""All C imports for the EIS message bus
"""

from libc.stdint cimport *

cdef extern from "stdbool.h":
    ctypedef bint bool

cdef extern from "eis/msgbus/msgbus.h" nogil:
    ctypedef enum msgbus_ret_t:
        MSG_SUCCESS = 0
        MSG_ERR_PUB_FAILED = 1
        MSG_ERR_SUB_FAILED = 2
        MSG_ERR_RESP_FAILED = 3
        MSG_ERR_RECV_FAILED = 4
        MSG_ERR_RECV_EMPTY = 5
        MSG_ERR_ALREADY_RECEIVED = 6
        MSG_ERR_NO_SUCH_SERVICE = 7
        MSG_ERR_SERVICE_ALREADY_EXIST = 8
        MSG_ERR_BUS_CONTEXT_DESTROYED = 9
        MSG_ERR_NO_MEMORY = 10
        MSG_ERR_ELEM_NOT_EXIST = 11
        MSG_ERR_ELEM_ALREADY_EXISTS = 12
        MSG_ERR_ELEM_BLOB_ALREADY_SET = 13
        MSG_ERR_ELEM_BLOB_MALFORMED = 14
        MSG_RECV_NO_MESSAGE = 15
        MSG_ERR_SERVICE_INIT_FAILED = 16
        MSG_ERR_REQ_FAILED = 17
        MSG_ERR_EINTR = 18
        MSG_ERR_MSG_SEND_FAILED = 19
        MSG_ERR_DISCONNECTED = 20
        MSG_ERR_AUTH_FAILED = 21
        MSG_ERR_ELEM_OBJ = 22
        MSG_ERR_ELEM_ARR = 23
        MSG_ERR_DESERIALIZE_FAILED = 24
        MSG_ERR_UNKNOWN = 255

    ctypedef struct owned_blob_t:
        void* ptr
        void (*free)(void*)
        bool owned
        size_t len
        const char* bytes

    ctypedef enum config_value_type_t:
        CVT_INTEGER  = 0
        CVT_FLOATING = 1
        CVT_STRING   = 2
        CVT_BOOLEAN  = 3
        CVT_OBJECT   = 4
        CVT_ARRAY    = 5
        CVT_NONE     = 6

    # Forward declaration
    ctypedef struct config_value_t

    ctypedef struct config_value_object_t:
        void* object
        void (*free)(void* object)

    ctypedef struct config_value_array_t:
        void* array
        size_t length
        config_value_t* (*get)(void*,int)
        void (*free)(void*)

    ctypedef union config_value_type_body_union_t:
        int64_t      integer
        double       floating
        const char*  string
        bool         boolean
        config_value_object_t* object
        config_value_array_t* array

    ctypedef struct config_value_t:
        config_value_type_t type
        config_value_type_body_union_t body

    ctypedef enum content_type_t:
        CT_JSON = 0
        CT_BLOB = 1

    ctypedef struct hashmap_t:
        pass

    ctypedef struct linkedlist_t:
        pass

    ctypedef enum msg_envelope_data_type_t:
        MSG_ENV_DT_INT      = 0
        MSG_ENV_DT_FLOATING = 1
        MSG_ENV_DT_STRING   = 2
        MSG_ENV_DT_BOOLEAN  = 3
        MSG_ENV_DT_BLOB     = 4
        MSG_ENV_DT_OBJECT   = 5
        MSG_ENV_DT_ARRAY    = 6
        MSG_ENV_DT_NONE     = 7

    ctypedef struct msg_envelope_blob_t:
        owned_blob_t* shared
        uint64_t len
        const char*    data

    ctypedef union msg_envelope_elem_body_body_t:
        int64_t integer
        double floating
        char* string
        bool boolean
        msg_envelope_blob_t* blob
        hashmap_t* object
        linkedlist_t* array

    ctypedef struct msg_envelope_elem_body_t:
        msg_envelope_data_type_t type
        msg_envelope_elem_body_body_t body

    ctypedef struct msg_envelope_elem_t:
        char* key
        bool in_use
        msg_envelope_elem_body_t* body

    ctypedef struct msg_envelope_t:
        char* name
        char* correlation_id
        content_type_t content_type
        int size
        int max_size
        msg_envelope_elem_t* elems
        msg_envelope_elem_body_t* blob

    ctypedef struct msg_envelope_serialized_part_t:
        size_t len
        char* bytes

    ctypedef struct user_data_t:
        pass

    ctypedef struct recv_ctx_t:
        pass

    ctypedef void* publisher_ctx_t

    ctypedef struct config_t:
        pass

    ctypedef void (*msgbus_free_fn)(void*)
    ctypedef config_value_t* (*get_config_value_fn)(const void*,const char*)

    config_t* config_new(
            void* cfg, msgbus_free_fn free_fn,
            get_config_value_fn get_config_value)
    config_value_t* config_value_new_integer(int64_t value)
    config_value_t* config_value_new_floating(double value)
    config_value_t* config_value_new_string(const char* value)
    config_value_t* config_value_new_boolean(bool value)
    config_value_t* config_value_new_object(
            void* value, config_value_t* (*get)(const void*,const char*),
            void (*free_fn)(void*));
    config_value_t* config_value_new_array(
            void* array, size_t length, config_value_t* (*get)(const void*,int),
            void (*free_fn)(void*));
    void config_value_destroy(config_value_t* value)
    void config_destroy(config_t* config)
    void* msgbus_initialize(config_t* config)
    void msgbus_destroy(void* ctx)
    msgbus_ret_t msgbus_publisher_new(
            void* ctx, const char* topic, publisher_ctx_t** pub_ctx)
    msgbus_ret_t msgbus_publisher_publish(
            void* ctx, publisher_ctx_t* pub_ctx, msg_envelope_t* message)
    void msgbus_publisher_destroy(void* ctx, publisher_ctx_t* pub_ctx)
    msgbus_ret_t msgbus_subscriber_new(
            void* ctx, const char* topic, user_data_t* user_data,
            recv_ctx_t** subscriber)
    void msgbus_recv_ctx_destroy(void* ctx, recv_ctx_t* recv_ctx)
    msgbus_ret_t msgbus_request(
            void* ctx, recv_ctx_t* service_ctx, msg_envelope_t* message)
    msgbus_ret_t msgbus_response(
            void* ctx, recv_ctx_t* service_ctx, msg_envelope_t* message)
    msgbus_ret_t msgbus_service_get(
            void* ctx, const char* service_name, void* user_data,
            recv_ctx_t** service_ctx)
    msgbus_ret_t msgbus_service_new(
            void* ctx, const char* service_name, void* user_data,
            recv_ctx_t** service_ctx)
    msgbus_ret_t msgbus_recv_wait(
            void* ctx, recv_ctx_t* recv_ctx, msg_envelope_t** message)
    msgbus_ret_t msgbus_recv_timedwait(
            void* ctx, recv_ctx_t* recv_ctx, int timeout,
            msg_envelope_t** message)
    msgbus_ret_t msgbus_recv_nowait(
            void* ctx, recv_ctx_t* recv_ctx, msg_envelope_t** message)
    msg_envelope_t* msgbus_msg_envelope_new(content_type_t ct)
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_string(
            const char* string)
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_none()
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_array()
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_object()
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_integer(int64_t integer)
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_floating(double floating)
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_bool(bool boolean)
    msg_envelope_elem_body_t* msgbus_msg_envelope_new_blob(
            const char* data, size_t len)
    msgbus_ret_t msgbus_msg_envelope_elem_object_put(
            msg_envelope_elem_body_t* obj, const char* key,
            msg_envelope_elem_body_t* value)
    msg_envelope_elem_body_t* msgbus_msg_envelope_elem_object_get(
            msg_envelope_elem_body_t* obj, const char* key)
    msgbus_ret_t msgbus_msg_envelope_elem_object_remove(
            msg_envelope_elem_body_t* obj, const char* key)
    msgbus_ret_t msgbus_msg_envelope_elem_array_add(
            msg_envelope_elem_body_t* arr,
            msg_envelope_elem_body_t* value)
    msg_envelope_elem_body_t* msgbus_msg_envelope_elem_array_get_at(
            msg_envelope_elem_body_t* arr, int idx)
    msgbus_ret_t msgbus_msg_envelope_elem_array_remove_at(
            msg_envelope_elem_body_t* arr, int idx)
    void msgbus_msg_envelope_elem_destroy(msg_envelope_elem_body_t* elem)
    msgbus_ret_t msgbus_msg_envelope_put(
            msg_envelope_t* env, const char* key,
            msg_envelope_elem_body_t* data)
    msgbus_ret_t msgbus_msg_envelope_remove(
            msg_envelope_t* env, const char* key)
    msgbus_ret_t msgbus_msg_envelope_get(
            msg_envelope_t* env, const char* key,
            msg_envelope_elem_body_t** data)
    int msgbus_msg_envelope_serialize(
            msg_envelope_t* env, msg_envelope_serialized_part_t** parts)
    msgbus_ret_t msgbus_msg_envelope_deserialize(
            content_type_t ct, msg_envelope_serialized_part_t* data,
            size_t num_parts, const char* name, msg_envelope_t** env)
    msgbus_ret_t msgbus_msg_envelope_serialize_parts_new(
            int num_parts, msg_envelope_serialized_part_t** parts)
    void msgbus_msg_envelope_serialize_destroy(
            msg_envelope_serialized_part_t* parts, int num_parts)
    void msgbus_msg_envelope_destroy(msg_envelope_t* msg)
