# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""
etcd_client module is a etc wrapper to read any keys and watch on a directory
in etcd distributed key store
"""

import etcd3
import logging
import json
import os
import socket
import time


class EtcdCli:

    def __init__(self, config):
        """ constructor which creates an EtcdCli instance, checks for
        the etcd service port availability
        :param config: config of type Dict with certFile, keyFile
        and trustFile"""

        self.logger = logging.getLogger(__name__)

        self.etcd_key_prefix = None
        hostname = "localhost"

        # This change will be moved to an argument to the function in 2.3
        # This is done now for backward compatibility
        etcd_host = os.getenv("ETCD_HOST")
        if etcd_host is not None and etcd_host != "":
            hostname = etcd_host

        try:
            port = os.getenv("ETCD_CLIENT_PORT", "2379")
        except Exception as e:
            log.debug("Using default etcd port")

        etcd_endpoint = os.getenv("ETCD_ENDPOINT")
        if etcd_endpoint is not None and etcd_endpoint != "":
            hostname = etcd_endpoint.split(':')[0]
            port = etcd_endpoint.split(':')[1]

        key_prefix = os.getenv("ETCD_PREFIX")
        if key_prefix is not None and key_prefix != "":
            self.etcd_key_prefix = key_prefix

        if not self.check_port_availability(hostname, port):
            raise Exception("etcd service port {} is not up!".format(port))

        try:
            if config["trustFile"] == "" and config["keyFile"] == "" \
               and config["certFile"] == "":
                self.etcd = etcd3.client(host=hostname, port=port)
            else:
                self.etcd = etcd3.client(host=hostname, port=port,
                                         ca_cert=config["trustFile"],
                                         cert_key=config["keyFile"],
                                         cert_cert=config["certFile"])
        except Exception as e:
            self.logger.error("Exception raised when creating etcd \
                client instance with error:{}".format(e))
            raise e
        self.callback = None
        self._setEnv()

    def _setEnv(self):
        """ _setEnv is a local function to set global env """
        try:
            global_env_prfix = "/GlobalEnv/"
            if self.etcd_key_prefix is not None:
                global_env_prfix = self.etcd_key_prefix + "/GlobalEnv/"
            value = self.etcd.get(global_env_prfix)
            if value[0] is not None:
                jsonConfig = json.loads(value[0].decode('utf-8'))
                for key in jsonConfig.keys():
                    os.environ[key] = jsonConfig[key]
            else:
                raise TypeError("config manager key {} must be set as \
                    a prerequisite ...".format(global_env_prfix))
        except Exception as e:
            self.logger.error("Exception raised in _setEnv\
                with error:{}".format(e))
            raise e

    # Duplicated this function from utils to avoid the python eis
    # config manager dependency on utils
    def check_port_availability(self, hostname, port):
        """Verifies port availability on hostname for accepting connection

        :param hostname: hostname of the machine
        :type hostname: str
        :param port: port
        :type port: str
        :return: portUp (whether port is up or not)
        :rtype: Boolean
        """
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.logger.debug("Attempting to connect to {}:{}\
            ".format(hostname, port))
        num_retries = 1000
        retry_count = 0
        port_up = False
        while(retry_count < num_retries):
            if(sock.connect_ex((hostname, int(port)))):
                self.logger.debug("{} port is up on {}".format(port, hostname))
                port_up = True
                break
            retry_count += 1
            time.sleep(0.1)
        return port_up

    def GetConfig(self, key):
        """ GetConfig gets the value of a key from Etcd
        :param key: key to be queried on, form etcd
        :type: string
        :return: values returned from etcd based on key
        :rtype: string"""
        try:
            if self.etcd_key_prefix is not None:
                key = self.etcd_key_prefix + key
            value = self.etcd.get(key)
        except Exception as e:
            self.logger.error("Exception raised in GetConfig\
                with error:{}".format(e))
            raise e
        if value[0] is not None:
            return value[0].decode('utf-8')
        return value[0]

    def PutConfig(self, key, value):
        """ PutConfig to save a value to Etcd
        :param key: keyin etcd to set
        :type: string
        :param value: value to set key to
        :type: string
        """
        try:
            if self.etcd_key_prefix is not None:
                key = self.etcd_key_prefix + key
            self.etcd.put(key, value)
        except Exception as e:
            self.logger.error("Exception raised in PutConfig \
                with error:{}".format(e))
            raise e

    def onChangeCallback(self, event):
        key = event.events[0].key
        value = event.events[0].value
        self.callback(key.decode('utf-8'), value.decode('utf-8'))

    def RegisterDirWatch(self, key, callback):
        """ RegisterDirWatch registers to a callback and keeps a watch on the
        prefix of a specified key
        :param key: prefix of a key to keep a watch on, in etcd
        :type string
        :callback : callback function
        :return None"""

        self.callback = callback
        try:
            if self.etcd_key_prefix is not None:
                key = self.etcd_key_prefix + key
            self.etcd.add_watch_prefix_callback(key, self.onChangeCallback)
        except Exception as e:
            self.logger.error("Exception raised RegisterDirWatch\
                with error:{}".format(e))
            raise e

    def RegisterKeyWatch(self, key, callback):
        """ RegisterKeyWatch registers to a callback and keeps a watch
        on a specified key
        :param key: key to keep a watch on, in etcd
        :type string
        :callback : callback function
        :return None"""

        self.callback = callback
        try:
            if self.etcd_key_prefix is not None:
                key = self.etcd_key_prefix + key
            self.etcd.add_watch_callback(key, self.onChangeCallback)
        except Exception as e:
            self.logger.error("Exception raised in RegisterKeyWatch\
                 with error:{}".format(e))
            raise e
