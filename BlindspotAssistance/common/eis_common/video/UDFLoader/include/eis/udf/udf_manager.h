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
 * @brief UDF Manager thread with input/output frame queues.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */
#ifndef _EIS_UDF_UDF_MANAGER_H
#define _EIS_UDF_UDF_MANAGER_H

#include <thread>
#include <atomic>
#include <vector>
#include <eis/utils/config.h>
#include <eis/utils/thread_safe_queue.h>
#include <eis/utils/thread_pool.h>
#include <eis/utils/profiling.h>

#include "eis/udf/udf_handle.h"
#include "eis/udf/frame.h"

namespace eis {
namespace udf {

typedef utils::ThreadSafeQueue<Frame*> FrameQueue;

/**
 * UdfManager class
 */
class UdfManager {
private:
    // Thread
    std::thread* m_th;

    // Flag to stop the UDFManager thread
    std::atomic<bool> m_stop;

    // Configuration
    config_t* m_config;

    // UDF input queue
    FrameQueue* m_udf_input_queue;

    // UDF output queue
    FrameQueue* m_udf_output_queue;

    // Thread pool
    utils::ThreadPool* m_pool;

    // UDF Handles
    std::vector<UdfHandle*> m_udfs;

    // Profiling handle
    utils::Profiling* m_profile;

    // Queue blocked variable
    std::string m_udf_push_block_key;

    // UDF exit profiling key
    std::string m_udf_push_entry_key;

    // Caller's AppName
    std::string m_service_name;

    // Encoding details
    EncodeType m_enc_type;
    int m_enc_lvl;

    /**
     * @c UDFManager private thread run method.
     */
    void run();

    /**
     * Private @c UdfManager copy constructor.
     *
     * \note The constructor is copied, because this object is not supposed to
     *      be copied. It is important to note that this only blocks callers
     *      from passing the @c UdfManager by reference rather than as a
     *      pointer.
     */
    UdfManager(const UdfManager& src);

public:
    /**
     * Constructor
     *
     * @param udf_cfg      - UDF configurations
     * @param input_queue  - Input frame queue
     * @param output_queue - Output frame queue
     * @param enc_type     - Encoding to use on all frames put into the output
     *                       queue. (df: EncodeType::NONE)
     * @param enc_lvl      - Encoding level, must be between 0 and 9 for PNG
     *                       and 0 and 100 for JPEG (df: 0)
     */
    UdfManager(config_t* udf_cfg, FrameQueue* input_queue,
               FrameQueue* output_queue, std::string service_name,
               EncodeType enc_type=EncodeType::NONE,
               int enc_lvl=0);

    /**
     * Destructor
     */
    ~UdfManager();

    /**
     * Start the UDFManager thread
     */
    void start();

    /**
     * Stop the UDFManager thread
     */
    void stop();
};

} // udf
} // eis
#endif // _EIS_UDF_UDF_MANAGER_H