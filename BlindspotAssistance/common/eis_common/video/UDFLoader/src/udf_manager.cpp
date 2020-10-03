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
 * @brief @c UdfManager class implementation.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include <chrono>
#include <eis/utils/logger.h>
#include <sstream>
#include <random>
#include <safe_lib.h>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include "eis/udf/udf_manager.h"
#include "eis/udf/frame.h"
#include "eis/udf/loader.h"

using namespace eis::udf;
using namespace eis::utils;

#define CFG_UDFS            "udfs"
#define CFG_MAX_JOBS        "max_jobs"
#define CFG_MAX_WORKERS     "max_workers"
#define DEFAULT_MAX_WORKERS 4  // Default 4 threads to submit jobs to
#define DEFAULT_MAX_JOBS    20 // Default for the number of queued jobs
#define RANDOM_STR_LENGTH   5  // Size of random strings to be added for profiling keys

// Globals
UdfLoader g_loader;

void free_fn(void* ptr) {
    config_value_t* obj = (config_value_t*) ptr;
    config_value_destroy(obj);
}

config_value_t* get_config_value(const void* cfg, const char* key) {
    config_value_t* obj = (config_value_t*) cfg;
    return config_value_object_get(obj, key);
}

std::string generate_rand_string(const int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        const auto rc = dis(gen);
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

UdfManager::UdfManager(
        config_t* udf_cfg, FrameQueue* input_queue, FrameQueue* output_queue,
        std::string service_name, EncodeType enc_type, int enc_lvl) :
    m_th(NULL), m_stop(false), m_config(udf_cfg),
    m_udf_input_queue(input_queue), m_udf_output_queue(output_queue),
    m_service_name(service_name), m_enc_type(enc_type), m_enc_lvl(enc_lvl)
{
    config_value_t* udfs = NULL;

    LOG_DEBUG_0("Loading UDFs");
    udfs = config_get(m_config, CFG_UDFS);
    if(udfs == NULL) {
        throw "Failed to get UDFs";
    }
    if(udfs->type != CVT_ARRAY) {
        config_value_destroy(udfs);
        throw "\"udfs\" must be an array";
    }

    // Get maximum jobs (if it exists)
    int max_jobs = DEFAULT_MAX_JOBS;
    config_value_t* cfg_max_jobs = config_get(m_config, CFG_MAX_JOBS);
    if(cfg_max_jobs != NULL) {
        if(cfg_max_jobs->type != CVT_INTEGER) {
            config_value_destroy(cfg_max_jobs);
            config_value_destroy(udfs);
            throw "\"max_jobs\" must be an integer";
        }
        max_jobs = cfg_max_jobs->body.integer;
        config_value_destroy(cfg_max_jobs);
    }
    LOG_INFO("max_jobs: %d", max_jobs);

    // Get the maximum number of workers
    int max_workers = DEFAULT_MAX_WORKERS;
    config_value_t* cfg_max_workers = config_get(m_config, CFG_MAX_WORKERS);
    if(cfg_max_workers != NULL) {
        if(cfg_max_workers->type != CVT_INTEGER) {
            config_value_destroy(cfg_max_workers);
            config_value_destroy(udfs);
            throw "\"max_jobs\" must be an integer";
        }
        max_workers = cfg_max_workers->body.integer;
        config_value_destroy(cfg_max_workers);
    }
    LOG_INFO("max_workers: %d", max_workers);

    // Initialize thread pool
    m_pool = new ThreadPool(max_workers, max_jobs);

    m_profile = new Profiling();

    int len = (int) config_value_array_len(udfs);

    for(int i = 0; i < len; i++) {
        config_value_t* cfg_obj = config_value_array_get(udfs, i);
        if(cfg_obj == NULL) {
            throw "Failed to get configuration array element";
        }
        if(cfg_obj->type != CVT_OBJECT) {
            throw "UDF configuration must be objects";
        }
        config_value_t* name = config_value_object_get(cfg_obj, "name");
        if(name == NULL) {
            throw "Failed to get UDF name";
        }
        if(name->type != CVT_STRING) {
            throw "UDF name must be a string";
        }
        // TODO: Add max workers
        void (*free_ptr)(void*) = NULL;
        if(cfg_obj->body.object->free == NULL) {
            free_ptr = free_fn;
        } else {
            free_ptr = cfg_obj->body.object->free;
        }
        config_t* cfg = config_new(
                (void*) cfg_obj, free_ptr, get_config_value);
        if(cfg == NULL) {
            throw "Failed to initialize configuration for UDF";
        }

        LOG_DEBUG("Loading UDF...");
        UdfHandle* handle = g_loader.load(name->body.string, cfg, 1);
        if(handle == NULL) {
            throw "Failed to load UDF";
        }

        if(m_profile->is_profiling_enabled()) {

            std::string udf_name_str(name->body.string);
            std::string rand_str = generate_rand_string(RANDOM_STR_LENGTH);
            if(i == 0) {
                std::string udf_entry_str = udf_name_str + "_" + rand_str + "_" + m_service_name + "_first" + "_entry";
                std::string udf_exit_str = udf_name_str + "_" + rand_str + "_" + m_service_name + "_first" + "_exit";

                handle->set_prof_entry_key(udf_entry_str);
                handle->set_prof_exit_key(udf_exit_str);

            } else {
                std::string udf_entry_str = udf_name_str + "_" + rand_str + "_" + m_service_name + "_entry";
                std::string udf_exit_str = udf_name_str + "_" + rand_str + "_" + m_service_name + "_exit";

                handle->set_prof_entry_key(udf_entry_str);
                handle->set_prof_exit_key(udf_exit_str);

            }
        }
        config_value_destroy(name);
        m_udfs.push_back(handle);
    }
    m_udf_push_entry_key = m_service_name + "_UDF_output_queue_ts";
    m_udf_push_block_key = m_service_name + "_UDF_output_queue_blocked_ts";

    config_value_destroy(udfs);
}

UdfManager::UdfManager(const UdfManager& src) {
    // This method does nothing, because the object is not supposed to be
    // copied
}

UdfManager::~UdfManager() {
    this->stop();
    if(m_th != NULL) {
        delete m_th;
    }

    delete m_pool;

    LOG_DEBUG_0("Deleting all handles");
    for(auto handle : m_udfs) {
        delete handle;
    }

    LOG_DEBUG_0("Deleting UDF timestamp related variables");
    if(m_profile) {
        delete m_profile;
    }

    LOG_DEBUG_0("Clearing udf input queue");
    // Clear queues and delete them
    while(!m_udf_input_queue->empty()) {
        Frame* frame = m_udf_input_queue->front();
        m_udf_input_queue->pop();
        delete frame;
    }
    LOG_DEBUG_0("Cleared udf input queue");
    delete m_udf_input_queue;

    LOG_DEBUG_0("Clearing udf output queue");
    while(!m_udf_output_queue->empty()) {
        Frame* frame = m_udf_output_queue->front();
        m_udf_output_queue->pop();
        delete frame;
    }
    LOG_DEBUG_0("Cleared udf output queue");
    delete m_udf_output_queue;

    config_destroy(m_config);
    LOG_DEBUG_0("Done with ~UdfManager()");
}

/**
 * Context object for the worker function to get it's frame, UDF handle, and
 * the output queue for resulting frames.
 */
class UdfWorker {
private:
    // Profiling variable
    Profiling* m_profile;

    // UDF entry profiling key
    std::string m_udf_push_entry_key;

    // Queue blocked variable
    std::string m_udf_push_block_key;

public:
    // UDF to be ran by the worker
    // UdfHandle* handle;

    // Frame for the UDF to process
    Frame* frame;

    // Output queue for the frame (if it not dropped)
    FrameQueue* output_queue;

    // UDF pipeline to continue
    std::vector<UdfHandle*>* udfs;

    JobHandle* handle;

    /**
     * Constructor
     *
     * @param handle       - UDF handle
     * @param frame        - Frame to process
     * @param output_queue - Output queue to put processed frames into
     */
    UdfWorker(Frame* frame, std::vector<UdfHandle*>* udfs,
              Profiling* profile,
              std::string frame_push_key,
              std::string frame_block_key,
              FrameQueue* output_queue) :
        m_profile(profile), m_udf_push_entry_key(frame_push_key),
        m_udf_push_block_key(frame_block_key),
        frame(frame), output_queue(output_queue), udfs(udfs), handle(NULL)
    {};

    /**
     * Destructor
     */
    ~UdfWorker() {
        if(handle != NULL) {
            delete handle;
        }
    };

    /**
     * UDF worker run method to be executed in a thread pool. This method is
     * static so that it can be passed to the thread pool as a function
     * pointer.
     */
    static void run(void* vargs) {
        LOG_DEBUG_0("UdfWorker::run()");
        UdfWorker* ctx = (UdfWorker*) vargs;
        UdfRetCode ret;

        for(auto handle : *ctx->udfs) {
            LOG_DEBUG_0("Running UdfHandle::process()");

            if(ctx->m_profile->is_profiling_enabled()) {
                msg_envelope_t* meta_data = ctx->frame->get_meta_data();

                // Add entry timestamp
                DO_PROFILING(ctx->m_profile, meta_data, handle->get_prof_entry_key().c_str());

                ret = handle->process(ctx->frame);

                // Add exit timestamp
                DO_PROFILING(ctx->m_profile, meta_data, handle->get_prof_exit_key().c_str());
            } else {
                ret = handle->process(ctx->frame);
            }

            // TODO: Should probably have better error reporting here...
            switch(ret) {
                case UdfRetCode::UDF_DROP_FRAME:
                    LOG_DEBUG_0("Dropping frame");
                    delete ctx->frame;
                    return;
                case UdfRetCode::UDF_ERROR:
                    LOG_ERROR_0("Failed to process frame");
                    delete ctx->frame;
                    return;
                case UdfRetCode::UDF_FRAME_MODIFIED:
                case UdfRetCode::UDF_OK:
                    LOG_DEBUG_0("UDF_OK");
                    break;
                default:
                    LOG_ERROR_0("Reached default case, this should not happen");
                    delete ctx->frame;
                    return;
            }
            LOG_DEBUG_0("Done with UDF handle");
        }

        LOG_DEBUG_0("Pushing frame to output queue");
        // Add queue entry timestamp
        DO_PROFILING(ctx->m_profile, ctx->frame->get_meta_data(), ctx->m_udf_push_entry_key.c_str());
        QueueRetCode ret_queue = ctx->output_queue->push(ctx->frame);
        if(ret_queue == QueueRetCode::QUEUE_FULL) {
            if(ctx->output_queue->push_wait(ctx->frame) != QueueRetCode::SUCCESS) {
                LOG_ERROR_0("Failed to enqueue received message, "
                            "message dropped");
                delete ctx->frame;
            }
            // Add timestamp which acts as a marker if queue if blocked
            DO_PROFILING(ctx->m_profile, ctx->frame->get_meta_data(), ctx->m_udf_push_block_key.c_str());
        }
        LOG_DEBUG_0("Done running worker function");
    };
};

static void free_udf_worker(void* varg) {
    UdfWorker* ctx = (UdfWorker*) varg;
    delete ctx;
}

void UdfManager::run() {
    LOG_INFO_0("UDFManager thread started");

    // How often to check if the thread should quit
    auto duration = std::chrono::milliseconds(250);

    while(!m_stop.load()) {
        if(m_udf_input_queue->wait_for(duration)) {
            LOG_DEBUG_0("Popping frame from input queue");
            Frame* frame = m_udf_input_queue->front();
            m_udf_input_queue->pop();

	    EncodeType enc_type = frame->get_encode_type();
	    int enc_lvl = frame->get_encode_level();

            if((enc_type != m_enc_type) || (enc_lvl != m_enc_lvl)) {
                try {
                    frame->set_encoding(m_enc_type, m_enc_lvl);
                } catch(const char *err) {
                    LOG_ERROR("Exception: %s", err);
                } catch(...) {
                    LOG_ERROR("Exception occurred in set_encoding()");
                }
            }

            // Create the worker to execute the UDF pipeline on the given frame
            UdfWorker* ctx = new UdfWorker(frame, &m_udfs, m_profile, m_udf_push_entry_key,
                                           m_udf_push_block_key, m_udf_output_queue);

            LOG_DEBUG_0("Submitting job to job pool")
            JobHandle* job_handle = NULL;

            // Submit the job to run in the thread pool
            job_handle = m_pool->submit(&UdfWorker::run, ctx, free_udf_worker);
            LOG_DEBUG_0("Done submitting the job")

            // The job handle is not actually needed in this use of the
            // thread pool
            //delete job_handle;
            ctx->handle = job_handle;
        }
    }

    LOG_INFO_0("UDFManager thread stopped");
}

void UdfManager::start() {
    if(m_th == NULL && !m_stop.load()) {
        m_th = new std::thread(&UdfManager::run, this);
    } else {
        LOG_WARN_0("Start attempted after stop or after start");
    }
}

void UdfManager::stop() {
    if(m_th != NULL && !m_stop.load()) {
        m_stop.store(true);
        m_th->join();
        m_pool->stop();
    }
}
