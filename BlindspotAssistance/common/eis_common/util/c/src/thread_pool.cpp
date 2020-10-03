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
 * @brief @c ThreadPool implementation
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include "eis/utils/thread_pool.h"
#include "eis/utils/logger.h"

using namespace eis::utils;

//
// ThreadPool implementation
//

ThreadPool::ThreadPool(int max_threads, int max_jobs) :
    m_max_threads(max_threads), m_funcs(max_jobs), m_stop(false)
{}

ThreadPool::~ThreadPool() {
    this->stop();

    // Clear any queued job which did not run
    while(!m_funcs.empty()) {
        Func* func = m_funcs.front();
        m_funcs.pop();
        delete func;
    }
}

void ThreadPool::stop() {
    if(m_stop.load()) {
        return;
    }

    // Set stop flag
    m_stop.store(true);

    // Wait for all threads to join
    for(auto th : m_threads) {
        th->join();
        delete th;
    }
}

JobHandle* ThreadPool::submit(
        void(*fn)(void*), void* vargs, void(*free_fn)(void*)) {
    if(m_stop.load()) {
        LOG_ERROR_0("Job submitted after thread pool stopped");
        return NULL;
    }

    JobHandle* handle = new JobHandle();
    Func* func = new Func(fn, vargs, free_fn, handle);

    // Put the function into the job queue, if it is full block indefinitely
    // until the job can be pushed
    m_funcs.push_wait(func);

    if(((int) m_threads.size()) < m_max_threads) {
        LOG_DEBUG_0("Launching new thread in thread pool");
        std::lock_guard<std::mutex> lk(m_mtx);
        // Start a new thread for the execution of the function
        std::thread* th = new std::thread(&ThreadPool::run, this);
        m_threads.push_back(th);
    }

    return handle;
}

void ThreadPool::run() {
    LOG_DEBUG_0("New thread pool thread started");

    // Amount of time to wait for a new function to execute, if no new jobs
    // then check if we should stop
    auto duration = std::chrono::milliseconds(500);

    while(!m_stop.load()) {
        if(m_funcs.wait_for(duration)) {
            // Get the function
            Func* func = NULL;

            // NOTE: Creating separate block for the lock guard
            {
                // Locking here and checking if empty, because the job could
                // have been taken by another waiting thread
                std::lock_guard<std::mutex> lk(m_job_mtx);
                if(m_funcs.empty())
                    continue;
                func = m_funcs.front();
                m_funcs.pop();
            }

            // Verify that a job was actually picked up
            if(func == NULL) {
                LOG_ERROR_0("This should not have happend, no available job");
                continue;
            }

            try {
                // Calling function
                func->call();
            } catch(const std::exception& e) {
                LOG_ERROR("Error calling function: %s", e.what());
            }
            delete func;
        }
    }

    LOG_DEBUG_0("Thread pool thread stopped");
}

//
// JobHandle implementation
//

JobHandle::JobHandle() : m_completed(false) {}

JobHandle::~JobHandle() {}

void JobHandle::wait() {
    if(m_completed.load()) {
        // Job has already completed
        return;
    }

    // The job has not yet completed, wait for it to complete
    std::unique_lock<std::mutex> lk(m_mtx);
    m_cv.wait(lk);
}

void JobHandle::completed() {
    m_completed.store(true);
    m_cv.notify_all();
}

//
// Func implementation
//

Func::Func(void(*fn)(void*), void* vargs, void(*free_fn)(void*),
           JobHandle* handle) :
    m_vargs(vargs), m_fn(fn), m_free(free_fn), m_handle(handle)
{}

Func::~Func() {
    if(m_free != NULL) {
        m_free(m_vargs);
    }
}

void Func::call() {
    // Call the function
    m_fn(m_vargs);

    // Mark the job handle for the function as completed
    m_handle->completed();
}
