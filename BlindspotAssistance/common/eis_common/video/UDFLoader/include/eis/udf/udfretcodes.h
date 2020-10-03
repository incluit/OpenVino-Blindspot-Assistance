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
 * @brief Return values for UDFs.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UDF_UDF_RET_CODES_H
#define _EIS_UDF_UDF_RET_CODES_H

namespace eis {
namespace udf {

enum UdfRetCode {
    // Specifies that the UDF has processed and all is good, no action needed
    // by the caller
    UDF_OK = 0,

    // Specifies that the frame given to the process() method should dropped
    UDF_DROP_FRAME = 1,

    // Return value used specifically for Python UDFs
    UDF_FRAME_MODIFIED = 2,

    // The UDF encountered an error
    UDF_ERROR = 255,
};

} // udf
} // eis

#endif // _EIS_UDF_UDF_RET_CODES_H
