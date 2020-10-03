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
 * @brief Messaging envelope abstraction
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#ifndef _EIS_MESSAGE_BUS_MSGENV_H
#define _EIS_MESSAGE_BUS_MSGENV_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "eis/msgbus/msgbusret.h"
#include "eis/msgbus/hashmap.h"
#include "eis/msgbus/linkedlist.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Content types
 */
typedef enum {
    CT_JSON = 0,

    // For custom serialized data by the user (assumed done by the caller of
    // the `msgbus_recv()` prior to calling the method)
    CT_BLOB = 1,

    // TODO: ADD support for CBOR
    // CT_CBOR = 2,
} content_type_t;

/**
 * Message envelope value data types.
 */
typedef enum {
    MSG_ENV_DT_INT      = 0,
    MSG_ENV_DT_FLOATING = 1,
    MSG_ENV_DT_STRING   = 2,
    MSG_ENV_DT_BOOLEAN  = 3,
    MSG_ENV_DT_BLOB     = 4,
    MSG_ENV_DT_OBJECT   = 5,
    MSG_ENV_DT_ARRAY    = 6,
    MSG_ENV_DT_NONE     = 7,
} msg_envelope_data_type_t;

/**
 * Shared object structure for message bus data blobs.
 */
typedef struct {
    void* ptr;
    void (*free)(void*);
    bool owned;

    size_t len;
    const char* bytes;
} owned_blob_t;

/**
 * Message envelope blob data type.
 */
typedef struct {
    owned_blob_t* shared;

    uint64_t len;
    const char*    data;
} msg_envelope_blob_t;

/**
 * Message envelope element body type.
 */
typedef struct {
    msg_envelope_data_type_t type;

    union {
        int64_t              integer;
        double               floating;
        char*                string;
        bool                 boolean;
        msg_envelope_blob_t* blob;
        hashmap_t*           object;
        linkedlist_t*        array;
    } body;
} msg_envelope_elem_body_t;

/**
 * Message envelope around a given message that is to be sent or received over
 * the message bus.
 */
typedef struct {
    char* name;
    char* correlation_id;
    content_type_t content_type;

    // Internal tracking for (key, value) pairs
    hashmap_t* map;

    // Internal tracking for blob data
    msg_envelope_elem_body_t* blob;
} msg_envelope_t;

/**
 * Part of a serialized message envelope.
 */
typedef struct {
    owned_blob_t* shared;

    // Convenience values
    size_t len;
    const char* bytes;
} msg_envelope_serialized_part_t;

/**
 * Create a new msg_envelope_t to be sent over the message bus.
 *
 * @param ct      - Content type
 * @return msg_envelope_t, or NULL if an error occurs
 */
msg_envelope_t* msgbus_msg_envelope_new(content_type_t ct);

/**
 * Create a new empty msg envelope element.
 *
 * @return @c msg_envelope_elem_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_none();

/**
 * Create a new array element to be added to a message envelope.
 *
 * @return @c msg_envelope_elem_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_array();

/**
 * Create a new nested object to be added to a message envelope, array, or
 * other object.
 *
 * @return @c msg_envelope_elem_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_object();

/**
 * Helper function for creating a new message envelope element containing
 * a string value.
 *
 * @param string - String value to be placed in the envelope element
 * @return msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_string(const char* string);

/**
 * Helper function for creating a new message envelope element containing
 * an integer value.
 *
 * @param integer - Integer value to be placed in the envelope element
 * @return msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_integer(int64_t integer);

/**
 * Helper function for creating a new message envelope element containing
 * a floating point value.
 *
 * @param floating - Floating point value to be placed in the envelope element
 * @return msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_floating(double floating);

/**
 * Helper function for creating a new message envelope element containing
 * a boolean value.
 *
 * @param boolean - Boolean value to be placed in the envelope element
 * @return msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_bool(bool boolean);

/**
 * Helper function for creating a new message envelope element containing
 * a data blob.
 *
 * \note The enevelope element takes ownership of releasing the data.
 *
 * @param blob - Blob data to be placed in the envelope element
 * @param len  - Size of the data blob
 * @return msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_new_blob(
        char* data, size_t len);

/**
 * Put a new (key, value) pair into a message envelope nested object.
 *
 * @param obj   - Message envelope object element to add the (key, value) pair to
 * @param key   - Key for the value
 * @param value - Value associated to the key
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_elem_object_put(
        msg_envelope_elem_body_t* obj, const char* key,
        msg_envelope_elem_body_t* value);

/**
 * Get value from a message envelope nested object.
 *
 * @param obj - Message envelope object element to retrieve value from
 * @param key - Key of the value to retrieve
 * @return @c msg_envelope_body_t, or NULL if errors occur
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_elem_object_get(
        msg_envelope_elem_body_t* obj, const char* key);

/**
 * Remove (key, value) pair from the message envelope nested object.
 *
 * @param obj - Message envelope object element to remove element from
 * @param key - Key to remove
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_elem_object_remove(
        msg_envelope_elem_body_t* obj, const char* key);

/**
 * Add item to the msg envelope array element.
 *
 * @param arr   - Array to add the element to
 * @param value - Element to add
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_elem_array_add(
        msg_envelope_elem_body_t* arr,
        msg_envelope_elem_body_t* value);

/**
 * Get item in the msg envelope array element.
 *
 * @param arr - Array to remove the element from
 * @param idx - Index of the item to get
 * @return msg_envelope_elem_body_t, or NULL if an error occurs
 */
msg_envelope_elem_body_t* msgbus_msg_envelope_elem_array_get_at(
        msg_envelope_elem_body_t* arr, int idx);

/**
 * Remove item to the msg envelope array element.
 *
 * @param arr   - Array to remove the element from
 * @param value - Element to remove
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_elem_array_remove_at(
        msg_envelope_elem_body_t* arr, int idx);

/**
 * Helper function to destroy a message envelope element.
 *
 * @param elem - Element to destroy
 */
void msgbus_msg_envelope_elem_destroy(msg_envelope_elem_body_t* elem);

/**
 * Add (key, value) pair to the message envelope.
 *
 * \note{If the message envelope is set to be a `CT_BLOB`, then it will act
 *  differently than a message set to a different content type. For a blob the
 *  data can only be set once for the message and the key value will be ignored
 *  and the key `BLOB` will be used. Additionally, the value body must be a
 *  blob as well.}
 *
 * @param env  - Message envelope
 * @param key  - Key for the value
 * @param data - Value to be added
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_put(
        msg_envelope_t* env, const char* key, msg_envelope_elem_body_t* data);

/**
 * Remove the (key, value) pair with the given key.
 *
 * @param env - Message envelope
 * @param key - Key to remove
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_remove(msg_envelope_t* env, const char* key);

/**
 * Get the value for the given key in the message bus.
 *
 * \note{If the content type is `CT_BLOB`, then use "BLOB" as the key to
 *  retrieve the blob data.}
 *
 * @param[in]  env  - Message envelope
 * @param[in]  key  - Key for the element to find
 * @param[out] data - Data for the key
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_get(
        msg_envelope_t* env, const char* key, msg_envelope_elem_body_t** data);

/**
 * Serialize the data in the envelope into the given message parts buffer based
 * on the content type given when msgbus_msg_envelope_new() was called.
 *
 * @param[in]  env   - Message envelope
 * @param[out] parts - Serialized parts
 * @return Number of serialized message parts
 */
int msgbus_msg_envelope_serialize(
        msg_envelope_t* env, msg_envelope_serialized_part_t** parts);

/**
 * Deserialize the given data into a msg_envelope_t.
 *
 * If the content type is set to CT_BLOB, then this method assumes that there
 * will only be one serialized message part.
 *
 * Additionally, if the content type is CT_JSON, then this method assumes that
 * there will be either one or two message parts. The first part MUST always
 * be a JSON string. If a second part is present it MUST be a binary blob of
 * data.
 *
 * @param[in]  ct        - Message content type
 * @param[in]  parts     - Serialized parts to deserailize
 * @param[in]  num_parts - Number of message parts
 * @param[in] name       - Topic name
 * @param[out] env       - Output message envelope
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_deserialize(
        content_type_t ct, msg_envelope_serialized_part_t* parts,
        int num_parts, const char* name, msg_envelope_t** env);

/**
 * Create a new list of serialized message parts.
 *
 * @param[in]  num_parts - Number of serialized message parts
 * @param[out] parts     - Serialzied parts
 * @return msgbus_ret_t
 */
msgbus_ret_t msgbus_msg_envelope_serialize_parts_new(
        int num_parts, msg_envelope_serialized_part_t** parts);

/**
 * Destroy the serialized parts of a message
 *
 * @param parts     - Serialized parts
 * @param num_parts - Number of serialized parts
 * @return msgbus_ret_t
 */
void msgbus_msg_envelope_serialize_destroy(
        msg_envelope_serialized_part_t* parts, int num_parts);

/**
 * Delete and clean up a message envelope structure.
 *
 * @param msg - Message envelope to delete
 */
void msgbus_msg_envelope_destroy(msg_envelope_t* msg);

/**
 * Helper for initializing owned blob pointer.
 *
 * \note Assumes data is owned
 */
owned_blob_t* owned_blob_new(
        void* ptr, void (*free_fn)(void*), const char* data, size_t len);

/**
 * Copy a shared blob, except assume the underlying data is NOT owned by the
 * copy of the blob.
 */
owned_blob_t* owned_blob_copy(owned_blob_t* to_copy);

/**
 * Helper for destroying owned blob pointer.
 */
void owned_blob_destroy(owned_blob_t* shared);

#ifdef __cplusplus
} // extern "C"

// If in C++, then add Serializable interface for objects which can be
// serialized into a msg_envelope_t* structure
namespace eis {
namespace msgbus {

/**
 * Base interface for objects which can be serialized into @c msg_envelope_t
 * structures.
 */
class Serializable {
protected:
    msg_envelope_t* m_msg;

public:
    /**
     * Default constructor.
     */
    Serializable() : m_msg(NULL) {};

    /**
     * Constructor.
     *
     * \note Subclasses should not destroy the message, that is handled by the
     *      parents destructor.
     *
     * @param msg - Message to deserialize
     */
    Serializable(msg_envelope_t* msg) : m_msg(msg) {};

    /**
     * Destructor
     */
    virtual ~Serializable() {
        if(m_msg != NULL) {
            msgbus_msg_envelope_destroy(m_msg);
        }
    };

    /**
     * Method to be overriden by subclasses which shall be called to serialize
     * the child object.
     *
     * @return @c msg_envelope_t*, @c NULL if an error occurs
     */
    virtual msg_envelope_t* serialize() = 0;
};

} // msgbus
} // eis

#endif // __cplusplus

#endif // _EIS_MESSAGE_BUD_MSGENV_H
