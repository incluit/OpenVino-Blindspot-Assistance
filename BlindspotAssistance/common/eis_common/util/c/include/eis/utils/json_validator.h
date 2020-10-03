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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief Utility function for validating a JSON file
 * @author Vishwas R (vishwas.r@intel.com)
 */

#include <wjelement.h>
#include <wjreader.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback that automatically get called
 * internally to free/cleanup open schema
 *
 *
 * @param schema - schema json object
 * @param client  - Calling object
 * @return NULL
 */
static void schema_free(WJElement schema, void *client);

/**
 * Callback that gets called to print schema
 * errors if any to stderr
 *
 *
 * @param client  - Calling object
 * @param format  - String to be printed
 * @return NULL
 */
static void schema_error(void *client, const char *format, ...);

/**
 * Validate the config json against the schema
 *
 *
 * @param schema  - schema json object
 * @param config  - config json object
 * @return @c int - return 1 on success & 0 on failure
 */
bool validate_json(WJElement schema, WJElement config);

#ifdef __cplusplus
}
#endif

