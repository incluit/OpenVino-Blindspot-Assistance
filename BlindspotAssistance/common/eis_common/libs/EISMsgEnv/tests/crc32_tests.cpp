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
 * @brief CRC32 Tests
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

// Enable use of timeit utility
#define WITH_TIMEIT

#include <gtest/gtest.h>
#include "eis/msgbus/crc32.h"

#define STR_VALUE      "CRC32_TEST"
#define EXPECTED_CRC32 737090954

TEST(crc32_tests, crc32) {
    uint32_t value = msgbus_crc32(STR_VALUE, strlen(STR_VALUE));
    uint32_t value2 = msgbus_crc32(STR_VALUE, strlen(STR_VALUE));
    ASSERT_EQ(value, value2) << "CRC32 did not return the same value twice";
    ASSERT_EQ(value, EXPECTED_CRC32) << "CRC32 was not expected value";
}
