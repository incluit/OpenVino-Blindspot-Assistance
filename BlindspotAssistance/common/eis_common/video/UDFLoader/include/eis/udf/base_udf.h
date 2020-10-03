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
 * @brief Base UDF class for native UDF implementations.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UDF_BASE_UDF_H
#define _EIS_UDF_BASE_UDF_H

#include <atomic>
#include <opencv2/opencv.hpp>
#include <eis/msgbus/msg_envelope.h>
#include <eis/utils/config.h>
#include "eis/udf/udfretcodes.h"

namespace eis {
namespace udf {

class BaseUdf {
protected:
    // UDF configuration
    //
    // NOTE: The memory for this configuration is managed by the
    // @c UdfHandle class and does not need to be freed in the UDF object.
    config_t* m_config;

public:
    /**
     * Constructor
     */
    BaseUdf(config_t* config);

    /**
     * Destructor
     */
    virtual ~BaseUdf();

    /**
     * Process the given frame.
     *
     * @param frame - @c cv::Mat frame object
     * @param meta  - @c msg_envelope_t for the meta data to add to the frame
     *                after the UDF executes over it.
     * @return @c UdfRetCode
     */
    virtual UdfRetCode process(cv::Mat& frame, cv::Mat& output, msg_envelope_t* meta) = 0;
};

} // udf
} // eis

#endif // _EIS_UDF_BASE_UDF_H
