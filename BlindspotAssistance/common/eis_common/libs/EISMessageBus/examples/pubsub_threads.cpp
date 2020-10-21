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
 * @brief EIS Message Bus example using the C++ thread helper classes.
 * @author Kevin Midkiff <kevin.midkiff@intel.com>
 */

#include <chrono>
#include <cstring>
#include <csignal>
#include <condition_variable>

#include <eis/utils/logger.h>
#include <eis/utils/json_config.h>
#include "eis/msgbus/msgbus.h"

#define SERVICE_NAME "pubsub-threads"

using namespace eis::utils;
using namespace eis::msgbus;

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
     * Destructur
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

// Globals
Publisher* g_publisher = NULL;
Subscriber<ExampleMessage>* g_subscriber = NULL;
MessageQueue* g_input_queue = NULL;
MessageQueue* g_output_queue = NULL;

/**
 * Function to print the usage of the application.
 */
void usage(const char* name) {
    fprintf(stderr, "usage: %s [-h|--help] <json-config>\n", name);
    fprintf(stderr, "\t-h|--help   - Show this help\n");
    fprintf(stderr, "\tjson-config - Path to JSON configuration file\n");
}

/**
 * Signal handler
 */
void signal_handler(int signo) {
    LOG_INFO_0("Quitting...");
    if(g_publisher != NULL)     delete g_publisher;
    if(g_subscriber != NULL)    delete g_subscriber;
    if(g_input_queue != NULL)   delete g_input_queue;
    if(g_output_queue != NULL)  delete g_output_queue;
}

int main(int argc, char** argv) {
    if(argc == 1) {
        LOG_ERROR_0("Too few arguments");
        return -1;
    } else if(argc > 2) {
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

    config_t* pub_config = json_config_new(argv[1]);
    if(pub_config == NULL) {
        LOG_ERROR_0("Failed to load JSON configuration");
        return -1;
    }
    config_t* sub_config = json_config_new(argv[1]);
    if(pub_config == NULL) {
        LOG_ERROR_0("Failed to load JSON configuration");
        config_destroy(pub_config);
        return -1;
    }

    set_log_level(LOG_LVL_INFO);

    LOG_INFO_0("Initializing publisher/subscriber");

    std::condition_variable err_cv;

    g_input_queue = new MessageQueue(-1);
    g_output_queue = new MessageQueue(-1);
    g_publisher = new Publisher(
            pub_config, err_cv, "PUBSUB_TOPIC", g_input_queue,
            SERVICE_NAME);
    g_subscriber = new Subscriber<ExampleMessage>(
            sub_config, err_cv, "PUBSUB_TOPIC", g_output_queue,
            SERVICE_NAME);

    g_publisher->start();
    g_subscriber->start();

    // Give time to initialize publisher and subscriber
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    char* msg = new char[14];
    memcpy(msg, "Hello, World!", 14);
    msg[13] = '\0';

    ExampleMessage* wrap = new ExampleMessage(msg);

    LOG_INFO_0("Enquing message to send");
    g_input_queue->push(wrap);

    LOG_INFO_0("Waiting to receive the message");
    g_output_queue->wait();

    ExampleMessage* received = (ExampleMessage*) g_output_queue->front();
    LOG_INFO("Received: %s", received->get_message());

    delete received;
    delete g_publisher;
    delete g_subscriber;
    delete g_input_queue;
    delete g_output_queue;

    return 0;
}
