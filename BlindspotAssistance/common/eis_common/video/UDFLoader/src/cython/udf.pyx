# cython: language_level=3, boundscheck=False
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
# distutils: language = c++
"""UDF Cython code to interface with Python
"""

# Cython imports
from libc.stdint cimport *
from cpython cimport bool
from cpython.ref cimport PyObject, Py_INCREF

# Python imports
import json
import warnings
import inspect
import importlib
import numpy as np
from distutils.util import strtobool
from util.log import configure_logging


cdef extern from "stdbool.h":
    ctypedef bint c_bool

cdef extern from "eis/utils/config.h":
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
        config_value_t* (*get)(const void*,const char*)
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
        c_bool       boolean
        config_value_object_t* object
        config_value_array_t* array

    ctypedef struct config_value_t:
        config_value_type_t type
        config_value_type_body_union_t body

    ctypedef config_value_t* (*get_config_value_fn)(const void*,const char*)

    ctypedef struct config_t:
        void* cfg
        void (*free)(void*)
        config_value_t* (*get_config_value)(const void*, const char*)

    config_value_t* config_get(const config_t* config, const char* key)
    void config_value_destroy(config_value_t* config_value)
    config_value_t* config_value_object_get(
        const config_value_t* obj, const char* key);
    config_value_t* config_value_array_get(const config_value_t* arr, int idx);
    size_t config_value_array_len(const config_value_t* arr);


cdef extern from "eis/msgbus/msg_envelope.h":
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
        c_bool owned
        size_t len
        const char* bytes

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
        c_bool boolean
        msg_envelope_blob_t* blob
        hashmap_t* object
        linkedlist_t* array

    ctypedef struct msg_envelope_elem_body_t:
        msg_envelope_data_type_t type
        msg_envelope_elem_body_body_t body

    ctypedef struct msg_envelope_t:
        content_type_t content_type
        msg_envelope_elem_body_t* blob

    ctypedef struct msg_envelope_serialized_part_t:
        owned_blob_t* shared;
        size_t len
        char* bytes

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
    int msgbus_msg_envelope_serialize(
            msg_envelope_t* env, msg_envelope_serialized_part_t** parts)
    msgbus_ret_t msgbus_msg_envelope_deserialize(
            content_type_t ct, msg_envelope_serialized_part_t* data,
            size_t num_parts, const char* name, msg_envelope_t** env)
    msgbus_ret_t msgbus_msg_envelope_serialize_parts_new(
            int num_parts, msg_envelope_serialized_part_t** parts)
    void msgbus_msg_envelope_serialize_destroy(
            msg_envelope_serialized_part_t* parts, int num_parts)


cdef extern from "eis/udf/udfretcodes.h" namespace "eis::udf":
    ctypedef enum UdfRetCode:
        UDF_OK = 0
        UDF_DROP_FRAME = 1
        UDF_FRAME_MODIFIED = 2
        UDF_ERROR = 255

# cdef extern from "eis/udf/python_udf_handle.h" namespace "eis::udf":
#     ctypedef struct PythonUdfRet:
#         UdfRetCode return_code
#         object updated_frame

cdef class ConfigurationObject:
    """Wrapper object around a config_value_t structure which is a CVT_OBJECT
    type. This object provides the interfaces for retrieving values from the
    object as if it were a Python dictionary.
    """
    cdef config_value_t* _value

    def __cinit__(self):
        """Cython constructor
        """
        self._value = NULL

    def __dealloc__(self):
        """Cython destructor
        """
        # NOTE: Not freeing _config because that should be freed by the
        # Configuration object
        if self._value != NULL:
            config_value_destroy(self._value)

    def __getitem__(self, key):
        """Get item from the configuration object

        :param key: Key for the value in the object
        :type: str
        :return: Python object for the value
        :rtype: object
        """
        cdef config_value_t* value
        bkey = bytes(key, 'utf-8')
        value = config_value_object_get(self._value, bkey)
        if value == NULL:
            raise KeyError(key)
        return cfv_to_object(value)

    @staticmethod
    cdef create(config_value_t* value):
        """Helper method to initialize a configuration object.
        """
        assert value.type == CVT_OBJECT, 'Config value must be an object'
        c = ConfigurationObject()
        c._value = value
        return c


cdef object cfv_to_object(config_value_t* value):
    """Convert a config_value_t* to a Python object.

    :param cv: Configuration value to convert
    :type: config_value_t*
    :return: Python object
    :type: object
    """
    cdef config_value_t* arr_val
    ret_val = None

    if value.type == CVT_INTEGER:
        ret_val = <int> value.body.integer
    elif value.type == CVT_FLOATING:
        ret_val = <float> value.body.floating
    elif value.type == CVT_STRING:
        ret_val = <bytes> value.body.string
        ret_val = ret_val.decode('utf-8')
    elif value.type == CVT_BOOLEAN:
        ret_val = <bool> value.body.boolean
    elif value.type == CVT_NONE:
        pass  # Do nothing here, this should return None
    elif value.type == CVT_OBJECT:
        # This is a special case for returning because we do not want the
        # config_value_t structure to be destroyed at the end
        return ConfigurationObject.create(value)
        # return None
    elif value.type == CVT_ARRAY:
        # Recursively convert all array values
        arr = []
        ret_val = []
        length = config_value_array_len(value)
        for i in range(length):
            arr_val = config_value_array_get(value, i)
            if arr_val == NULL:
                raise IndexError(f'Cannot get element: {i}')
            py_value = cfv_to_object(arr_val)
            # TODO: Veirfy that arr_val is not leaked...
            arr.append(py_value)
        ret_val = arr
    else:
        config_value_destroy(value)
        raise RuntimeError('Unknown CVT type, this should never happen')

    config_value_destroy(value)
    return ret_val


cdef public void cython_initialize(char* dev_mode, char* log_lvl):
    """Initialize the Cython Python environment
    """
    if dev_mode == NULL:
        py_dev_mode = False
    else:
        py_dev_mode = <bytes> dev_mode
        py_dev_mode = py_dev_mode.decode('utf-8')
        py_dev_mode = bool(strtobool(py_dev_mode))

    if log_lvl == NULL:
        py_log_lvl = 'ERROR'
    else:
        py_log_lvl = <bytes> log_lvl
        py_log_lvl = py_log_lvl.decode('utf-8').upper()

    configure_logging(py_log_lvl, 'UDFLoader', py_dev_mode)


cdef public object load_udf(const char* name, config_t* config) with gil:
    """Load Python UDF.

    :param name: Name of the UDF to load (can be full package path)
    :type: const char*
    :param config: Configuration for the UDF
    :type: config_t*
    :return: Python UDF object
    :type: object
    """
    cdef config_value_t* value

    py_name = <bytes> name
    py_name = py_name.decode('utf-8')

    try:
        lib = importlib.import_module(f'{py_name}')

        arg_names = inspect.getargspec(lib.Udf.__init__).args[1:]
        if len(arg_names) > 0:
            # Skipping the first argument since it is the self argument
            args = []
            for a in arg_names:
                key = bytes(a, 'utf-8')
                value = config_get(config, key)
                if value == NULL:
                    raise KeyError(f'UDF config missing key: {a}')
                py_value = cfv_to_object(value)
                args.append(py_value)
        else:
            args = []

        return lib.Udf(*args)
    except AttributeError:
        raise AttributeError(f'{py_name} module is missing the Udf class')
    except ImportError:
        raise ImportError(f'Failed to load UDF: {py_name}')
    except Exception as ex:
        print("Exception : {}".format(ex))
        raise


cdef object char_to_bytes(const char* data, int length):
    """Helper function to convert char* to byte array without stopping on a
    NULL termination.

    NOTE: This is workaround for Cython's built-in way of doing this which will
    automatically stop when it hits a NULL byte.
    """
    return <bytes> data[:length]


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


cdef object msg_envelope_to_python(msg_envelope_t* msg):
    """Convert msg_envelope_t to Python dictionary or bytes object.

    :param msg: Message envelope to convert
    :type: msg_envelope_t*
    """
    cdef msg_envelope_serialized_part_t* parts = NULL

    num_parts = msgbus_msg_envelope_serialize(msg, &parts)
    if num_parts <= 0:
        raise RuntimeError('Error serializing to Python representation')

    if num_parts > 2:
        warnings.warn('The Python library only supports 2 parts!')

    try:
        if num_parts == 2:
            parts[1].shared.owned = False
            msg.blob.body.blob.shared.owned = True

        data = None
        data = json.loads(char_to_bytes(parts[0].bytes, parts[0].len))
        return data
    finally:
        msgbus_msg_envelope_serialize_destroy(parts, num_parts)


cdef public UdfRetCode call_udf(
        object udf, object frame, PyObject*& output, msg_envelope_t* meta) except * with gil:
    """Call UDF
    """
    cdef msgbus_ret_t ret = MSG_SUCCESS
    cdef msg_envelope_elem_body_t* body
    cdef UdfRetCode ret_code = UDF_OK

    # Convert current meta-data to Python dictionary
    py_meta = msg_envelope_to_python(meta)

    # Create copy for later
    py_meta_cpy = dict(py_meta)

    pret = udf.process(frame, py_meta)

    # Verify UDF return value
    assert pret is not None, 'UDF return NoneType, must return tuple'
    assert isinstance(pret, (list, tuple,)), f'UDF returned {type(ret)}, must be tuple'
    assert len(pret) == 3, f'Return tuple must only have 3 elements'

    # Break apart tuple
    drop, updated_frame, new_meta = pret

    # Verifying data types in return tuple
    assert isinstance(drop, bool), 'First elem in return tuple must be a bool'
    if new_meta is not None:
        assert isinstance(new_meta, dict), 'Meta data must be a dict'

    if drop:
        return UDF_DROP_FRAME

    if updated_frame is not None:
        assert isinstance(updated_frame, np.ndarray), 'Frame must be NumPy'
        Py_INCREF(updated_frame)
        (&output)[0] = <PyObject*> updated_frame
        # print(output)
        ret_code = UDF_FRAME_MODIFIED

    if new_meta is not None:
        for k,v in new_meta.items():
            # Check if key is in the message envelope
            if k in py_meta_cpy:
                # Value was pre-existing in the message envelope, remove it
                # in case it changed
                remove_key = bytes(k, 'utf-8')
                ret = msgbus_msg_envelope_remove(meta, <char*> remove_key)
                assert ret == MSG_SUCCESS, 'Failed to remove element'

            body = python_to_msg_env_elem_body(v)
            if body == NULL:
                raise RuntimeError(f'Failed to convert: {k} to envelope')

            k = bytes(k, 'utf-8')
            ret = msgbus_msg_envelope_put(meta, <char*> k, body)
            if ret != msgbus_ret_t.MSG_SUCCESS:
                msgbus_msg_envelope_elem_destroy(body)
                raise RuntimeError(f'Failed to put element {k}')
            else:
                # The message envelope takes ownership of the memory allocated
                # for these elements. Setting to NULL to keep the state clean.
                body = NULL

    return ret_code
