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
 * @brief @c UdfHandle implementation
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include "eis/udf/udf_handle.h"
#include "eis/utils/logger.h"

using namespace eis::udf;

UdfHandle::UdfHandle(std::string name, int max_workers) :
    m_name(name), m_initialized(false), m_max_workers(max_workers),
    m_config(NULL)
{}

UdfHandle::~UdfHandle() {
    LOG_DEBUG_0("Base UdfHandle destructor");

    if(m_initialized.load()) {
        config_destroy(m_config);
    }

    // TODO: Clean up thread pool
}

bool UdfHandle::initialize(config_t* config) {
    // Verify the UDF handle has not already been initialized
    if(m_initialized.load()) {
        LOG_WARN_0("Initialize called twice for a given UDF handle");
        return true;
    }

    // Initialize internal state variables
    m_initialized.store(true);
    m_config = config;

    return true;
}

std::string UdfHandle::get_name() {
    return m_name;
}

std::string UdfHandle::get_prof_entry_key() {
    return m_prof_entry_key;
}

std::string UdfHandle::get_prof_exit_key() {
    return m_prof_exit_key;
}

void UdfHandle::set_prof_entry_key(std::string value) {
    m_prof_entry_key = value;
}

void UdfHandle::set_prof_exit_key(std::string value) {
    m_prof_exit_key = value;
}