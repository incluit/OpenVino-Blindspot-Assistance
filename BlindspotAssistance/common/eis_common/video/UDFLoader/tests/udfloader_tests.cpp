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
 * @brief Unit tests for the @c UDFLoader object and for calling UDFs
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include <chrono>
#include <gtest/gtest.h>
#include <eis/utils/logger.h>
#include <eis/utils/json_config.h>
#include "eis/udf/loader.h"
#include "eis/udf/udf_manager.h"

#define ORIG_FRAME_DATA "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define NEW_FRAME_DATA  "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
#define DATA_LEN 10

using namespace eis::udf;

#define ASSERT_NULL(val) { \
    if(val != NULL) FAIL() << "Value should be NULL"; \
}

#define ASSERT_NOT_NULL(val) { \
    if(val == NULL) FAIL() << "Value shoud not be NULL"; \
}

//
// Helper objects
//

/**
 * Test object to represent a video frame
 */
class TestFrame {
public:
    uint8_t* data;

    TestFrame(uint8_t* data) : data(data)
    {};

    ~TestFrame() { delete[] data; };
};

void test_frame_free(void* hint) {
    TestFrame* tf = (TestFrame*) hint;
    delete tf;
}

Frame* init_frame() {
    uint8_t* data = new uint8_t[DATA_LEN];
    memcpy(data, ORIG_FRAME_DATA, DATA_LEN);

    TestFrame* tf = new TestFrame(data);

    Frame* frame = new Frame(
            (void*) tf, DATA_LEN, 1, 1, (void*) data, test_frame_free);

    return frame;
}

UdfLoader* loader = NULL;

// Test to modify the underlying frame from a Python UDF and to modify the meta
// data. This test also tests the UDFLoader's ability to load a UDF that is in
// a Python package.
TEST(udfloader_tests, py_modify) {
    // Load a configuration
    config_t* config = json_config_new("test_config.json");
    ASSERT_NOT_NULL(config);

    // Initialize the UDFLoader and load the UDF
    UdfHandle* handle = loader->load("py_tests.modify", config, 1);
    ASSERT_NOT_NULL(handle);

    // Initialize the frame to use
    Frame* frame = init_frame();
    ASSERT_NOT_NULL(frame);

    // Execute the UDF over the frame
    UdfRetCode ret = handle->process(frame);
    ASSERT_EQ(ret, UdfRetCode::UDF_OK);

    // Verify frame data is correct
    uint8_t* frame_data = (uint8_t*) frame->get_data();
    for(int i = 0; i < DATA_LEN; i++) {
        ASSERT_EQ(frame_data[i], NEW_FRAME_DATA[i]);
    }

    // Verify that the added meta-data shows up in the meta-data of the frame
    msg_envelope_t* meta = frame->get_meta_data();
    ASSERT_NOT_NULL(meta);

    msg_envelope_elem_body_t* added;
    msgbus_ret_t m_ret = msgbus_msg_envelope_get(meta, "ADDED", &added);
    ASSERT_EQ(m_ret, MSG_SUCCESS);
    ASSERT_EQ(added->type, MSG_ENV_DT_INT);
    ASSERT_EQ(added->body.integer, 55);

    // Clean up
    delete frame;
    delete handle;
}

// Test to drop a frame
TEST(udfloader_tests, py_drop_frame) {
    // Load a configuration
    config_t* config = json_config_new("test_config.json");
    ASSERT_NOT_NULL(config);

    // Initialize the UDFLoader and load the UDF
    UdfHandle* handle = loader->load("py_tests.drop", config, 1);
    ASSERT_NOT_NULL(handle);

    // Initialize the frame to use
    Frame* frame = init_frame();
    ASSERT_NOT_NULL(frame);

    // Execute the UDF over the frame
    UdfRetCode ret = handle->process(frame);
    ASSERT_EQ(ret, UdfRetCode::UDF_DROP_FRAME);

    // Clean up
    delete frame;
    delete handle;
}

// Test to verify configuration
TEST(udfloader_tests, py_config) {
    // Load a configuration
    config_t* config = json_config_new("test_config.json");
    ASSERT_NOT_NULL(config);

    // Initialize the UDFLoader and load the UDF
    UdfHandle* handle = loader->load("py_tests.config", config, 1);
    ASSERT_NOT_NULL(handle);

    // Clean up
    delete handle;
}

// Test for exception in constructor
TEST(udfloader_tests, py_constructor_error) {
    // Load a configuration
    config_t* config = json_config_new("test_config.json");
    ASSERT_NOT_NULL(config);

    // Initialize the UDFLoader and load the UDF
    UdfHandle* handle = loader->load("py_tests.error", config, 1);
    ASSERT_NULL(handle);

    // Clean up
    delete handle;
}

// Test exception in process
TEST(udfloader_tests, py_process_error) {
    // Load a configuration
    config_t* config = json_config_new("test_config.json");
    ASSERT_NOT_NULL(config);

    // Initialize the UDFLoader and load the UDF
    UdfHandle* handle = loader->load("py_tests.process_error", config, 1);
    ASSERT_NOT_NULL(handle);

    // Initialize the frame to use
    Frame* frame = init_frame();
    ASSERT_NOT_NULL(frame);

    // Execute the UDF over the frame
    UdfRetCode ret = handle->process(frame);
    ASSERT_EQ(ret, UdfRetCode::UDF_ERROR);

    // Clean up
    delete frame;
    delete handle;
}

TEST(udfloader_tests, reinitialize) {
    try {
        config_t* config = json_config_new("test_udf_mgr_config.json");
        ASSERT_NOT_NULL(config);

        FrameQueue* input_queue = new FrameQueue(-1);
        FrameQueue* output_queue = new FrameQueue(-1);

        UdfManager* manager = new UdfManager(config, input_queue, output_queue, "");
        manager->start();

        Frame* frame = init_frame();
        ASSERT_NOT_NULL(frame);

        input_queue->push(frame);

        std::this_thread::sleep_for(std::chrono::seconds(3));

        delete manager;

        input_queue = new FrameQueue(-1);
        output_queue = new FrameQueue(-1);
        config = json_config_new("test_udf_mgr_config.json");
        manager = new UdfManager(config, input_queue, output_queue, "");
        manager->start();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        delete manager;
    } catch(const std::exception& ex) {
        FAIL() << ex.what();
    }
}

/**
 * Overridden GTest main method
 */
GTEST_API_ int main(int argc, char** argv) {
    // Parse out gTest command line parameters
    ::testing::InitGoogleTest(&argc, argv);
    set_log_level(LOG_LVL_DEBUG);
    loader = new UdfLoader();
    int res = RUN_ALL_TESTS();
    delete loader;
    return res;
}
