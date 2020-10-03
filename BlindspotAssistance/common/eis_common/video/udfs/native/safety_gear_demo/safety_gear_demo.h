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

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>

#include <eis/udf/base_udf.h>
#include <inference_engine.hpp>

#ifdef WITH_EXTENSIONS
#include <ext_list.hpp>
#endif

using namespace InferenceEngine;
using namespace eis::udf;

/**
 * The safety gear detection UDF (C++ based)
 */
class SafetyDemo : public BaseUdf {
    private:
        // Convolutional Neural network object to represent IR files
        CNNNetwork m_network;
        //Inference request object for submitting image
        InferRequest m_infer_request;
        // First tensor's image input name
        std::string m_image_input_name;
        // Input image name for different model type.(input shape = 2)
        std::string m_image_info_input_name;
        // Input information to the network
        InputsDataMap m_inputs_info;
        // Pointer to Input to the network
        InputInfo::Ptr m_input_info;
        // Output layer's name in XML
        std::string m_output_name;
        //output layer's last dimension.
        int m_object_size;
        //Maximum number of batches can run in paralell.
        int m_max_proposal_count;

    public:
        SafetyDemo(config_t *config);

        ~SafetyDemo();

        UdfRetCode process(cv::Mat &frame, cv::Mat &output, msg_envelope_t *meta) override;
};