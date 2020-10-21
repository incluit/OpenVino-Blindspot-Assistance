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
"""EIS Message Bus echo service Python example.
"""

import json
import argparse
import eis.msgbus as mb

# Argument parsing
ap = argparse.ArgumentParser()
ap.add_argument('config', help='JSON configuration')
ap.add_argument('-s', '--service-name', dest='service_name',
                default='echo_service', help='Service name')
args = ap.parse_args()

msgbus = None
service = None

with open(args.config, 'r') as f:
    config = json.load(f)

try:
    print('[INFO] Initializing message bus context')
    msgbus = mb.MsgbusContext(config)

    print(f'[INFO] Initializing service for topic \'{args.service_name}\'')
    service = msgbus.new_service(args.service_name)

    print('[INFO] Running...')
    while True:
        request = service.recv()
        print(f'[INFO] Received request: {request.get_meta_data()}')
        service.response(request.get_meta_data())
except KeyboardInterrupt:
    print('[INFO] Quitting...')
finally:
    if service is not None:
        service.close()
