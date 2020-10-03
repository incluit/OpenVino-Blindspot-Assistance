// Copyright (c) 2020 Intel Corporation.
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
 * @brief Handle for a UDF to be called via the loader.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UDF_UDF_HANDLE_H
#define _EIS_UDF_UDF_HANDLE_H

#include <atomic>
#include <string>
#include <eis/utils/config.h>
#include "eis/udf/frame.h"
#include "eis/udf/udfretcodes.h"

namespace eis {
namespace udf {

/**
 * UDF handle class.
 */
class UdfHandle {
private:
    // Name of the UDF
    std::string m_name;

    // Flag for if the UDF is initialized
    std::atomic<bool> m_initialized;

    // Max number of worker threads for executing the UDF
    int m_max_workers;

    // Profiling start timestamp key
    std::string m_prof_entry_key;

    // Profiling end timestamp key
    std::string m_prof_exit_key;

protected:
    // UDF Configuration
    config_t* m_config;

    // TODO: Add thread pool definition

public:
    /**
     * Constructor
     *
     * @param name        - Name of the UDF
     * @param max_workers - Max number of worker threads for the UDF
     */
    UdfHandle(std::string name, int max_workers);

    /**
     * Destructor
     */
    virtual ~UdfHandle();

    /**
     * Initialize the UDF.
     *
     * @param config - UDF configuration
     * @return bool
     */
    virtual bool initialize(config_t* config);

    /**
     * Return whether or not the UDF has been initialized.
     * @return bool
     */
    bool is_initialized();

    /**
     * Process the given frame.
     *
     * The return values have the following meanings for this method:
     *
     * - @c UdfRetCode::UDF_OK - Frame processed, no action required by caller
     * - @c UdfRetCode::UDF_DROP_FRAME - The caller of the UDF should not
     *      continue processing the frame given to the UDF
     * - @c UdfRetCode::UDF_MODIFIED_FRAMe - The caller should continue with
     *      the modified version of the frame
     *
     * @param frame - Frame to process
     * @return @c UdfRetCode
     */
    virtual UdfRetCode process(Frame* frame) = 0;

    /**
     * Get the name of the UDF.
     *
     * @return Name of the UDF
     */
    std::string get_name();

    /**
     * Get profiling start timestamp key
     *
     * @return profiling start timestamp key
     */
    std::string get_prof_entry_key();

    /**
     * Get profiling end timestamp key
     *
     * @return profiling end timestamp key
     */
    std::string get_prof_exit_key();

    /**
     * Set profiling start timestamp key
     *
     * @param value - Value to be set
     */
    void set_prof_entry_key(std::string value);

    /**
     * Set profiling end timestamp key
     *
     * @param value - Value to be set
     */
    void set_prof_exit_key(std::string value);
};

} // udf
} // eis

#endif // _EIS_UDF_UDF_HANDLE_H
