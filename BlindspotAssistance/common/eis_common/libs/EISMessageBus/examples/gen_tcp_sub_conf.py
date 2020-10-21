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
"""Helper script for generating the configuration for the message bus to
subscribe to the pub/sub topics coming from the publisher-many C example.
"""

import os
import json
import argparse


# Parse command line arguments
ap = argparse.ArgumentParser()
ap.add_argument('pub_config', help='Publisher many JSON configuration')
ap.add_argument('output', help='Output JSON file')
ap.add_argument('num_publishers', type=int, help='Number of publishers')
args = ap.parse_args()

# Get the publisher config
assert os.path.exists(args.pub_config), f'{args.pub_config} does not exist'

with open(args.pub_config, 'r') as f:
    pub_config = json.load(f)

assert 'zmq_tcp_publish' in pub_config, 'Malformatted publisher config'

host = pub_config['zmq_tcp_publish']['host']
port = pub_config['zmq_tcp_publish']['port']

config = {'type': 'zmq_tcp'}

print('[INFO] Creating configuration')
for i in range(args.num_publishers):
    config[f'pub-{i}'] = {'host': host, 'port': port}


print(f'[INFO] Writing configuration to {args.output}')
with open(args.output, 'w') as f:
    json.dump(config, f, indent=4)

print('[INFO] Done.')
