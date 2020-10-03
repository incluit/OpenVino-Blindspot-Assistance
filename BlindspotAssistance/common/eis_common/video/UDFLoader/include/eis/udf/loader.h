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
 * @brief UDF loader library entry point. Provides the object for loading UDFs
 *      into the application.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UDF_LOADER_H
#define _EIS_UDF_LOADER_H

#include <eis/utils/config.h>
#include "eis/udf/udf_handle.h"

namespace eis {
namespace udf {

/**
 * UDF loader object.
 */
class UdfLoader {
public:
    /**
     * Constructor
     */
    UdfLoader();

    /**
     * Destructor
     */
    ~UdfLoader();

    /**
     * Load a UDF from a library either in the `LD_LIBRARY_PATH` or the
     * `PYTHONPATH`.
     *
     * For native UDF implmentations, the `load()` method will search
     * in the LD_LIBRARY_PATH environmental variable directories. The
     * library names are expected to follow the naming convention:
     * `lib<name>.so`.
     *
     * @param name        - Name of the UDF to load
     * @param config      - Configuration for the UDF
     * @param max_workers - Maximum number of worker threads for executing UDFs
     * @return @c UdfHandle, NULL if not found
     */
    UdfHandle* load(std::string name, config_t* config, int max_workers);
};

} // udf
} // eis

#endif // _EIS_UDF_LOADER_H
