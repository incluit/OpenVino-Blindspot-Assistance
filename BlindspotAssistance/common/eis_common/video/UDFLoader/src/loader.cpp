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
 * @brief Implementation of the @c UdfLoader class
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include "eis/udf/loader.h"
#include "eis/udf/python_udf_handle.h"
#include "eis/udf/native_udf_handle.h"
#include <eis/utils/logger.h>
#include "cython/udf.h"

using namespace eis::udf;

// Globals for Python interpreter
// PyInterpreterState* g_state;
PyThreadState* g_th_state = NULL;

UdfLoader::UdfLoader() {}

UdfLoader::~UdfLoader() {
    LOG_DEBUG_0("Destroying UDF Loader");
    if(g_th_state != NULL) {
        PyEval_RestoreThread(g_th_state);
        Py_FinalizeEx();
        g_th_state = NULL;
    }
}

UdfHandle* UdfLoader::load(
        std::string name, config_t* config, int max_workers)
{
	//TODO remove strcompare and convert to an enum?
	config_value_t* type = config_get(config, "type");

	if(type == NULL) {
		LOG_ERROR_0("Error retreiving UDF type");
		return NULL;
	}

	if(type->type != CVT_STRING) {
		LOG_ERROR_0("UDF type should be a string!");
		return NULL;
	}

    UdfHandle* udf = NULL;

	if(strcmp(type->body.string, "python") == 0) {
        if(!Py_IsInitialized()) {
            LOG_DEBUG_0("Initializing python");
            PyImport_AppendInittab("udf", PyInit_udf);
            Py_Initialize();
            PyEval_InitThreads();
            g_th_state = PyThreadState_Get();
        } else {
            // Attempt to load Python UDF
            PyEval_RestoreThread(g_th_state);
        }

    	udf = new PythonUdfHandle(name, max_workers);
    	if(!udf->initialize(config)) {
        	delete udf;
        	udf = NULL;
    	}

        PyEval_SaveThread();
        LOG_DEBUG("Has GIL: %d", PyGILState_Check());
    } else if (strcmp(type->body.string, "native") == 0) {
		//Attempt to load native UDF
		udf = new NativeUdfHandle(name, max_workers);
		if(!udf->initialize(config)) {
			delete udf;
			udf = NULL;
		}
	}

	config_value_destroy(type);

    return udf;
}
