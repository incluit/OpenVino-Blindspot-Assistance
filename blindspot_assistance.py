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

import numpy as np
import io
from openvino.inference_engine import IENetwork, IECore
from pathlib import Path
sys.path.insert(0, str(Path().resolve().parent.parent))
from demoTools.demoutils import progressUpdate

def build_argparser():
    parser = ArgumentParser(add_help=False)
    args = parser.add_argument_group('Options')
    args.add_argument('-h', '--help', action='help', default=SUPPRESS, help='Show this help message and exit.')
    args.add_argument("-m", "--model", help="Required. Path to an .xml file with a trained model.",
                      required=True, type=str)
    args.add_argument("-i", "--input",
                      help="Required. Path to video file or image. 'cam' for capturing video stream from camera",
                      required=True, type=str)
    args.add_argument("-l", "--cpu_extension",
                      help="Optional. Required for CPU custom layers. Absolute path to a shared library with the "
                           "kernels implementations.", type=str, default=None)
    args.add_argument("-pp", "--plugin_dir", help="Path to a plugin folder", type=str, default=None)
    args.add_argument("-d", "--device",
                      help="Optional. Specify the target device to infer on; CPU, GPU, FPGA, HDDL or MYRIAD is "
                           "acceptable. The demo will look for a suitable plugin for device specified. "
                           "Default value is CPU", default="CPU", type=str)
    args.add_argument("--labels", help="Optional. Path to labels mapping file", default=None, type=str)
    args.add_argument("-pt", "--prob_threshold", help="Optional. Probability threshold for detections filtering",
                      default=0.5, type=float)
    args.add_argument("-ct", "--cpu_threads", help="Optional. Specifies number of threads that CPU plugin should "
                            "use for inference. Zero (default) means using all (logical) cores", default=None, type=str)
    args.add_argument("-o", "--output_dir", help="If set, it will write a video here instead of displaying it",
                            default=None, type=str)

    return parser

def processBoxes(frame_count, res, labels_map, prob_threshold, frame, initial_w, initial_h, result_file, det_time):
    for obj in res[0][0]:
        dims = ""
        # Draw only objects when probability more than specified threshold
        if obj[2] > prob_threshold:
            xmin = str(int(obj[3] * initial_w))
            ymin = str(int(obj[4] * initial_h))
            xmax = str(int(obj[5] * initial_w))
            ymax = str(int(obj[6] * initial_h))
            class_id = str(int(obj[1]))
            est = str(round(obj[2]*100, 1))
            time = round(det_time*1000)
            out_list = [str(frame_count), xmin, ymin, xmax, ymax, class_id, est, str(time)]
            for i in range(len(out_list)):
                dims += out_list[i]+' '
            dims += '\n'
            result_file.write(dims)

def switch_class(argument):
    switcher = {
        1: "Vehicle",
        2: "Pedestrian"
    }
    return switcher.get(argument, "Invalid Class")

def main():
    path = os.getcwd()
    print ("Welcome to Blindspot Assistance")

    log.basicConfig(format="[ %(levelname)s ] %(message)s", level=log.INFO, stream=sys.stdout)
    args = build_argparser().parse_args()

    model_xml = args.model
    model_bin = os.path.splitext(model_xml)[0] + ".bin"
    
    log.info("Creating Inference Engine...")
    ie = IECore()
    if args.cpu_threads:
        ie.set_config({'CPU_THREADS_NUM':args.cpu_threads},args.device)
    if args.cpu_extension and 'CPU' in args.device:
        ie.add_extension(args.cpu_extension, "CPU")
    # Read IR
    log.info("Loading network files:\n\t{}\n\t{}".format(model_xml, model_bin))
    net = IENetwork(model=model_xml, weights=model_bin)
    
    if "CPU" in args.device:
        supported_layers = ie.query_network(net, "CPU")
        not_supported_layers = [l for l in net.layers.keys() if l not in supported_layers]
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
    exec_net = ie.load_network(network=net, num_requests=2, device_name=args.device)
    # Read and pre-process input image
    n, c, h, w = net.inputs[input_blob].shape
    if img_info_input_blob:
        feed_dict[img_info_input_blob] = [h, w, 1]

    if args.input == 'cam':
        input_stream = 0
        out_file_name = 'cam'
    else:
        input_stream = args.input
        assert os.path.isfile(args.input), "Specified input file doesn't exist"
        out_file_name = os.path.splitext(os.path.basename(args.input))[0]

    if args.output_dir:
        out_path = os.path.join(args.output_dir, out_file_name+'.mp4')

    if args.labels:
        with open(args.labels, 'r') as f:
            labels_map = [x.strip() for x in f]
    else:
        labels_map = None

    cap = cv2.VideoCapture(input_stream)
    video_len = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    cur_request_id = 0
    next_request_id = 1

    log.info("Starting inference in async mode...")

    job_id = os.environ['PBS_JOBID']
    result_file = open(os.path.join(args.output_dir,'output_'+str(job_id)+'.txt'), "w")
    progress_file_path = os.path.join(args.output_dir,'i_progress_'+str(job_id)+'.txt')

    is_async_mode = True
    render_time = 0
    fps_sum = 0
    frame_count = 0
    inf_list = []
    res_list = []

    ret, frame = cap.read()
    
    roi = [0, 0 , int(cap.get(3) * 0.25), int(cap.get(4))]  # ROI: Autoselected 15% of the left

    print("To close the application, press 'CTRL+C' here or switch to the output window and press ESC key")
    print("To switch between sync/async modes, press TAB key in the output window")
    
    object_time = 0
    alarm = False
    object_detected = False

    try:
        infer_time_start = time.time()
        while cap.isOpened():
            if is_async_mode:
                read_time = time.time()
                ret, next_frame = cap.read()
            else:
                ret, frame = cap.read()
            if not ret:
                break
            initial_w = cap.get(3)
            initial_h = cap.get(4)

            # Selected rectangle overlay
            overlay = frame.copy()
            # cv2.rectangle(overlay, (roi[0], roi[1]), (roi[0] + roi[2], roi[1] + roi[3]), (0, 0, 0), -1)  # A filled rectangle
            alpha = 0.3  # Transparency factor.
            # cv2.addWeighted(overlay, alpha, frame, 1 - alpha, 0, frame) # Following line overlays transparent rectangle over the image

            # Main sync point:
            # in the truly Async mode we start the NEXT infer request, while waiting for the CURRENT to complete
            # in the regular mode we start the CURRENT request and immediately wait for it's completion
            inf_start = time.time()
            if is_async_mode:
                in_frame = cv2.resize(next_frame, (w, h))
                in_frame = in_frame.transpose((2, 0, 1))  # Change data layout from HWC to CHW
                in_frame = in_frame.reshape((n, c, h, w))
                feed_dict[input_blob] = in_frame
                exec_net.start_async(request_id=next_request_id, inputs=feed_dict)
            else:
                in_frame = cv2.resize(frame, (w, h))
                in_frame = in_frame.transpose((2, 0, 1))  # Change data layout from HWC to CHW
                in_frame = in_frame.reshape((n, c, h, w))
                feed_dict[input_blob] = in_frame
                exec_net.start_async(request_id=cur_request_id, inputs=feed_dict)
            if exec_net.requests[cur_request_id].wait(-1) == 0:
                inf_end = time.time()
                det_time = inf_end - inf_start

                # Parse detection results of the current request
                # output_blob = [image_id, label, conf, x_min, y_min, x_max, y_max]
                res = exec_net.requests[cur_request_id].outputs[out_blob]
                processBoxes(frame_count, res, labels_map, args.prob_threshold, frame, initial_w, initial_h, result_file, det_time)
                frame_count+=1
                #Write data to progress tracker
                if frame_count%10 == 0: 
                    progressUpdate(progress_file_path, time.time()-infer_time_start, frame_count, video_len) 

                for obj in res[0][0]:
                    # Draw only objects when probability more than specified threshold
                    if obj[2] > args.prob_threshold:
                        xmin = int(obj[3] * initial_w)
                        ymin = int(obj[4] * initial_h)
                        xmax = int(obj[5] * initial_w)
                        ymax = int(obj[6] * initial_h)
                        class_id = int(obj[1])
                        # Draw box and label\class_id
                        if (class_id == 1):
                            color = (0,255,0)
                        else:
                            color = (255,0,0)
                        #color = (min(class_id * 12.5, 255), min(class_id * 7, 255), min(class_id * 5, 255))
                        # cv2.rectangle(frame, (xmin, ymin), (xmax, ymax), color, 1)
                        det_label = labels_map[class_id] if labels_map else str(switch_class(class_id))
                        # cv2.putText(frame, det_label + ' ' + str(round(obj[2] * 100, 1)) + ' %', (xmin, ymin - 7),cv2.FONT_HERSHEY_COMPLEX, 0.5, color, 1)                    

                        if (xmin > roi[0] and xmin < roi[0] + roi[2]) or (xmax > roi[0] and xmax < roi[0] + roi[2]) or (xmin < roi[0] and xmax > roi[0] + roi[2]) :
                            if(ymin > roi[1] and ymin < roi[1] + roi[3]) or (ymax > roi[1] and ymax < roi[1] + roi[3]) or (ymin < roi[1] and ymax > roi[1] + roi[3]) : 
                                object_detected = True
                                last_object = str(switch_class(class_id))
                                    
            if object_detected:
                object_time = time.time()
                object_detected = False
                alarm = True
            else: 
                if (time.time() - object_time > 2):
                    alarm = False
            
            # Draw performance stats
            inf_time_message = "Inference time: N\A for async mode" if is_async_mode else \
                "Inference time: {:.3f} ms".format(det_time * 1000)
            render_time_message = "OpenCV rendering time: {:.3f} ms".format(render_time * 1000)
            async_mode_message = "Async mode is on. Processing request {}".format(cur_request_id) if is_async_mode else \
                "Async mode is off. Processing request {}".format(cur_request_id)

            # cv2.putText(frame, inf_time_message, (15, 15), cv2.FONT_HERSHEY_COMPLEX, 0.5, (200, 10, 10), 1)
            # cv2.putText(frame, render_time_message, (15, 30), cv2.FONT_HERSHEY_COMPLEX, 0.5, (10, 10, 200), 1)
            # cv2.putText(frame, async_mode_message, (10, int(initial_h - 20)), cv2.FONT_HERSHEY_COMPLEX, 0.5,(10, 10, 200), 1)

            render_start = time.time()
            # cv2.imshow("Detection Results", frame)
            
            render_end = time.time()
            render_time = render_end - render_start

            if is_async_mode:
                cur_request_id, next_request_id = next_request_id, cur_request_id
                frame = next_frame

            key = cv2.waitKey(1)
            if key == ord('l'):
                showCrosshair = False
                fromCenter = True
                
                roi = cv2.selectROI("Detection Results", frame, fromCenter, showCrosshair)
            if key == 27:
                break
            if (9 == key):
                is_async_mode = not is_async_mode
                log.info("Switched to {} mode".format("async" if is_async_mode else "sync"))

    ##End while loop /
        cap.release()
        result_file.close()

        if args.output_dir is None:
            cv2.destroyAllWindows()
        else:
            total_time = time.time() - infer_time_start
            with open(os.path.join(args.output_dir, 'stats_'+str(job_id)+'.txt'), 'w') as f:
                f.write(str(round(total_time, 1))+'\n')
                f.write(str(frame_count)+'\n')
    finally:
        del exec_net
        del ie

if __name__ == '__main__':
    sys.exit(main() or 0)
