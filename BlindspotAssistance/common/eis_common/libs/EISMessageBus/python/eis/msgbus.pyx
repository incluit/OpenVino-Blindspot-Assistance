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
"""EIS Message Bus Python mapping
"""

# Python imports
from .exc import *

# Cython imports
from .libeismsgbus cimport *
from .publisher cimport Publisher
from .receive_context cimport ReceiveContext
from .service cimport Service
from .service_requester cimport ServiceRequester
from cpython cimport bool


# Forward declaration of the get_array_value method
cdef extern config_value_t* get_array_value(const void* obj, int idx) with gil


cdef config_value_t* py_to_cvt(object conf_val):
    """Helper method for converting a Python object to a config_value_t.
    """
    cdef config_value_t* value
    if isinstance(conf_val, bool):
        value = config_value_new_boolean(<bint> conf_val)
    elif isinstance(conf_val, str):
        conf_val = bytes(conf_val, 'utf-8')
        value = config_value_new_string(conf_val)
    elif isinstance(conf_val, int):
        value = config_value_new_integer(<int> conf_val)
    elif isinstance(conf_val, dict):
        value = config_value_new_object(
                <void*> conf_val, get_config_value, NULL)
    elif isinstance(conf_val, (tuple, list,)):
        value = config_value_new_array(
                <void*> conf_val, len(conf_val), get_array_value, NULL)
    else:
        # Unknown type for configuration value
        return NULL

    return value


cdef config_value_t* get_array_value(const void* obj, int idx) with gil:
    """Get the value in the array at the given index.
    """
    cdef config_value_t* value
    arr = <object> obj

    try:
        value = py_to_cvt(arr[idx])
        return value
    except IndexError:
        return NULL


cdef config_value_t* get_config_value(
        const void* obj, const char* key) with gil:
    """Get configuration value implementation for a python dictionary object.
    """
    conf = <object> obj

    try:
        conf_val = conf[key.decode('utf-8')]
        return py_to_cvt(conf_val)
    except KeyError:
        return NULL


cdef void free_conf(void* conf) nogil:
    """Free method for configuration object. Does nothing since the config
    object is a Python dictionary, and therefore garbage collected by the
    Python interpreter.
    """
    pass  # Does nothing since Python


cdef class MsgbusContext:
    """EIS Message Bus context object
    """
    cdef void* context

    def __init__(self, config):
        """Constructor

        :param config: Configuration object
        :type: dict
        """
        cdef config_t* conf
        conf = config_new(<void*> config, free_conf, get_config_value)
        self.context =  msgbus_initialize(conf)
        if self.context == NULL:
            raise MessageBusError('Initialization failed')

    def __cinit__(self, *args, **kwargs):
        """Basic C init
        """
        self.context = NULL

    def __dealloc__(self):
        """Deconstructor
        """
        if self.context != NULL:
            msgbus_destroy(self.context)

    def new_publisher(self, topic):
        """Create a new publisher object.

        :param topic: Publisher's topic
        :type: str
        :return: Publisher object
        :rtype: Publisher
        """
        cdef msgbus_ret_t ret
        cdef publisher_ctx_t* pub_ctx

        btopic = bytes(topic, 'utf-8')
        ret = msgbus_publisher_new(self.context, btopic, &pub_ctx)
        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Failed to create publisher context')

        return Publisher.create(self.context, pub_ctx)

    def new_subscriber(self, topic):
        """Create a new subscriber object.

        :param topic: Topic to subscribe to
        :type: str
        :return: Subscriber object
        :rtype: Subscriber
        """
        cdef msgbus_ret_t ret
        cdef recv_ctx_t* sub_ctx

        btopic = bytes(topic, 'utf-8')
        ret = msgbus_subscriber_new(self.context, btopic, NULL, &sub_ctx)
        if ret == msgbus_ret_t.MSG_ERR_AUTH_FAILED:
            raise MessageBusError('Authentication failed')
        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Failed to subscribe to topic')

        return ReceiveContext.create(self.context, sub_ctx)

    def new_service(self, service_name):
        """Create a new service context for receiving requests.

        Note that this method will expect to find the configuration attributes
        needed to communicate with the specified service in the configuration
        given to the constructor.

        :param service_name: Name of the service
        :type: str
        :return: Service object
        :rtype: Service
        """
        cdef msgbus_ret_t ret
        cdef recv_ctx_t* serv_ctx

        bservice_name = bytes(service_name, 'utf-8')
        ret = msgbus_service_new(self.context, bservice_name, NULL, &serv_ctx)
        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Failed to subscribe to topic')

        return Service.create(self.context, serv_ctx)

    def get_service(self, service_name):
        """Create a new service context for issuing requests to a service.

        Note that this method will expect to find the configuration attributes
        needed to communicate with the specified service in the configuration
        given to the constructor.

        :param service_name: Name of the service
        :type: str
        :return: Service object
        :rtype: Service
        """
        cdef msgbus_ret_t ret
        cdef recv_ctx_t* serv_ctx

        bservice_name = bytes(service_name, 'utf-8')
        ret = msgbus_service_get(self.context, bservice_name, NULL, &serv_ctx)
        if ret == msgbus_ret_t.MSG_ERR_AUTH_FAILED:
            raise MessageBusError('Authentication failed')
        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Failed to subscribe to topic')

        return ServiceRequester.create(self.context, serv_ctx)
