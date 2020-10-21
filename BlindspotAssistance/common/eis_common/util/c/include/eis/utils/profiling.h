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
 * @brief C++ Profiling Library
 * @author Nagdeep GK <nagdeep.gk@intel.com>
 */

#ifndef _EIS_UTILS_PROFILING_H
#define _EIS_UTILS_PROFILING_H

#include <chrono>
#include <string>
#include <eis/msgbus/msg_envelope.h>
#include <eis/utils/config.h>
#include <eis/utils/logger.h>

namespace eis {
    namespace utils {
        class Profiling {
        private:
            // flag for if profiling enabled
            bool m_profiling_enabled;

        public:
            /** Constructor which read the profiling mode value from env variable
             * ,converts it to lower case & stores in member variable to be used 
             * the clients who create Profiling objects.
             * */
            Profiling();

            /**
             * API to check if profiling is enabled or not.
             * @return bool
             */
            bool is_profiling_enabled();

            /**
             * Add a profiling timestamp to the given meta data message envelope
             * API which Reads the current time as no: of miliseconds since epoch, then
             * converts it to int64_t format & then wraps it into msg_envelope_elem_body_t
             * format which is then encloses in msg_envelope format
             * @param meta                  -Input parameter: Message envelope to add meta-data to
             *                               The metadata object pointer of type msg_envelope_t to be
             *                               passed by the calling module.
             * @param key                   - INput parameter string: Key for the timestamp to be added
             *                                The "key" value to be placed in teh metadata
             *                                key/value pair for profiling.
             * */
            void add_profiling_ts(msg_envelope_t* meta_data, const char* key);

            /**
             * Utility function to be used to get the current time since epoch in miliseconds 
             * in int64_t format
             * */
            int64_t get_curr_time_as_int_epoch();
        };

        //Macros for ease of use by calling modules
        #define DO_PROFILING(profile, meta, ts_key) \
            if(profile->is_profiling_enabled()) { profile->add_profiling_ts(meta, ts_key);}

    };
};


#endif