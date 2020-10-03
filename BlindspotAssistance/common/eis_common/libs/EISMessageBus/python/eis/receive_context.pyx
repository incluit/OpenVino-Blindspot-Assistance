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
"""EIS Message Bus Python receive context wrapper
"""

# Python imports
from .exc import *

# Cython imports
from .libeismsgbus cimport *
from .msg_envelope cimport msg_envelope_to_python


cdef class ReceiveContext:
    """EIS Message Bus receive context wrapper object
    """
    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.msgbus_ctx = NULL
        self.recv_ctx = NULL

    @staticmethod
    cdef create(void* msgbus_ctx, recv_ctx_t* recv_ctx):
        """Helper method for initializing the receive context object. This
        should never be used outside of the MssgbusContext object.
        """
        r = ReceiveContext()
        r.msgbus_ctx = msgbus_ctx
        r.recv_ctx = recv_ctx
        return r

    def close(self):
        """Close the receive context.

        This MUST be called before the program exists. If it is not your
        program may hang.

        Note that this method is not thread-safe.
        """
        if self.recv_ctx != NULL:
            msgbus_recv_ctx_destroy(self.msgbus_ctx, self.recv_ctx)
            self.recv_ctx = NULL

    def recv(self, blocking=True, timeout=-1):
        """Receive a message on the message bus for the given receive context.
        Note that the receive context can be a Subscriber or a Service object.

        Additionally, if the timeout is set to -1, then this method will
        operate based on whether blocking is set to True or False (i.e. block
        or do not block). However, it the timeout is set to anything > -1,
        then this method will ignore whether blocking is set and use operate
        on a timeout.

        :param blocking: (Optional) Block until message received, or return
            immediately.
        :type: bool
        :param timeout: (Optional) Timeout in milliseconds to receive a message
        :type: int
        :return: Received message
        :rtype: dict or bytes
        """
        cdef msgbus_ret_t ret
        cdef msg_envelope_t* msg = NULL
        cdef void* ctx = self.msgbus_ctx
        cdef recv_ctx_t* recv_ctx = self.recv_ctx
        cdef int c_timeout = <int> timeout
        data = None

        if timeout > -1:
            with nogil:
                ret = msgbus_recv_timedwait(
                        ctx, recv_ctx, c_timeout, &msg)
            if ret == msgbus_ret_t.MSG_RECV_NO_MESSAGE:
                raise ReceiveTimeout('Timeout')
        elif blocking:
            with nogil:
                ret = msgbus_recv_wait(ctx, recv_ctx, &msg)
        else:
            with nogil:
                ret = msgbus_recv_nowait(ctx, recv_ctx, &msg)

        if ret == msgbus_ret_t.MSG_ERR_EINTR:
            return None
        if ret == msgbus_ret_t.MSG_ERR_DISCONNECTED:
            raise Disconnected('Receive context has been disconnected')
        if ret == msgbus_ret_t.MSG_ERR_AUTH_FAILED:
            raise MessageBusAuthenticationFailed('Authentication Failed')
        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Receive failed')

        try:
            data = msg_envelope_to_python(msg)
            msgbus_msg_envelope_destroy(msg)
            return data
        except:
            msgbus_msg_envelope_destroy(msg)
            raise  # Re-raise exception
