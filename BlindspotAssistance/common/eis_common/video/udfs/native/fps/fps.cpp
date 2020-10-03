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
#include "fps.h"

using namespace eis::udf;

FpsUdf::FpsUdf(config_t* config): BaseUdf(config) {
    m_frame_count = 0;
    m_fps = 0;
    m_first_frame = true;
    int ret = 0;
    m_ret = MSG_SUCCESS;
    char* appname = getenv("AppName");
    if (appname == NULL) {
        LOG_ERROR_0("Failed to read AppName");
        throw "Failed to read Appname";
    }
    size_t appname_len = strlen(appname);
    m_fps_key = new char(appname_len);
    if (m_fps_key == NULL) {
        LOG_ERROR_0("Failed to allocate memory for fps key");
        throw "Failed to allocate memory for fps key";
    }

    ret = strncpy_s(m_fps_key,(appname_len + 1), appname, appname_len);
    if (ret != 0) {
        LOG_ERROR_0("Failed to read appname");
        throw "Failed to read appname";
    }

    ret = strncat_s(m_fps_key, (appname_len + 4), "Fps", 3);
    if (ret != 0) {
        LOG_ERROR_0("Failed to concatenate appname with fps key");
        throw "Failed to concatenate appname with fps key";
    }
}

FpsUdf::~FpsUdf() {
    delete m_fps_key;
}

UdfRetCode FpsUdf::process(cv::Mat& frame, cv::Mat& output, msg_envelope_t* meta) {
    m_mtx.lock();
    if (m_first_frame) {
        m_start = std::chrono::system_clock::now();
        m_first_frame = false; // First frame has been received
    }
    m_frame_count += 1;
    m_end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = m_end - m_start;
    if (elapsed_seconds.count() >= 1.0) {
	m_fps = m_frame_count;
	LOG_DEBUG("FPS: %d", m_fps);
	m_frame_count = 0;
	m_start = std::chrono::system_clock::now();
    }
    m_mtx.unlock();

    msg_envelope_elem_body_t *fps_int = msgbus_msg_envelope_new_integer(m_fps);
    if (fps_int == NULL) {
        LOG_ERROR_0("Failed to allocate fps integer");
        return UdfRetCode::UDF_ERROR;
    }

    m_ret = msgbus_msg_envelope_put(meta, m_fps_key, fps_int);
    if(m_ret != MSG_SUCCESS) {
        LOG_ERROR_0("Failed to add fps results in metadata");
        msgbus_msg_envelope_elem_destroy(fps_int);
        return UdfRetCode::UDF_ERROR;
    }
    return UdfRetCode::UDF_OK;
}

extern "C" {

/**
 * Create the UDF.
 *
 * @return void*
 */
void* initialize_udf(config_t* config) {
    FpsUdf* udf = new FpsUdf(config);
    return (void*) udf;
}

} // extern "C"

