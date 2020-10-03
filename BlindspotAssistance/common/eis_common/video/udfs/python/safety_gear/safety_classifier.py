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


import os
import logging
import cv2
import numpy as np
import json
import threading
from openvino.inference_engine import IECore
from distutils.util import strtobool
import time
import sys

"""
Labels based on trained model used in this sample app

safety_helmet = 1
safety_jacket = 2
safe = 3
violation = 4
"""


class Udf:
    """Classifier object
    """

    def __init__(self, model_xml, model_bin, device):
        """Constructor of Classifier class

        :param classifier_config: Configuration object for the classifier
        :type classifier_config: dict
        :param input_queue: input queue for classifier
        :type input_queue: queue
        :param output_queue: output queue of classifier
        :type output_queue: queue
        :return: Classification object
        :rtype: Object
        """
        self.log = logging.getLogger('WORKER_SAFETY_DETECTION')
        self.model_xml = model_xml
        self.model_bin = model_bin
        self.device = device

        assert os.path.exists(self.model_xml), \
            'Model xml file missing: {}'.format(self.model_xml)
        assert os.path.exists(self.model_bin), \
            'Model bin file missing: {}'.format(self.model_bin)

        # Load OpenVINO model
        self.ie = IECore()
        self.neuralNet = self.ie.read_network(
            model=self.model_xml, weights=self.model_bin)

        if self.neuralNet is not None:
            self.inputBlob = next(iter(self.neuralNet.input_info))
            self.outputBlob = next(iter(self.neuralNet.outputs))
            self.neuralNet.batch_size = 1
            self.executionNet = self.ie.load_network(network=self.neuralNet,
                device_name=self.device.upper())

        self.profiling = bool(strtobool(os.environ['PROFILING_MODE']))

    # Main classification algorithm
    def process(self, frame, metadata):
        """Reads the image frame from input queue for classifier
        and classifies against the specified reference image.
        """
        if self.profiling is True:
            metadata['ts_va_classify_entry'] = time.time()*1000

        defects = []
        d_info = []

        n, c, h, w = self.neuralNet.inputs[self.inputBlob].shape
        cur_request_id = 0
        labels_map = None

        inf_start = time.time()
        initial_h = frame.shape[0]
        initial_w = frame.shape[1]

        in_frame = cv2.resize(frame, (w, h))
        # Change data layout from HWC to CHW
        in_frame = in_frame.transpose((2, 0, 1))
        in_frame = in_frame.reshape((n, c, h, w))
        self.executionNet.start_async(request_id=cur_request_id, inputs={
            self.inputBlob: in_frame})

        if self.executionNet.requests[cur_request_id].wait(-1) == 0:
            inf_end = time.time()
            det_time = inf_end - inf_start
            fps = str("%.2f" % (1/det_time))

            # Parse detection results of the current request
            res = self.executionNet.requests[cur_request_id].outputs[
                self.outputBlob]

            for obj in res[0][0]:
                # obj[1] representing the category of the object detection
                # Draw only objects when probability more than specified
                # threshold represented by obj[2]

                if obj[1] == 1 and obj[2] > 0.57:
                    xmin = int(obj[3] * initial_w)
                    ymin = int(obj[4] * initial_h)
                    xmax = int(obj[5] * initial_w)
                    ymax = int(obj[6] * initial_h)
                    class_id = int(obj[1])
                    prob = obj[2]

                    # defect type returned as string, no user_labels mapping
                    # required
                    defects.append({'type': 'safety_helmet',
                                    'tl': (xmin, ymin),
                                    'br': (xmax, ymax)})

                if obj[1] == 2 and obj[2] > 0.525:
                    xmin = int(obj[3] * initial_w)
                    ymin = int(obj[4] * initial_h)
                    xmax = int(obj[5] * initial_w)
                    ymax = int(obj[6] * initial_h)
                    class_id = int(obj[1])
                    prob = obj[2]

                    # defect type returned as string, no user_labels mapping
                    # required
                    defects.append({'type': 'safety_jacket',
                                    'tl': (xmin, ymin),
                                    'br': (xmax, ymax)})

                if obj[1] == 3 and obj[2] > 0.3:
                    xmin = int(obj[3] * initial_w)
                    ymin = int(obj[4] * initial_h)
                    xmax = int(obj[5] * initial_w)
                    ymax = int(obj[6] * initial_h)
                    class_id = int(obj[1])
                    prob = obj[2]

                    # defect type returned as string, no user_labels mapping
                    # required
                    defects.append({'type': 'safe', 'tl': (xmin, ymin),
                                    'br': (xmax, ymax)})

                if obj[1] == 4 and obj[2] > 0.35:
                    xmin = int(obj[3] * initial_w)
                    ymin = int(obj[4] * initial_h)
                    xmax = int(obj[5] * initial_w)
                    ymax = int(obj[6] * initial_h)
                    class_id = int(obj[1])
                    prob = obj[2]

                    # defect type returned as string, no user_labels mapping
                    # required
                    defects.append({'type': 'violation', 'tl': (xmin, ymin),
                                    'br': (xmax, ymax)})

        metadata["defects"] = defects

        if self.profiling is True:
            metadata['ts_va_classify_exit'] = time.time()*1000
        return False, None, metadata
