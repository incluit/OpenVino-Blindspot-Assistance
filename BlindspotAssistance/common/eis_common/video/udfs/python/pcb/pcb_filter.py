# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.


import logging
import cv2
import numpy as np
import time
from datetime import datetime
from random import uniform

"""Visual trigger for PCB anomaly detection.
"""


class Udf:
    """PCB anomaly detection trigger object.
    """

    def __init__(self, n_right_px, n_left_px,
                 n_total_px, training_mode, scale_ratio):
        """Udf constructor

        :param n_right_px: minimum number of pixels
                           to the right of PCB mask (default 1000)
        :type n_right_px: int
        :param n_left_px: minimum number of pixels
                          to the left of the PCB mask (default 1000)
        :type n_left_px: int
        :param n_total_px: minimum number of pixels
                           in the PCB mask (default 300000)
        :type n_total_px: int
        :param training_mode: flag to save image ROI's
                              for training (default false)
        :type training_mode: bool
        """
        self.log = logging.getLogger('PCB_FILTER')
        self.log.debug("In ctor")
        self.ratio = scale_ratio
        # Initialize background subtractor
        self.fgbg = cv2.createBackgroundSubtractorMOG2()
        # Total white pixel # on MOG applied
        # frame after morphological operations
        self.n_total_px = n_total_px/(self.ratio*self.ratio)
        # Total white pixel # on left edge of MOG
        # applied frame after morphological operations
        self.n_left_px = n_left_px/(self.ratio*self.ratio)
        # Total white pixel # on right edge of MOG
        # applied frame after morphological operations
        self.n_right_px = n_right_px/(self.ratio*self.ratio)
        # Flag to lock trigger from forwarding frames to classifier
        self.filter_lock = False
        self.training_mode = training_mode
        self.profiling = False
        self.count = 0
        self.lock_frame_count = 0
        self.threads = 0

    def _check_frame(self, frame):
        """Determines if the given frame is the key frame of interest for
        further processing or not

        :param frame: frame blob
        :type frame: numpy.ndarray
        :return: True if the given frame is a key frame, else False
        :rtype: bool
        """
        # Apply background subtractor on frame
        fgmask = self.fgbg.apply(frame)
        rows, columns = fgmask.shape
        if self.filter_lock is False:
            # Applying morphological operations
            kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (20, 20))
            ret, thresh = cv2.threshold(fgmask, 0, 255,
                                        cv2.THRESH_BINARY+cv2.THRESH_OTSU)
            thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel)
            # Crop left and right edges of frame
            left = thresh[:, 0:10]
            right = thresh[:, (columns - 10):(columns)]

            # Count the # of white pixels in thresh
            n_total = np.sum(thresh == 255)
            n_left = np.sum(left == 255)
            n_right = np.sum(right == 255)
            # If the PCB is in view of camera & is not
            # touching the left, right edge of frame
            if (n_total > self.n_total_px) & \
                (n_left < self.n_left_px) & \
                    (n_right < self.n_right_px):
                # Find the PCB contour
                contours, hier = cv2.findContours(thresh.copy(),
                                                  cv2.RETR_EXTERNAL,
                                                  cv2.CHAIN_APPROX_NONE)
                if len(contours) != 0:
                    # Contour with largest area would be bounding the PCB
                    c = max(contours, key=cv2.contourArea)

                    # Obtain the bounding rectangle
                    # for the contour and calculate the center
                    x, y, w, h = cv2.boundingRect(c)
                    cX = int(x + (w / 2))

                    # If the rectangle bounding the
                    # PCB doesn't touch the left or right edge
                    # of frame and the center x lies within
                    if (x != 0) & ((x + w) != columns) & \
                       ((columns/2 - (100/self.ratio)) <= cX and
                       cX <= (columns/2 + (100/self.ratio))):
                        return True
                    else:
                        return False
        return False

    def process(self, frame, metadata):
        """Processes every frame it receives based on the filter logic used

        :param frame: frame blob
        :type frame: numpy.ndarray
        :param metadata: frame's metadata
        :type metadata: str
        :return:  (should the frame be dropped, has the frame been updated,
                   new metadata for the frame if any)
        :rtype: (bool, numpy.ndarray, str)
        """
        frame_height, frame_width = frame.shape[:-1]
        resized_frame = cv2.resize(frame, (int(frame_width/self.ratio),
                                           int(frame_height/self.ratio)))

        if self.training_mode is True:
            self.count = self.count + 1
            cv2.imwrite("/EIS/test_videos/"+str(self.count)+".png", frame)
            return True, None, None
        else:
            if self.filter_lock is False:
                if self._check_frame(resized_frame):
                    self.filter_lock = True
                    # Re-initialize frame count during trigger lock to 0
                    self.lock_frame_count = 0
                    return False, None, metadata
                else:
                    return True, None, None
            else:
                # Continue applying background subtractor to
                # keep track of PCB positions
                self._check_frame(resized_frame)
                # Increment frame count during trigger lock phase
                self.lock_frame_count = self.lock_frame_count + 1
                if self.lock_frame_count == 7:
                    # Clear trigger lock after timeout
                    # period (measured in frame count here)
                    self.filter_lock = False
                return True, None, None
