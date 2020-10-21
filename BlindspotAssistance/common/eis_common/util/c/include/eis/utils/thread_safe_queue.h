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
 * @brief Thread safe queue implementation
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */
#ifndef _EIS_THREAD_SAFE_QUEUE_H
#define _EIS_THREAD_SAFE_QUEUE_H

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace eis {
namespace utils {

/**
 * @c ThreadSafeQueue return values
 */
enum QueueRetCode {
    SUCCESS,
    QUEUE_FULL,
    QUEUE_EMPTY,
};

/**
 * Thread-safe queue object
 *
 * The API for this queue follows a stripped down version of the STL C++
 * queue.
 */
template<typename T>
class ThreadSafeQueue {
private:
    // Underlying queue object
    std::queue<T> m_queue;

    // Mutex object
    std::mutex m_mtx;

    // Condition variable for waiting for an element to be ready
    std::condition_variable m_cv;

    // Condition variable for waiting for a free spot in the queue
    std::condition_variable m_full_cv;

    // Max size of the queue
    int m_max_size;

public:
    /**
     * Constructor
     *
     * @param max_size - Maximum queue depth, -1 or 0 for no limit
     */
    ThreadSafeQueue(int max_size) : m_max_size(max_size) {}

    /**
     * Destructor
     */
    ~ThreadSafeQueue() {
        // Clearing queue
        while (!this->empty()) {
            //T elem = this->front();
            this->pop();
            //delete elem;
        }
    }

    /**
     * Push item onto the queue.
     */
    QueueRetCode push(T value) {
        if(m_max_size > 0) {
            std::lock_guard<std::mutex> lk(m_mtx);
            size_t size = m_queue.size();
            if(size != 0 && ((size + 1) >= ((size_t) m_max_size)))
                return QueueRetCode::QUEUE_FULL;
        }

        // Block for the lock guard
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_queue.push(value);
            m_cv.notify_all();
        }

        return QueueRetCode::SUCCESS;
    }

    /**
     * Push item onto the queue. If it is full wait for a space to become
     * available to put the given value into.
     *
     * @param value - Value to enqueue
     */
    QueueRetCode push_wait(T value) {
        QueueRetCode ret;

        do {
            ret = this->push(value);
            if(ret == QueueRetCode::QUEUE_FULL) {
                std::unique_lock<std::mutex> lk(m_mtx);
                this->m_full_cv.wait(lk);
            }
        } while(ret != QueueRetCode::SUCCESS);

        return QueueRetCode::SUCCESS;
    }

    /**
     * Returns whether the queue is empty.
     *
     * @return bool
     */
    bool empty() {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_queue.empty();
    }

    /**
     * Returns the number of elements in the queue.
     *
     * @return size_t
     */
    size_t size() {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_queue.size();
    }

    /**
     * Returns reference to the next element in the queue.
     *
     * @return T
     */
    T front() {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_queue.front();
    }

    /**
     * Removes the next element in the queue reducing the size by one.
     */
    void pop() {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_queue.pop();
        m_full_cv.notify_one();
    }

    /**
     * Wait for an element to become available.
     *
     * \note This method will return immediately if the queue is not empty.
     */
    void wait() {
        if(!this->empty())
            return;

        std::unique_lock<std::mutex> lk(m_mtx);
        m_cv.wait(lk);
    }

    /**
     * Wait for an element to be available in the queue or until the given
     * amount of time has passed.
     *
     * \note This method will return immediately if the queue is not empty.
     *
     * @param duration - Duration to wait
     * @return True if element exists, False if timeout occurred
     */
    template<class Rep, class Period>
    bool wait_for(std::chrono::duration<Rep, Period>& duration) {
        if(!this->empty())
            return true;

        std::unique_lock<std::mutex> lk(m_mtx);
        if(m_cv.wait_for(lk, duration) == std::cv_status::timeout) {
            return false;
        } else {
            return true;
        }
    }
};

} // utils
} // eis

#endif // _EIS_THREAD_SAFE_QUEUE_H
