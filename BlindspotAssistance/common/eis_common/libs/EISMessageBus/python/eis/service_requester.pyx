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
"""EIS Message Bus service wrapper object to issue requests
"""

# Python imports
from .exc import MessageBusError

# Cython imports
from .libeismsgbus cimport *
from .receive_context cimport ReceiveContext
from .msg_envelope cimport python_to_msg_envelope


cdef class ServiceRequester(ReceiveContext):
    """EIS Message Bus service wrapper object to issue requests
    """
    @staticmethod
    cdef create(void* msgbus_ctx, recv_ctx_t* recv_ctx):
        """Helper method for initializing the receive context object. This
        should never be used outside of the MssgbusContext object.
        """
        r = ServiceRequester()
        r.msgbus_ctx = msgbus_ctx
        r.recv_ctx = recv_ctx
        return r

    def request(self, request):
        """Issue a request to the service.

        :param request: Request to issue to the service
        :type: bytes or dict
        """
        cdef msgbus_ret_t ret
        cdef msg_envelope_t* msg = python_to_msg_envelope(request)

        # Check if the message envelope was successfully converted from its
        # Python representation and raise the exception that was raised in the
        # python_to_msg_envelope() function
        if msg == NULL:
            raise

        ret = msgbus_request(self.msgbus_ctx, self.recv_ctx, msg)
        if ret != msgbus_ret_t.MSG_SUCCESS:
            msgbus_msg_envelope_destroy(msg)
            raise MessageBusError('Failed to issue request to the service')

        msgbus_msg_envelope_destroy(msg)
