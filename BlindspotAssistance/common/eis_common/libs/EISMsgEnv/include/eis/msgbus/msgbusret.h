// Copyright (c) 2019 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief Messaging return codes
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#ifndef _EIS_MESSAGE_BUS_MSGRET_H
#define _EIS_MESSAGE_BUS_MSGRET_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return type for messaging actions.
 */
typedef enum {
    MSG_SUCCESS = 0,
    MSG_ERR_PUB_FAILED = 1,
    MSG_ERR_SUB_FAILED = 2,
    MSG_ERR_RESP_FAILED = 3,
    MSG_ERR_RECV_FAILED = 4,
    MSG_ERR_RECV_EMPTY = 5,
    MSG_ERR_ALREADY_RECEIVED = 6,
    MSG_ERR_NO_SUCH_SERVICE = 7,
    MSG_ERR_SERVICE_ALREADY_EXIST = 8,
    MSG_ERR_BUS_CONTEXT_DESTROYED = 9,
    MSG_ERR_NO_MEMORY = 10,
    MSG_ERR_ELEM_NOT_EXIST = 11,
    MSG_ERR_ELEM_ALREADY_EXISTS = 12,
    MSG_ERR_ELEM_BLOB_ALREADY_SET = 13,
    MSG_ERR_ELEM_BLOB_MALFORMED = 14,
    MSG_RECV_NO_MESSAGE = 15,
    MSG_ERR_SERVICE_INIT_FAILED = 16,
    MSG_ERR_REQ_FAILED = 17,
    MSG_ERR_EINTR = 18,
    MSG_ERR_MSG_SEND_FAILED = 19,
    MSG_ERR_DISCONNECTED = 20,
    MSG_ERR_AUTH_FAILED = 21,
    MSG_ERR_ELEM_OBJ = 22,
    MSG_ERR_ELEM_ARR = 23,
    MSG_ERR_DESERIALIZE_FAILED = 24,
    MSG_ERR_UNKNOWN = 255,
} msgbus_ret_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _EIS_MESSAGE_BUS_MSGRET_H
