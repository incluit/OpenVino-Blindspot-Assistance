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
"""EIS Message Bus Publisher wrapper object
"""

from .exc import MessageBusError

from .libeismsgbus cimport *
from .msg_envelope cimport python_to_msg_envelope


cdef class Publisher:
    """EIS Message Bus Publisher object
    """
    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.msgbus_ctx = NULL
        self.pub_ctx = NULL

    @staticmethod
    cdef create(void* msgbus_ctx, publisher_ctx_t* pub_ctx):
        """Helper method for initializing the publisher object. This should
        never be used outside of the MssgbusContext object.
        """
        p = Publisher()
        p.msgbus_ctx = msgbus_ctx
        p.pub_ctx = pub_ctx
        return p

    def __dealloc__(self):
        """Cython destructor
        """
        self.close()

    def close(self):
        """Close the publisher.

        This MUST be called before the program exists. If it is not your
        program may hang.

        Note that this method is not thread-safe.
        """
        if self.pub_ctx != NULL:
            msgbus_publisher_destroy(self.msgbus_ctx, self.pub_ctx)
            self.pub_ctx = NULL

    def publish(self, message):
        """Publish message on the publisher object.

        The message object passed to this method can be either a Python bytes
        object or a Python dictionary. When a bytes object is given to be
        published, then this method will construct a message envelope for a
        blob. If a dictionary is given, then a JSON message envelope will be
        constructed and published.

        :param message: Message to publish
        :type: bytes or dict
        """
        cdef msgbus_ret_t ret
        cdef msg_envelope_t* env = python_to_msg_envelope(message)
        cdef void* msgbus_ctx = self.msgbus_ctx
        cdef publisher_ctx_t* pub_ctx = self.pub_ctx

        # Check if the message envelope was successfully converted from its
        # Python representation and raise the exception that was raised in the
        # python_to_msg_envelope() function
        if env == NULL:
            raise

        with nogil:
            # Attempt to publish the message
            ret = msgbus_publisher_publish(msgbus_ctx, pub_ctx, env)
            # Destroy the published message
            msgbus_msg_envelope_destroy(env)

        if ret != msgbus_ret_t.MSG_SUCCESS:
            raise MessageBusError('Failed to publish message')
