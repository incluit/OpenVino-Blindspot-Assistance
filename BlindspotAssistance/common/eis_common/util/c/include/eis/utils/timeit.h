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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief Simple C macro utility for timing the execution time of a block of
 *        code.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UTILS_TIMEIT_H
#define _EIS_UTILS_TIMEIT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef WITH_TIMEIT

#include <time.h>
#include "eis/utils/logger.h"

#define TIMEIT(msg, ...) { \
    clock_t t; \
    t = clock(); \
    __VA_ARGS__; \
    t = clock() - t; \
    double elapsed = ((double)t) / CLOCKS_PER_SEC; \
    LOG_INFO(msg " : elapsed=%fs", elapsed); \
}
#else
#define TIMEIT(...)
#endif // WITH_TIMEIT

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _EIS_UTILS_TIMEIT_H
