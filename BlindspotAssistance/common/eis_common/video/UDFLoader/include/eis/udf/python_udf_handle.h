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
 * @brief Python UDF
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#ifndef _EIS_UDF_PYTHON_UDF_H
#define _EIS_UDF_PYTHON_UDF_H

#include <Python.h>
#include "eis/udf/udf_handle.h"

namespace eis {
namespace udf {

/**
 * Return value for a Python UDF.
 *
 * \note This is only used between the @c PythonUdfHandle and the Cython shim
 */
typedef struct {
    // UDF return code (i.e. UDF_OK, UDF_ERROR, etc.)
    UdfRetCode return_code;
    // Updated frame (if needed)
    PyObject* updated_frame;
} PythonUdfRet;

/**
 * Python UDF wrapper object
 */
class PythonUdfHandle : public UdfHandle {
private:
    // Reference to UDF Python object
    PyObject* m_udf_obj;

    // Reference to the process() method on the Python object
    PyObject* m_udf_func;

public:
    /**
     * Constructor
     *
     * @param name - Name of the Python UDF
     */
    PythonUdfHandle(std::string name, int max_workers);

    /**
     * Destructor
     */
    ~PythonUdfHandle();

    /**
     * Overridden initialization method
     *
     * @param config - UDF configuration
     * @return bool
     */
    bool initialize(config_t* config) override;

    /**
     * Overridden frame processing method.
     *
     * @param frame - Frame to process
     * @return UdfRetCode
     */
    UdfRetCode process(Frame* frame) override;
};

} // udf
} // eis

#endif // _EIS_UDF_PYTHON_UDF_H
