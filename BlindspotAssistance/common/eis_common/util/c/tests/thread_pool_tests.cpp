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
 * @brief EIS thread safe queue unit tests
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#include <stdlib.h>
#include <thread>
#include <gtest/gtest.h>
#include "eis/utils/thread_pool.h"

using namespace eis::utils;

#define FAIL_NULL(val, msg) { \
    if(val == NULL) FAIL() << msg; \
}

void simple_run(void* vargs) {
    int* num = (int*) vargs;
    printf("-- Simple fn: %d\n", *num);
}

void sleep_run(void* vargs) {
    int* num = (int*) vargs;
    printf("-- Sleep fn: %d\n", *num);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST(th_pool_tests, simple) {
    auto duration = std::chrono::milliseconds(500);
    int val = 42;
    ThreadPool tp(1, -1);

    // Submit the job
    JobHandle* handle = tp.submit(simple_run, (void*) &val, NULL);
    FAIL_NULL(handle, "Handle is null");

    // The wait really should return immediately
    ASSERT_EQ(handle->wait_for(duration), true);

    // Clean up
    delete handle;
    tp.stop();
}

TEST(th_pool_tests, multi_jobs) {
    auto duration = std::chrono::milliseconds(500);
    ThreadPool tp(2, -1);

    int val = 0;
    JobHandle* handle = tp.submit(sleep_run, (void*) &val, NULL);
    FAIL_NULL(handle, "Handle is null");

    int val2 = 1;
    JobHandle* handle2 = tp.submit(sleep_run, (void*) &val2, NULL);
    FAIL_NULL(handle2, "Handle2 is null");

    // The wait really should return immediately
    handle->wait();
    handle2->wait();

    // Clean up
    delete handle;
    delete handle2;
    tp.stop();
}

TEST(th_pool_tests, queue_full) {
    ThreadPool tp(1, 1);

    int val = 0;
    JobHandle* handle = tp.submit(sleep_run, (void*) &val, NULL);
    FAIL_NULL(handle, "Handle is null");

    int val2 = 1;
    JobHandle* handle2 = tp.submit(sleep_run, (void*) &val2, NULL);
    FAIL_NULL(handle2, "Handle2 is null");

    handle2->wait();

    delete handle;
    delete handle2;
    tp.stop();
}
