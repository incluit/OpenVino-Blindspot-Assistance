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
 * @brief C++ UDF
 * @author Caleb McMillan <caleb.m.mcmillan@intel.com>
 */

#ifndef _EIS_UDF_NATIVE_UDF_H
#define _EIS_UDF_NATIVE_UDF_H


#include "eis/udf/udf_handle.h"
#include "eis/udf/base_udf.h"

namespace eis {
namespace udf {

class NativeUdfHandle : public UdfHandle {
private:
	//References needed after init
	void* m_lib_handle;
	void* (*m_func_initialize_udf)(config_t*);
	BaseUdf* m_udf;

    /**
     * Private @c NativeUdfHandle copy constructor.
     *
     * \note The constructor is copied, because this object is not supposed to
     *      be copied. It is important to note that this only blocks callers
     *      from passing the @c NativeUdfHandle by reference rather than as a
     *      pointer.
     */
    NativeUdfHandle(const NativeUdfHandle& src);

public:
    /**
     * Constructor
     *
     * @param name - Name of the Native UDF
     */
    NativeUdfHandle(std::string name, int max_workers);

    /**
     * Destructor
     */
    ~NativeUdfHandle();

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

} // eis
} // udf

#endif // _EIS_UDF_NATIVE_UDF_H
