// Copyright (c) 2019 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief SafetyGear Demo UDF Implementation, ported to EIS from OpenVINO.
 */
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <eis/utils/logger.h>
#include <eis/utils/config.h>
#include <opencv2/opencv.hpp>

#include "safety_gear_demo.h"

using namespace InferenceEngine;
using namespace eis::udf;
//using namespace eis::udfsamples;
using namespace eis::msgbus;


SafetyDemo::SafetyDemo(config_t *config) : BaseUdf(config) {
    // --------------------------- Load inference engine -------------------------------------
    LOG_DEBUG("Initializing UDF, Entered constructor...");
    Core ie;
    config_value_t *path_to_bin = NULL;
    config_value_t *path_to_xml = NULL;
    config_value_t *device_type = NULL;

    if (config == NULL) {
        const char *err = " Config passed to UDF is NULL";
        LOG_ERROR("%s", err);
        throw err;
   }

    path_to_xml = config_get(config, "model_xml");
    if(path_to_xml != NULL) {
        if(path_to_xml->type != CVT_STRING) {
            const char *err = "IR file path must be a string";
            config_value_destroy(path_to_xml);
            throw err;
        }
    } else {
        const char *err = "XML PATH NULL config recieved";
        LOG_ERROR("%s", err);
        throw err;
    }
    path_to_bin = config_get(config, "model_bin");
    if(path_to_bin != NULL) {
        if(path_to_bin->type != CVT_STRING) {
            const char * err = "IR file path must be a string";
            LOG_ERROR("%s", err);
            config_value_destroy(path_to_xml);
            config_value_destroy(path_to_bin);
            throw err;
        }
    } else {
        const char *err = "BIN PATH NULL config recieved";
        LOG_ERROR("%s", err);
        throw err;
    }
    device_type = config_get(config, "device");
    if(device_type != NULL) {
        if(device_type->type != CVT_STRING) {
            const char * err = "device type must be a string";
            LOG_ERROR("%s", err);
            config_value_destroy(path_to_xml);
            config_value_destroy(path_to_bin);
            config_value_destroy(device_type);
            throw err;
        }
    } else {
        const char *err = "DEVICE TYPE NULL config recieved";
        LOG_ERROR("%s", err);
        throw err;
    }

    LOG_DEBUG_0("COMPLETED CONFIG READING....");

    if ((std::string("CPU").compare(device_type->body.string) == 0) ||
        (std::string("GPU").compare(device_type->body.string) == 0) ||
        (std::string("HDDL").compare(device_type->body.string) == 0) ||
        (std::string("MYRIAD").compare(device_type->body.string) == 0) ||
	(std::string("HETERO:FPGA,CPU").compare(device_type->body.string) == 0) ||
	(std::string("HETERO:FPGA,GPU").compare(device_type->body.string) == 0) ||
	(std::string("HETERO:FPGA,CPU,GPU").compare(device_type->body.string) == 0)) {
     #ifdef WITH_EXTENSIONS
        ie.AddExtension(std::make_shared<Extensions::Cpu::CpuExtensions>(), "CPU");
     #endif
    } else {
        //TODO: Will add support for GPU and improvise the above "if" caluse.
        const char *err = "Not a supported device to run Analytics";
        LOG_ERROR("Not a supported device: %s to run Analytics",
                  device_type->body.string);
        throw err;
    }
    LOG_DEBUG("COMPLETED LOADING CPU EXTENSION....");
    LOG_DEBUG("Loading IR files: \n\txml: %s, \n\tbin: %s\n",\
               path_to_xml->body.string, path_to_bin->body.string);

    InferenceEngine::Core core;
    /** Read network model **/
    m_network = core.ReadNetwork(path_to_xml->body.string, path_to_bin->body.string);

    LOG_DEBUG("COMPLETED scanning IR files....");

    // --------------------------- Prepare input blobs --------------------------------------------------
    LOG_INFO_0("Preparing input blobs");

    /** Taking information about all topology inputs **/
    m_inputs_info = m_network.getInputsInfo();

    m_input_info = nullptr;

    /** Iterating over all input blobs **/
    for (auto &item : m_inputs_info) {
        /** Working with first input tensor that stores image **/
        int size_first_tensor = item.second->getInputData()->getTensorDesc().getDims().size();
        if (size_first_tensor == 4) {
            m_image_input_name = item.first;

            m_input_info = item.second;

            /** Creating first input blob **/
            Precision inputPrecision = Precision::U8;
            item.second->setPrecision(inputPrecision);
        } else if (size_first_tensor == 2) {
            // This section will be needed for a different dimension model.
            m_image_info_input_name = item.first;

            Precision inputPrecision = Precision::FP32;
            item.second->setPrecision(inputPrecision);
            if ((item.second->getTensorDesc().getDims()[1] != 3 && item.second->getTensorDesc().getDims()[1] != 6)) {
                const char *err = "Invalid input info. Should be 3 or 6 values length";
                LOG_ERROR("%s", err);
                throw err;
            }
        }
    }

    LOG_DEBUG_0("COMPLETED PREPARING INPUT BLOB FROM IR files");

    if (m_input_info == nullptr) {
        m_input_info = m_inputs_info.begin()->second;
    }

    // --------------------------- 6. Prepare output blobs -------------------------------------------------
    LOG_INFO_0("Preparing output blobs");

    OutputsDataMap outputsInfo(m_network.getOutputsInfo());

    if (outputsInfo.size() != 1) {
        const char *err = "This application only supports networks with one output";
        LOG_ERROR("%s", err);
        throw err;
    }

    DataPtr outputInfo;
    m_output_name = outputsInfo.begin()->first;
    outputInfo = outputsInfo.begin()->second;

    if (outputInfo == nullptr) {
        const char *err = "Can't find a DetectionOutput layer in the topology";
        LOG_ERROR("%s", err);
        throw err;
    }

    const SizeVector outputDims = outputInfo->getTensorDesc().getDims();

    m_max_proposal_count = outputDims[2];
    m_object_size = outputDims[3];

    if (m_object_size != 7) {
        const char *err = "Output item should have 7 as a last dimension";
        LOG_ERROR("%s", err);
        throw err;
    }

    if (outputDims.size() != 4) {
        const char *err = "Incorrect output dimensions for SSD model";
        LOG_ERROR("%s", err);
        throw err;
    }
    LOG_DEBUG_0("COMPLETED PREPARING OUTPUT BLOB");

    /** Set the precision of output data provided by the user,
         *  should be called before load of the network to the device **/
    outputInfo->setPrecision(Precision::FP32);
    // ---------------------------------------------------------------

    // ---------------------------Loading model to the device --------
    LOG_INFO("Loading model to the device");
    ExecutableNetwork executable_network =
        ie.LoadNetwork(m_network, device_type->body.string);
    // -------------------------------------------------------------------

    // --------------------------- Create infer request ---------------
    LOG_INFO("Creating inference request");
    m_infer_request = executable_network.CreateInferRequest();
    LOG_INFO_0("COMPLETED UDF INTITIALIZATION....");

}

SafetyDemo::~SafetyDemo() {}

UdfRetCode SafetyDemo::process(cv::Mat &frame, cv::Mat &output, msg_envelope_t *meta) {

    LOG_DEBUG_0("Entered Native Safety Demo Udf::process() function...");

    msgbus_ret_t ret;
    /** Collect images data ptrs **/
    unsigned char* imagesData = nullptr;
    size_t imageWidths = 0;
    size_t imageHeights = 0;

    LOG_DEBUG("Resizing the image to \n\twidth: %lu \n\theight: %lu", m_input_info->getTensorDesc().getDims()[3],
                m_input_info->getTensorDesc().getDims()[2]);
    cv::Size size(m_input_info->getTensorDesc().getDims()[3], m_input_info->getTensorDesc().getDims()[2]);
    cv::Mat dst_image;
    resize(frame, dst_image, size);
    if (dst_image.data != NULL) {
        imagesData = dst_image.data;
        imageWidths = frame.size().width;
        imageHeights = frame.size().height;
    }

    /** Creating input blob **/
    Blob::Ptr imageInput = m_infer_request.GetBlob(m_image_input_name);

    /** Filling input tensor with images. First b channel, then g and r channels **/
    size_t num_channels = imageInput->getTensorDesc().getDims()[1];
    size_t image_size = imageInput->getTensorDesc().getDims()[3] * imageInput->getTensorDesc().getDims()[2];

    unsigned char *data = static_cast<unsigned char *>(imageInput->buffer());

    //Iterate over all pixel in image (b,g,r)
    LOG_DEBUG("Start copying to inference buffer...")
    for (size_t pid = 0; pid < image_size; pid++) {
        //Iterate over all channels
        for (size_t ch = 0; ch < num_channels; ++ch) {
            //[images stride + channels stride + pixel id] all in bytes
            data[ch * image_size + pid] = imagesData[pid * num_channels + ch];
        }
    }
    LOG_DEBUG("Copied the image to inference buffer")

    if (m_image_info_input_name != "") {
        Blob::Ptr input2 = m_infer_request.GetBlob(m_image_info_input_name);
        auto imInfoDim = m_inputs_info.find(m_image_info_input_name)->second->getTensorDesc().getDims()[1];

        /** Fill input tensor with values **/
        float *p = input2->buffer().as<PrecisionTrait<Precision::FP32>::value_type *>();
        p[0] = static_cast<float>(m_inputs_info[m_image_input_name]->getTensorDesc().getDims()[2]);
        p[1] = static_cast<float>(m_inputs_info[m_image_input_name]->getTensorDesc().getDims()[3]);
        for (size_t k = 2; k < imInfoDim; k++) {
            p[k] = 1.0f; // all scale factors are set to 1.0
        }
    }

    // --------------------------- Do inference ---------------------------------------------------------
    LOG_DEBUG("Start inference...");
    m_infer_request.Infer();
    // -----------------------------------------------------------------------------------------------------

    // --------------------------- Process output -------------------------------------------------------
    LOG_DEBUG("Processing output blobs....");
    const Blob::Ptr output_blob = m_infer_request.GetBlob(m_output_name);
    const float *detection = static_cast<PrecisionTrait<Precision::FP32>::value_type *>(output_blob->buffer());

    msg_envelope_elem_body_t *defects_arr = msgbus_msg_envelope_new_array();
    if (defects_arr == NULL) {
        LOG_ERROR_0("Failed to allocate defects array");
        return UdfRetCode::UDF_ERROR;
    }

    /* Each detection has image_id that denotes processed image */
    for (int curProposal = 0; curProposal < m_max_proposal_count; curProposal++) {
        int image_id = static_cast<int>(detection[curProposal * m_object_size + 0]);
        if (image_id < 0) {
            LOG_ERROR_0("In apropriate IMAGE ID obtained from inference");
            return UdfRetCode::UDF_ERROR;
        }

        float confidence = detection[curProposal * m_object_size + 2];
        int label = static_cast<int>(detection[curProposal * m_object_size + 1]);
        int xmin = static_cast<int>(detection[curProposal * m_object_size + 3] * imageWidths);
        int ymin = static_cast<int>(detection[curProposal * m_object_size + 4] * imageHeights);
        int xmax = static_cast<int>(detection[curProposal * m_object_size + 5] * imageWidths);
        int ymax = static_cast<int>(detection[curProposal * m_object_size + 6] * imageHeights);

        LOG_DEBUG("[%d, %d] element, prob = %f (%d, %d)-(%d, %d) image id : %d",
                curProposal, label, confidence, xmin, ymin, xmax, ymax, image_id);

        if (confidence > 0.5) {
            /**
             * defects object should look as below for visualizer to draw defects properly.
             *
             * {
             *      "defects": [
             *          {
             *              "type": label,
             *              "tl": [xmin, ymin],
             *              "br": [xmax, ymax]
             *          }
             *      ]
             * }
             */
            msg_envelope_elem_body_t *xmin_int = msgbus_msg_envelope_new_integer(xmin);
            if (xmin_int == NULL) {
                LOG_ERROR_0("Failed to allocate xmin integer");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                return UdfRetCode::UDF_ERROR;
            }
            msg_envelope_elem_body_t *ymin_int = msgbus_msg_envelope_new_integer(ymin);
            if (ymin_int == NULL) {
                LOG_ERROR_0("Failed to allocate ymin integer");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(xmin_int);
                return UdfRetCode::UDF_ERROR;
            }
            msg_envelope_elem_body_t *tl = msgbus_msg_envelope_new_array();
            if (tl == NULL) {
                LOG_ERROR_0("Failed to allocate topleft object");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(xmin_int);
                msgbus_msg_envelope_elem_destroy(ymin_int);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_array_add(tl, xmin_int);
            if (ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put xmin");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_array_add(tl, ymin_int);
            if (ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put ymin");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                return UdfRetCode::UDF_ERROR;
            }

            msg_envelope_elem_body_t *xmax_int = msgbus_msg_envelope_new_integer(xmax);
            if (xmax_int == NULL) {
                LOG_ERROR_0("Failed to allocate ymax integer");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                return UdfRetCode::UDF_ERROR;
            }
            msg_envelope_elem_body_t *ymax_int = msgbus_msg_envelope_new_integer(ymax);
            if (ymax_int == NULL) {
                LOG_ERROR_0("Failed to allocate ymax integer");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(xmax_int);
                return UdfRetCode::UDF_ERROR;
            }

            msg_envelope_elem_body_t *br = msgbus_msg_envelope_new_array();
            if (br == NULL) {
                LOG_ERROR_0("Failed to allocate bottomright object");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(xmax_int);
                msgbus_msg_envelope_elem_destroy(ymax_int);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_array_add(br, xmax_int);
            if(ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put xmax");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(br);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_array_add(br, ymax_int);
            if(ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put ymax");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(br);
                return UdfRetCode::UDF_ERROR;
            }

            msg_envelope_elem_body_t *roi = msgbus_msg_envelope_new_object();
            if (roi == NULL) {
                LOG_ERROR_0("Failed to allocate roi object");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(br);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_object_put(roi, "tl", tl);
            if (ret != MSG_SUCCESS) {
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(tl);
                msgbus_msg_envelope_elem_destroy(br);
                msgbus_msg_envelope_elem_destroy(roi);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_object_put(roi, "br", br);
            if(ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put br object in roi");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(roi);
                msgbus_msg_envelope_elem_destroy(br);
                return UdfRetCode::UDF_ERROR;
            }

            msg_envelope_elem_body_t *type = msgbus_msg_envelope_new_integer(label);
            if (type == NULL) {
                LOG_ERROR_0("Failed to allocate defect type specifying integer");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(roi);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_object_put(roi, "type", type);
            if(ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to label in roi");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(roi);
                msgbus_msg_envelope_elem_destroy(type);
                return UdfRetCode::UDF_ERROR;
            }

            ret = msgbus_msg_envelope_elem_array_add(defects_arr, roi);
            if(ret != MSG_SUCCESS) {
                LOG_ERROR_0("Failed to put roi to defect list");
                msgbus_msg_envelope_elem_destroy(defects_arr);
                msgbus_msg_envelope_elem_destroy(roi);
                return UdfRetCode::UDF_ERROR;
            }
            LOG_DEBUG_0("Above entry is detected with more than 0.5 Probability");
        }
    }

    ret = msgbus_msg_envelope_put(meta, "defects", defects_arr);
    if(ret != MSG_SUCCESS) {
        LOG_ERROR_0("Failed to defect array in meta config");
        //Element should be released recursively.
        msgbus_msg_envelope_elem_destroy(defects_arr);
        return UdfRetCode::UDF_ERROR;
    }

    LOG_DEBUG_0("Inference of frame completed.");
    return UdfRetCode::UDF_OK;
}

extern "C"
{

/**
 * ease the process of finding UDF symbol from shared object.
 *
 * @return void*
 */
    void *initialize_udf(config_t *config) {
        SafetyDemo *udf = new SafetyDemo(config);
        return (void *)udf;
    }

}; // extern "C"
