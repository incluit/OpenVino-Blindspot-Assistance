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
 * @brief EIS Message Bus example using the C++ thread helper classes.
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#include <chrono>
#include <cstring>
#include <csignal>
#include <atomic>
#include <condition_variable>

#include <eis/utils/logger.h>
#include <eis/utils/json_config.h>
#include "eis/msgbus/msgbus.h"

#define TOPIC "BLAS"
#define SERVICE_NAME "sub-thread-example"

using namespace eis::utils;
using namespace eis::msgbus;
using namespace std;
// Globals
std::atomic<bool> g_stop;

/**
 * Example object which is both serializable and deserialzable
 */
class ExampleMessage : public Serializable {
private:
    // Internal string message
    char* m_message;

public:
    /**
     * Default constructor.
     *
     * @param message - Message for the example message
     */
    ExampleMessage(char* message) :
        Serializable(NULL)
    {
        m_message = message;
    };

    /**
     * Deserializable constructor.
     *
     * @param msg - Message Envelope
     */
    ExampleMessage(msg_envelope_t* msg) :
        Serializable(NULL)
    {
        // Retrieve data out of the message envelope
        msg_envelope_elem_body_t* body = NULL;
        msgbus_ret_t ret = msgbus_msg_envelope_get(msg, "message", &body);
        if(ret != MSG_SUCCESS) {
            throw "Failed to retrieve \"message\" key from envelope";
        }
        if(body->type != MSG_ENV_DT_STRING) {
            msgbus_msg_envelope_elem_destroy(body);
            throw "\"message\" value must be a string";
        }

        // Copy over the string
        size_t len = strlen(body->body.string);
        m_message = new char[len + 1];
        memcpy(m_message, body->body.string, len);
        m_message[len] = '\0';
        msgbus_msg_envelope_destroy(msg);
    };

    /**
     * Destructor
     */
    ~ExampleMessage() {
        delete[] m_message;
    };

    /**
     * Overridden serialize method
     *
     * @return @c msg_envelope_t*
     */
    msg_envelope_t* serialize() override {
        msg_envelope_t* msg = msgbus_msg_envelope_new(CT_JSON);
        if(msg == NULL) {
            LOG_ERROR_0("Failed to initialize message");
            return NULL;
        }

        msg_envelope_elem_body_t* body = msgbus_msg_envelope_new_string(
                m_message);
        if(body == NULL) {
            LOG_ERROR_0("Failed to initialize message envelope body");
            msgbus_msg_envelope_destroy(msg);
            return NULL;
        }

        msgbus_ret_t ret = msgbus_msg_envelope_put(msg, "message", body);
        if(ret != MSG_SUCCESS) {
            LOG_ERROR_0("Failed to put \"message\" key into envelope");
            msgbus_msg_envelope_elem_destroy(body);
            msgbus_msg_envelope_destroy(msg);
            return NULL;
        }

        return msg;
    };

    /**
     * Get the message.
     *
     * @return char*
     */
    const char* get_message() {
        return m_message;
    };
};

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Quitting...");
    g_stop.store(true);
}

/**
 * Function to print publisher usage
 */
void usage(const char* name) {
    fprintf(stderr, "usage: %s [-h|--help] <json-config> [topic]\n", name);
    fprintf(stderr, "\t-h|--help   - Show this help\n");
    fprintf(stderr, "\tjson-config - Path to JSON configuration file\n");
    fprintf(stderr, "\ttopic       - (Optional) Topic string "\
                    "(df: publish_test)\n");
}

int main(int argc, char** argv) {
    if(argc == 1) {
        LOG_ERROR_0("Too few arguments");
        return -1;
    } else if(argc > 3) {
        LOG_ERROR_0("Too many arguments");
        return -1;
    }

    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        usage(argv[0]);
        return 0;
    }

    // Setting up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Set log level
    set_log_level(LOG_LVL_DEBUG);

    config_t* sub_config = json_config_new(argv[1]);
    if(sub_config == NULL) {
        LOG_ERROR_0("Failed to load JSON configuration");
        config_destroy(sub_config);
        return -1;
    }

    LOG_INFO_0("Initializing publisher/subscriber");

    std::condition_variable err_cv;

    MessageQueue* output_queue = new MessageQueue(-1);
    Subscriber<ExampleMessage>* subscriber = NULL;

    if(argc == 3) {
        subscriber = new Subscriber<ExampleMessage>(
                sub_config, err_cv, argv[2], output_queue, SERVICE_NAME);
    } else {
        subscriber = new Subscriber<ExampleMessage>(
                sub_config, err_cv, TOPIC, output_queue, SERVICE_NAME);
    }

    subscriber->start();

    auto timeout = std::chrono::milliseconds(250);
    while(!g_stop.load()) {
        if(output_queue->wait_for(timeout)) {
            LOG_INFO_0("Received message");
            ExampleMessage* received = (ExampleMessage*) output_queue->front();
            LOG_INFO("Received: %s", received->get_message());
            output_queue->pop();
            delete received;
        }
    }

    delete subscriber;
    delete output_queue;

    return 0;
}
