# Copyright (c) 2020 Intel Corporation.

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

import os
import logging
import cv2
import numpy as np
from time import time

from openvino.inference_engine import IECore


class DisplayInfo:
    """DisplayInfo class
    """
    def __init__(self, info, priority):
        """DisplayInfo class

        :param info: Information string to be displayed with the frame
        :type info: str
        :param priority: Priority of the information [0: low, 1: medium,
                         2: high]
        :type priority: int
        """
        self.info = info
        self.priority = priority

    def __repr__(self):
        return '<DisplayInfo(info={0}, priority={1})>'.format(
                self.info, self.priority)


class Udf:

    def __init__(self, model_xml, model_bin, device, labels_file_path):
        # Constructor of Sample classification algorithm

        self.log = logging.getLogger('SAMPLE CLASSIFICATION')
        # Assert all input parameters exist
        assert os.path.exists(model_xml), \
            'Classification model xml file missing: {}'.format(model_xml)
        assert os.path.exists(model_bin), \
            'Classification model bin file missing: {}'.format(model_bin)
        assert os.path.exists(labels_file_path), \
            'Labels mapping file missing: {}'.format(labels_file_path)

        # Load labels file associated with the model
        with open(labels_file_path, 'r') as f:
            self.labels_map = [x.split(sep=' ', maxsplit=1)[-1].strip() for x
                               in f]

        # Load OpenVINO model
        self.ie = IECore()
        self.net = self.ie.read_network(model=model_xml, weights=model_bin)
        self.log.debug("Loading network files:\n\t{}\n\t{}".format(model_xml,
                                                                   model_bin))
        if device.upper() == "CPU":
            supported_layers = self.ie.query_network(self.net, device_name="CPU")
            not_supported_layers = [l for l in self.net.layers.keys() if l not
                                    in supported_layers]
            if len(not_supported_layers) != 0:
                self.log.debug('ERROR: Following layers are not supported by \
                                {}'.format(not_supported_layer))

        assert len(self.net.input_info.keys()) == 1, \
            'Sample supports only single input topologies'
        assert len(self.net.outputs) == 1, \
            'Sample supports only single output topologies'

        self.input_blob = next(iter(self.net.input_info))
        self.output_blob = next(iter(self.net.outputs))
        self.net.batch_size = 1  # change to enable batch loading
        self.exec_net = self.ie.load_network(network=self.net,
            device_name=device.upper())

    # Main classification algorithm
    def process(self, frame, metadata):
        # Read and preprocess input images
        n, c, h, w = self.net.inputs[self.input_blob].shape
        images = np.ndarray(shape=(n, c, h, w))
        for i in range(n):
            if frame.shape[:-1] != (h, w):
                self.log.debug('Image is resized from {} to {}'.format(
                    frame.shape[:-1], (w, h)))
                frame = cv2.resize(frame, (w, h))
            # Change layout from HWC to CHW
            frame = frame.transpose((2, 0, 1))
            images[i] = frame
        self.log.debug('Batch size is {}'.format(n))

        # Start sync inference
        infer_time = []
        t0 = time()
        res = self.exec_net.infer(inputs={self.input_blob: images})
        infer_time.append((time() - t0)*1000)
        self.log.info('Average running time of one iteration: {} ms'.
                      format(np.average(np.asarray(infer_time))))

        # Display information for visualizer
        d_info = []

        # Processing output blob
        self.log.debug('Processing output blob')
        res = res[self.output_blob]
        self.log.info("Top 5 results :")

        for i, probs in enumerate(res):
            probs = np.squeeze(probs)
            top_ind = np.argsort(probs)[-5:][::-1]
            for id in top_ind:
                det_label = self.labels_map[id] \
                    if self.labels_map else '#{}'.format(id)
                self.log.info('prob: {:.7f}, label: {}'.format(probs[id],
                              det_label))
                # LOW priority information string to be displayed with
                # frame
                disp_info = DisplayInfo('prob: {:.7f}, label: {} \
                            '.format(probs[id], det_label), 0)
                d_info.append(disp_info)

        display_info = []
        for d in d_info:
            display_info.append({
                'info': d.info,
                'priority': d.priority
            })
        metadata["display_info"] = display_info
        self.log.debug("metadata: {} added to classifier output queue".
                       format(metadata))
        return False, None, metadata
