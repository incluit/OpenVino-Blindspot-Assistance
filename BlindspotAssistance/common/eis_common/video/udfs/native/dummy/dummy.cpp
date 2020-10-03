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
 * @brief Dummy UDF Implementation
 */

#include <eis/udf/base_udf.h>
#include <eis/utils/logger.h>
#include <iostream>

using namespace eis::udf;

namespace eis {
    namespace udfsamples {

    /**
     * The Dummy UDF - does no processing
     */
        class DummyUdf : public BaseUdf {
            public:
                DummyUdf(config_t* config) : BaseUdf(config) {};

                ~DummyUdf() {};

                UdfRetCode process(cv::Mat& frame, cv::Mat& output, msg_envelope_t* meta) override {
                    LOG_DEBUG("In %s method...", __PRETTY_FUNCTION__);
                    return UdfRetCode::UDF_OK;
                };
        };
    } // udf
} // eis

extern "C" {

/**
 * Create the UDF.
 *
 * @return void*
 */
void* initialize_udf(config_t* config) {
    eis::udfsamples::DummyUdf* udf = new eis::udfsamples::DummyUdf(config);
    return (void*) udf;
}

} // extern "C"

