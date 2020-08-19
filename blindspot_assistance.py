#!/usr/bin/env python
"""
 Copyright (C) 2018-2019 Intel Corporation

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

from __future__ import print_function
import sys
import os
from argparse import ArgumentParser, SUPPRESS
import cv2
import time
import logging as log
from threading import Thread
import pafy

from openvino.inference_engine import IENetwork, IECore


def build_argparser():
    parser = ArgumentParser(add_help=False)
    args = parser.add_argument_group('Options')
    args.add_argument('-h', '--help', action='help',
                      default=SUPPRESS, help='Show this help message and exit.')
    args.add_argument("-m", "--model", help="Required. Path to an .xml file with a trained model.",
                      required=True, type=str)
    args.add_argument("-i", "--input",
                      help="Required. Path to video file, YouTube video (URL) and image. 'cam' for capturing video stream from camera",
                      required=True, type=str)
    args.add_argument("-l", "--cpu_extension",
                      help="Optional. Required for CPU custom layers. Absolute path to a shared library with the "
                           "kernels implementations.", type=str, default=None)
    args.add_argument("-d", "--device",
                      help="Optional. Specify the target device to infer on; CPU, GPU, FPGA, HDDL or MYRIAD is "
                           "acceptable. The demo will look for a suitable plugin for device specified. "
                           "Default value is CPU", default="CPU", type=str)
    args.add_argument(
        "--labels", help="Optional. Path to labels mapping file", default=None, type=str)
    args.add_argument("-pt", "--prob_threshold", help="Optional. Probability threshold for detections filtering",
                      default=0.5, type=float)
    args.add_argument("-ct", "--cpu_threads", help="Optional. Specifies number of threads that CPU plugin should "
                      "use for inference. Zero (default) means using all (logical) cores", default=None, type=str)
    args.add_argument(
        "-o", "--output", help="Optional. Save the video output. Define the path of the video file", default=None, type=str)
    args.add_argument("-h_o", "--hide_output",
                      help="Optional. Hide the output.",  action='store_true')
    args.add_argument("-ni", "--number_iter",
                      help="Optional. Number of inference iterations", default=1, type=int)

    return parser


def switch_class(argument):
    switcher = {
        1: "Vehicle",
        2: "Pedestrian",
        3: "Bike"
    }
    return switcher.get(argument, "Invalid Class")


def switch_class_color(argument):
    switcher = {
        1: (0, 255, 0),
        2: (255, 0, 0),
        3: (0, 0, 255)
    }
    return switcher.get(argument, (255, 255, 255))


def main():
    path = os.getcwd()
    print("Welcome to Blindspot Assistance")

    log.basicConfig(format="[ %(levelname)s ] %(message)s",
                    level=log.INFO, stream=sys.stdout)
    args = build_argparser().parse_args()

    model_xml = args.model
    model_bin = os.path.splitext(model_xml)[0] + ".bin"

    log.info("Creating Inference Engine...")
    ie = IECore()
    if args.cpu_threads:
        ie.set_config({'CPU_THREADS_NUM': args.cpu_threads}, args.device)
    if args.cpu_extension and 'CPU' in args.device:
        ie.add_extension(args.cpu_extension, "CPU")
    # Read IR
    log.info("Loading network files:\n\t{}\n\t{}".format(model_xml, model_bin))
    net = IENetwork(model=model_xml, weights=model_bin)

    if "CPU" in args.device:
        supported_layers = ie.query_network(net, "CPU")
        not_supported_layers = [
            l for l in net.layers.keys() if l not in supported_layers]
        if len(not_supported_layers) != 0:
            log.error("Following layers are not supported by the plugin for specified device {}:\n {}".
                      format(args.device, ', '.join(not_supported_layers)))
            log.error("Please try to specify cpu extensions library path in sample's command line parameters using -l "
                      "or --cpu_extension command line argument")
            sys.exit(1)

    img_info_input_blob = None
    feed_dict = {}
    for blob_name in net.inputs:
        if len(net.inputs[blob_name].shape) == 4:
            input_blob = blob_name
        elif len(net.inputs[blob_name].shape) == 2:
            img_info_input_blob = blob_name
        else:
            raise RuntimeError("Unsupported {}D input layer '{}'. Only 2D and 4D input layers are supported"
                               .format(len(net.inputs[blob_name].shape), blob_name))

    assert len(net.outputs) == 1, "Demo supports only single output topologies"

    out_blob = next(iter(net.outputs))
    log.info("Loading IR to the plugin...")
    exec_net = ie.load_network(
        network=net, num_requests=args.number_iter, device_name=args.device)
    # Read and pre-process input image
    n, c, h, w = net.inputs[input_blob].shape
    if img_info_input_blob:
        feed_dict[img_info_input_blob] = [h, w, 1]

    if args.input == 'cam':
        input_stream = 0
    else:
        input_stream = args.input
        # Detect if the input is a Youtube Video (with Pafy)
        if "youtube.com" in input_stream:
            video = pafy.new(url=input_stream)
            stream = video.getbest()
            input_stream = stream.url
        else:
            assert os.path.isfile(
                args.input), "Specified input file doesn't exist"
    if args.labels:
        with open(args.labels, 'r') as f:
            labels_map = [x.strip() for x in f]
    else:
        labels_map = None

    cap = cv2.VideoCapture(input_stream)

    if args.output:
        FILE_OUTPUT = args.output
        if os.path.isfile(FILE_OUTPUT):
            os.remove(FILE_OUTPUT)
        fourcc = cv2.VideoWriter_fourcc('M', 'J', 'P', 'G')
        fps = cap.get(cv2.CAP_PROP_FPS)
        out = cv2.VideoWriter(FILE_OUTPUT, fourcc, fps,
                              (int(cap.get(3)), int(cap.get(4))))

    log.info("Starting inference in async mode...")
    is_async_mode = True
    render_time = 0
    ret, frame = cap.read()

    # ROI: Autoselected 15% of the left
    roi = [0, 0, int(cap.get(3) * 0.25), int(cap.get(4))]

    print("To close the application, press 'CTRL+C' here or switch to the output window and press ESC key")

    object_time = 0
    alarm = False
    object_detected = False

    while cap.isOpened():

        inf_start = time.time()
        for i in range(0, args.number_iter):
            ret, frame = cap.read()
            initial_w = cap.get(3)
            initial_h = cap.get(4)

            # Selected rectangle overlay
            overlay = frame.copy()
            cv2.rectangle(overlay, (roi[0], roi[1]), (roi[0] + roi[2],
                                                    roi[1] + roi[3]), (0, 0, 0), -1)  # A filled rectangle
            alpha = 0.3  # Transparency factor.
            # Following line overlays transparent rectangle over the image
            cv2.addWeighted(overlay, alpha, frame, 1 - alpha, 0, frame)

            in_frame = cv2.resize(frame, (w, h))
            # Change data layout from HWC to CHW
            in_frame = in_frame.transpose((2, 0, 1))
            in_frame = in_frame.reshape((n, c, h, w))
            feed_dict[input_blob] = in_frame
            exec_net.start_async(request_id=i, inputs=feed_dict)

        for i in range(0, args.number_iter):
            if exec_net.requests[i].wait(-1) == 0:
                # Parse detection results of the current request
                # output_blob = [image_id, label, conf, x_min, y_min, x_max, y_max]
                res = exec_net.requests[0].outputs[out_blob]
                for obj in res[0][0]:
                    # Draw only objects when probability more than specified threshold
                    if obj[2] > args.prob_threshold:
                        xmin = int(obj[3] * initial_w)
                        ymin = int(obj[4] * initial_h)
                        xmax = int(obj[5] * initial_w)
                        ymax = int(obj[6] * initial_h)
                        class_id = int(obj[1])
                        # Draw box and label\class_id
                        color = switch_class_color(class_id)
                        cv2.rectangle(frame, (xmin, ymin),
                                      (xmax, ymax), color, 1)
                        det_label = labels_map[class_id] if labels_map else str(
                            switch_class(class_id))
                        cv2.putText(frame, det_label + ' ' + str(round(obj[2] * 100, 1)) + ' %', (xmin, ymin - 7),
                                    cv2.FONT_HERSHEY_COMPLEX, 0.5, color, 1)

                        if (xmin > roi[0] and xmin < roi[0] + roi[2]) or (xmax > roi[0] and xmax < roi[0] + roi[2]) or (xmin < roi[0] and xmax > roi[0] + roi[2]):
                            if(ymin > roi[1] and ymin < roi[1] + roi[3]) or (ymax > roi[1] and ymax < roi[1] + roi[3]) or (ymin < roi[1] and ymax > roi[1] + roi[3]):
                                object_detected = True
                                last_object = str(switch_class(class_id))

            if object_detected:
                object_time = time.time()
                object_detected = False
                alarm = True
            else:
                if (time.time() - object_time > 2):
                    alarm = False
            if alarm:
                cv2.circle(frame, (25, 20), 10, (0, 0, 255), -1)
                cv2.putText(frame, "Last object detected: " + last_object,
                            (40, 25), cv2.FONT_HERSHEY_COMPLEX, 0.5, (0, 0, 255), 1)
            else:
                cv2.circle(frame, (25, 20), 10, (0, 255, 0), -1)
                cv2.putText(frame, "Nothing detected", (40, 25),
                            cv2.FONT_HERSHEY_COMPLEX, 0.5, (0, 255, 0), 1)

            render_start = time.time()

            if args.output:
                out.write(frame)
            if not args.hide_output:
                cv2.imshow("Detection Results", frame)

            render_end = time.time()
            render_time = render_end - render_start

        inf_end = time.time()
        det_time = (inf_end - inf_start)/args.number_iter
        print("Inference time: {:.1f} FPS".format(1 / det_time))
        # Draw performance stats

        key = cv2.waitKey(1)
        if key == ord('l'):
            showCrosshair = False
            fromCenter = True

            roi = cv2.selectROI("Detection Results", frame,
                                fromCenter, showCrosshair)
        if key == 27:
            break
        if (9 == key):
                is_async_mode = not is_async_mode
                log.info("Switched to {} mode".format(
                    "async" if is_async_mode else "sync"))

    cv2.destroyAllWindows()


if __name__ == '__main__':
    sys.exit(main() or 0)