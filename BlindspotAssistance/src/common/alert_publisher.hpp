#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <string>

// EIS includes
#include <cstring>
#include <csignal>
#include <condition_variable>

#include <eis/utils/logger.h>
#include <eis/utils/json_config.h>
#include "eis/msgbus/msgbus.h"

#define TOPIC "BLAS"
#define SERVICE_NAME "pubsub-threads"
//////////////////////////

using namespace eis::utils;
using namespace eis::msgbus;

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

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