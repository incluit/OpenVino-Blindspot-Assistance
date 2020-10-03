# Copyright (c) 2020 Intel Corporation.
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
"""Sample EIS ONNX User Defined Function (UDF)
"""
import os
import logging
import cv2
import numpy as np
from time import time
import onnxruntime as rt
from azureml.core.model import Model
from azureml.core import Workspace
from azureml.core.authentication import ServicePrincipalAuthentication


def softmax(x):
    """Calculate softmax value
    """
    x = x.reshape(-1)
    e_x = np.exp(x - np.max(x))
    return e_x / e_x.sum(axis=0)


def postprocess(result):
    """Post process inference results.
    """
    return softmax(np.array(result)).tolist()


class Udf:
    """Sample ONNX UDF.
    """
    def __init__(self, aml_ws, aml_subscription_id,
                 model_name, download_model):
        """Constructor.

        :param str aml_ws: Azure ML workspace name
        :param str aml_subscription_id: AzureML subscription ID
        :param str model_name: Name of the AzureML model to download, or the
            location of the ONNX model file to load
        :param bool download_model: Flag to download model from AzureML
        """
        self.log = logging.getLogger('Sample_ONNX')

        # Assert all input parameters exist
        if download_model:
            self.log.info('Attempting to download AzureML model')

            self.log.debug('Initializing service principal auth with Azure')
            sp = ServicePrincipalAuthentication(
                    tenant_id=os.environ['AML_TENANT_ID'],
                    service_principal_id=os.environ['AML_PRINCIPAL_ID'],
                    service_principal_password=os.environ['AML_PRINCIPAL_PASS'])

            self.log.debug('Initializing AzureML Workspace connection')
            azure_ws = Workspace.get(
                    name=aml_ws, auth=sp,
                    subscription_id=aml_subscription_id)

            self.log.debug(
                    f'Downloading AzureML model "{model_name}" from workspace')
            model_name = Model(azure_ws, model_name).download(
                    target_dir='/tmp', exist_ok=True)
            self.log.info('Model download successful')

        self.log.info('Initializing ONNX runtime session')
        self.onnx_sess = rt.InferenceSession(model_name)

        self.onnx_input = self.onnx_sess.get_inputs()[0].name
        self.onnx_label = self.onnx_sess.get_outputs()[0].name
        self.input_shape = self.onnx_sess.get_inputs()[0].shape

        self.log.info('ONNX UDF initialized successfully')

    def process(self, frame, metadata):
        """Execute algorithm over each incoming frame.

        :param numpy.ndarray frame: Incoming frame to process
        :param dict metadata: Meta-data associaed with the frame
        :return: Tuple of (flag to drop frame, modified frame, metadata)
        :rtype: tuple
        """
        # Resize frame / preprocess frame
        img = cv2.resize(frame, (self.input_shape[2], self.input_shape[3]))
        img = img.transpose((2, 0, 1))
        X = np.asarray(img).astype(np.float32)
        X = np.expand_dims(X, axis=0)

        # Execute inference over the frame
        res = self.onnx_sess.run([self.onnx_label], {self.onnx_input: X})

        # Post process inference results
        res = postprocess(res)
        idx = np.argmax(res)
        self.log.debug(idx)
        metadata['class_idx'] = int(idx)
        self.log.debug(
                f'metadata: {metadata} added to classifier output queue')


        return False, None, metadata
