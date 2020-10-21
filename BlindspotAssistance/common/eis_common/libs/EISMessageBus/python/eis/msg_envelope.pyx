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
"""EIS Message Envelope utility functions
"""

# Python imports
import json
import warnings
from .exc import MessageBusError

# Cython imports
from .libeismsgbus cimport *
from cpython cimport bool, Py_INCREF, Py_DECREF
from libc.stdint cimport int64_t


class MsgEnvelope:
    """EIS Message Envelope
    """
    def __init__(self, meta_data=None, blob=None, name=None):
        """Constructor

        :param str name: Topic string or service name
        :param dict meta_data: (key, value) data in the
                                message envelope, can be None
        :param bytes blob: Blob data in the message, can be None
        """
        self.name = name
        self.meta_data = meta_data
        self.blob = blob

    def get_name(self):
        """Get the topic string or service name in the message envelope.

        .. note:: This will only be assigned if the MsgEnvelope was received
            over the message bus.

        :return: Topic string or service name, will be None if not set
        :rtype: Union[None, str]
        """
        return self.name

    def get_blob(self):
        """Get the blob data in the message envelope.

        :return: Blob data in the message if any exists
        :rtype: Union[None, bytes]
        """
        return self.blob

    def get_meta_data(self):
        """Get the meta-data in the message envelope if any exists.

        :return: Meta-data in the message envelope
        :rtype: Union[None, dict]
        """
        return self.meta_data

    def __getitem__(self, key):
        """Override Python __getitem__() method to make the MsgEnvelope class
        behave like a tuple, dict, or bytes value to maintain backwards
        compatibility through the API.
        """
        tup = (self.meta_data, self.blob,)
        return tup[key]

    def __setitem__(self, key, value):
        """Override Python __setitem__() method to make the MsgEnvelope class
        assign values in the meta_data dictionary.
        """
        if self.meta_data is None:
            self.meta_data = {}
        self.meta_data[key] = value

    def __bytes__(self):
        """Overriden bytes method.
        """
        if self.meta_data is not None:
            # Raising the same error that Python raises for calling bytes on a
            # dict (this case also applies to when the object is tuple of
            # (dict, bytes))
            raise TypeError(
                    '\'dict\' object cannot be interpreted as an integer')
        else:
            return self.blob

    def __eq__(self, other):
        """Overridden equals method.
        """
        if isinstance(other, MsgEnvelope):
            return self.meta_data == other.meta_data and \
                self.blob == other.blob
        elif self.meta_data is not None and self.blob is not None:
            # Assume that other is a tuple and is being compared to this msg
            return (self.meta_data, self.blob,) == other
        elif self.meta_data is not None:
            # Assume that other is a dict and is being compared to meta_data
            return self.meta_data == other
        else:
            # Assume that other is a bytes object
            return self.blob == other


cdef void free_python_blob(void* vhint) nogil:
    """Method for freeing a Python blob by decreasing the number of references
    on it in the Python interpreter.

    This method is called once the message bus is done with the data.
    """
    # Explicitly acquiring the GIL here, must be done this way, otherwise
    # this method cannot be called from the underlying C library
    with gil:
        # Decrement the reference count and delete the Python object so it can
        # be freed by the Python garbage collector.
        obj = <object> vhint
        Py_DECREF(obj)
        del obj


cdef void put_bytes_helper(msg_envelope_t* env, data) except *:
    """Helper function to serialize a Python bytes object to a blob object.
    """
    cdef msgbus_ret_t ret
    cdef msg_envelope_elem_body_t* body

    body = msgbus_msg_envelope_new_blob(<char*> data, len(data))
    if body == NULL:
        raise MessageBusError('Failed to initialize new blob')

    # Put element into the message envelope
    ret = msgbus_msg_envelope_put(env, NULL, body)
    if ret != msgbus_ret_t.MSG_SUCCESS:
        msgbus_msg_envelope_elem_destroy(body)
        raise MessageBusError('Failed to put blob in message envelope')

    # Increment the reference count on the underlying Python object for the
    # blob data being published over the message bus. This will keep the data
    # from being garbage collected by the interpreter.
    Py_INCREF(data)
    env.blob.body.blob.shared.ptr = <void*> data
    env.blob.body.blob.shared.free = free_python_blob


cdef msg_envelope_elem_body_t* python_to_msg_env_elem_body(data):
    """Helper function to recursively convert a python object to
    msg_envelope_elem_body_t.
    """
    cdef msg_envelope_elem_body_t* elem = NULL
    cdef msg_envelope_elem_body_t* subelem = NULL
    cdef msgbus_ret_t ret = MSG_SUCCESS

    if isinstance(data, str):
        bv = bytes(data, 'utf-8')
        elem = msgbus_msg_envelope_new_string(bv)
    elif isinstance(data, int):
        elem = msgbus_msg_envelope_new_integer(<int64_t> data)
    elif isinstance(data, float):
        elem = msgbus_msg_envelope_new_floating(<double> data)
    elif isinstance(data, bool):
        elem = msgbus_msg_envelope_new_bool(<bint> data)
    elif isinstance(data, dict):
        elem = msgbus_msg_envelope_new_object()
        for k, v in data.items():
            # Convert the python element to a msg envelope element
            subelem = python_to_msg_env_elem_body(v)
            if subelem == NULL:
                msgbus_msg_envelope_elem_destroy(elem)
                return NULL

            # Add the element to the nested object
            k = bytes(k, 'utf-8')
            ret = msgbus_msg_envelope_elem_object_put(elem, <char*> k, subelem)
            if ret != MSG_SUCCESS:
                msgbus_msg_envelope_elem_destroy(subelem)
                msgbus_msg_envelope_elem_destroy(elem)
                return NULL
    elif isinstance(data, (list, tuple,)):
        elem = msgbus_msg_envelope_new_array()
        for v in data:
            # Convert the python element to a msg envelope element
            subelem = python_to_msg_env_elem_body(v)
            if subelem == NULL:
                msgbus_msg_envelope_elem_destroy(elem)
                return NULL

            # Add the element to the array
            ret = msgbus_msg_envelope_elem_array_add(elem, subelem)
            if ret != MSG_SUCCESS:
                msgbus_msg_envelope_elem_destroy(subelem)
                msgbus_msg_envelope_elem_destroy(elem)
                return NULL
    elif data is None:
        elem = msgbus_msg_envelope_new_none()

    return elem


cdef msg_envelope_t* python_to_msg_envelope(data) except *:
    """Helper function to create a msg_envelope_t from a Python bytes or
    dictionary object.

    :param data: Data for the message envelope
    :type: bytes or dict
    :return: Message envelope
    :rtype: msg_envelope_t
    """
    cdef msgbus_ret_t ret
    cdef msg_envelope_elem_body_t* body
    cdef msg_envelope_t* env
    cdef content_type_t ct
    cdef char* key = NULL

    binary = None
    kv_data = None

    if isinstance(data, bytes):
        ct = content_type_t.CT_BLOB
        binary = data
    elif isinstance(data, dict):
        ct = content_type_t.CT_JSON
        kv_data = data
    elif isinstance(data, (list, tuple,)):
        ct = content_type_t.CT_JSON
        if len(data) > 2:
            raise MessageBusError('List can only be 2 elements for a msg')

        if isinstance(data[0], bytes):
            if not isinstance(data[1], dict):
                raise MessageBusError('Second element must be dict')

            binary = data[0]
            kv_data = data[1]
        elif isinstance(data[0], dict):
            if not isinstance(data[1], bytes):
                raise MessageBusError('Second element must be bytes')

            binary = data[1]
            kv_data = data[0]
        else:
            raise MessageBusError(
                    f'Unknown data type: {type(data)}, must be bytes or dict')
    else:
        raise MessageBusError(
                'Unable to create msg envelope from type: {}'.format(
                    type(data)))

    # Initialize message envelope object
    env = msgbus_msg_envelope_new(ct)

    if env == NULL:
        raise MessageBusError('Failed to initialize message envelope')

    if ct == content_type_t.CT_BLOB:
        try:
            put_bytes_helper(env, data)
        except MessageBusError:
            msgbus_msg_envelope_destroy(env)
            raise  # Re-raise
    else:
        if binary is not None:
            try:
                put_bytes_helper(env, binary)
            except:
                msgbus_msg_envelope_destroy(env)
                raise  # Re-raise

        for k, v in kv_data.items():
            body = python_to_msg_env_elem_body(v)
            if body == NULL:
                raise MessageBusError(f'Failed to convert: {k} to envelope')

            k = bytes(k, 'utf-8')
            ret = msgbus_msg_envelope_put(env, <char*> k, body)
            if ret != msgbus_ret_t.MSG_SUCCESS:
                msgbus_msg_envelope_elem_destroy(body)
                msgbus_msg_envelope_destroy(env)
                raise MessageBusError(f'Failed to put element {k}')
            else:
                # The message envelope takes ownership of the memory allocated
                # for these elements. Setting to NULL to keep the state clean.
                body = NULL
                key = NULL

    return env


cdef object char_to_bytes(const char* data, int length):
    """Helper function to convert char* to byte array without stopping on a
    NULL termination.

    NOTE: This is workaround for Cython's built-in way of doing this which will
    automatically stop when it hits a NULL byte.
    """
    return <bytes> data[:length]


cdef object msg_envelope_to_python(msg_envelope_t* msg):
    """Convert msg_envelope_t to Python dictionary or bytes object.

    :param msg: Message envelope to convert
    :type: msg_envelope_t*
    """
    cdef msg_envelope_serialized_part_t* parts = NULL

    num_parts = msgbus_msg_envelope_serialize(msg, &parts)
    if num_parts <= 0:
        raise MessageBusError('Error serializing to Python representation')

    if num_parts > 2:
        warnings.warn('The Python library only supports 2 parts!')

    try:
        data = None
        msg_meta_data = None
        msg_blob = None
        msg_name = None
        if msg.name.decode() is not None:
            msg_name = msg.name.decode()
        if msg.content_type == content_type_t.CT_JSON:
            msg_meta_data = json.loads(char_to_bytes(parts[0].bytes, parts[0].len))
            if num_parts > 1:
                msg_blob = char_to_bytes(parts[1].bytes, parts[1].len)
        elif msg.content_type == content_type_t.CT_BLOB:
            msg_blob = char_to_bytes(parts[0].bytes, parts[0].len)
        else:
            raise MessageBusError('Unknown content type')
        data = MsgEnvelope(name=msg_name, meta_data=msg_meta_data, blob=msg_blob)
        return data
    finally:
        msgbus_msg_envelope_serialize_destroy(parts, num_parts)
