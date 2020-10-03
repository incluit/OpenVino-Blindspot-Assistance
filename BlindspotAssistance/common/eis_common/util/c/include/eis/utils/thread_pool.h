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
 * @brief Thread pool
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */
#ifndef _EIS_THREAD_POOL_QUEUE_H
#define _EIS_THREAD_POOL_QUEUE_H

#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

#include "eis/utils/thread_safe_queue.h"

namespace eis {
namespace utils {

// Forward declaration
class Func;

/**
 * Handle to a submitted job.
 */
class JobHandle {
private:
    // Condition variable for waiting
    std::condition_variable m_cv;

    // Flag for if the job has completed
    std::atomic<bool> m_completed;

    // Mutex for condition variable
    std::mutex m_mtx;

protected:
    /**
     * Notify the submitter of the job that it has completed.
     */
    void completed();

    // Making Func class a friend so it can access the completed() method
    friend Func;

public:
    /**
     * Constructor.
     */
    JobHandle();

    /**
     * Destructor
     */
    ~JobHandle();

    /**
     * Wait for the job to complete.
     */
    void wait();

    /**
     * Wait for the specified amount of time for the job to complete.
     *
     * @param duration - Duration to wait
     * @return True if the job has completed in the specified time, otherwise
     *         False
     */
    template<class Rep, class Period>
    bool wait_for(std::chrono::duration<Rep, Period>& duration) {
        if(m_completed.load()) {
            // The job has already completed
            return true;
        }

        // The job has not yet completed, wait for the specified duration to
        // see if the job completes in that time
        std::unique_lock<std::mutex> lk(m_mtx);
        if(m_cv.wait_for(lk, duration) == std::cv_status::timeout) {
            return false;
        } else {
            return true;
        }
    };
};

/**
 * Function to be called by the thread pool
 */
class Func {
private:
    // Arguments to pass to the method
    void* m_vargs;

    // Function to be called
    void(*m_fn)(void*);

    // Free function for the m_vargs
    void(*m_free)(void*);

    // Handle to the submitter for the job
    JobHandle* m_handle;

public:
    /**
     * Constructor.
     *
     * @param fn      - Function pointer to call
     * @param vargs   - Argument to pass to the function
     * @param free_fn - Free function for vargs
     * @param handle  - Handle to the job for the function
     */
    Func(void(*fn)(void*), void* vargs, void(*free_fn)(void*),
         JobHandle* handle);

    /**
     * Destructor.
     */
    ~Func();

    /**
     * Call the function.
     */
    void call();
};


/**
 * Thread Pool
 */
class ThreadPool {
private:
    // Maximum number of threads
    int m_max_threads;

    // Functions to be called
    ThreadSafeQueue<Func*> m_funcs;

    // Some locks
    std::mutex m_mtx;
    std::mutex m_job_mtx;

    // Threads
    std::vector<std::thread*> m_threads;

    // Flag for stopping the thread pool
    std::atomic<bool> m_stop;

    // Thread run method
    void run();

public:
    /**
     * Constructor.
     *
     * @param max_threads - Maximum number of threads to spawn
     * @param max_jobs    - Maximum number of jobs to allow to be queued
     *                      -1 or 0 for no limit.
     */
    ThreadPool(int max_threads, int max_jobs);

    /**
     * Destructor.
     */
    ~ThreadPool();

    /**
     * Stop the thread pool and do not allow any new jobs to be submitted.
     *
     * \note This will stop the thread pool, however, all threads currently
     *      executing a job will complete their job first, so this method may
     *      take some time to return.
     */
    void stop();

    /**
     * Submit a job to the thread pool.
     *
     * NULL will be returned if the thread pool is full, the thread pool has
     * been stopped or if the some other issue occurs.
     *
     * @param fn      - Function pointer
     * @param vargs   - Argument to pass to the function
     * @param freE_fn - vargs free function
     * @return @c JobHandle, NULL if an issue occurs
     */
    JobHandle* submit(void(*fn)(void*), void* vargs, void(*free_fn)(void*));
};

} // utils
} // eis

#endif //_EIS_THREAD_POOL_QUEUE_H
