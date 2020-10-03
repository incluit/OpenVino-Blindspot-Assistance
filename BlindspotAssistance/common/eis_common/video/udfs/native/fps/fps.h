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
 * @brief Fps UDF Implementation to measure the frame rate
 */

#ifndef _FPS_H
#define _FPS_H

#include <eis/udf/base_udf.h>
#include <eis/utils/logger.h>
#include <iostream>
#include <chrono>
#include <safe_lib.h>
#include <mutex>

using namespace eis::udf;

/**
* FPS UDF is used to measure total frames received every second
*/
class FpsUdf : public BaseUdf {
    private:
        // Frame counter     
        int m_frame_count;
        // Fps value
       	int m_fps;
        // Start timer
	std::chrono::time_point<std::chrono::system_clock> m_start;
        // End timer
       	std::chrono::time_point<std::chrono::system_clock> m_end;
        // MsbBus return value
	msgbus_ret_t m_ret;
        // FPS key 
	char* m_fps_key;
        // Flag for first frame
	bool m_first_frame;
        // Mutex lock
        std::mutex m_mtx;

    public:
	/**
         * Constructor
         *
         * @param config - Config of the Native UDF
         */
	FpsUdf(config_t* config);

	/**
         * Destructor
         */
	~FpsUdf();

	/**
         * Overridden frame processing method.
         *
         * @param frame - Frame to process
	 * @param output - Output frame
	 * @param meta - Frame metadata
         * @return UdfRetCode
         */
	UdfRetCode process(cv::Mat& frame, cv::Mat& output, msg_envelope_t* meta) override;
};
#endif //_FPS_H
