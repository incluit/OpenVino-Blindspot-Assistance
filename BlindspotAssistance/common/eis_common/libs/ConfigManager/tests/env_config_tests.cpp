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

#include <gtest/gtest.h>
#include "eis/config_manager/env_config.h"
#include "eis/config_manager/config_manager.h"
#include "eis/utils/logger.h"

void test_setup() {
	set_log_level(LOG_LVL_DEBUG);
	//clear all envs
	int ret;
	const char* env[] = {"DEV_MODE", "AppName", "PubTopics", "SubTopics"};
	for(int i = 0; i < sizeof(env)/sizeof(env[0]); i++) {
		ret = unsetenv(env[i]);
		if (ret != 0) {
			LOG_ERROR("%s env unset failed", env[i]);
		}
	}
}

TEST(env_config_tests, get_topics_from_env) {
	test_setup();

	env_config_t* env_config = env_config_new();
	char** results = NULL;

	LOG_INFO_0("========1. Test absence of SubTopics env========");
	results = env_config->get_topics_from_env("sub");
	ASSERT_TRUE(results == NULL);

	LOG_INFO_0("========2. Test absence of PubTopics env========");
	results = env_config->get_topics_from_env("pub");
	ASSERT_TRUE(results == NULL);

	LOG_INFO_0("========3. Test with SubTopics/PubTopics env multiple times========");
	const char* topics_list = "sub_topic1,sub_topic2";
	std::vector<std::string> topics{"sub_topic1", "sub_topic2"};

	std::map<std::string, std::string> topics_map = {
		{"pub", "PubTopics"},
		{"sub", "SubTopics"},
	};
	for(auto& ele : topics_map) {
		setenv((char*)&ele.second[0], topics_list, true);
		for(int i = 0; i < 4; i++) {
			results = env_config->get_topics_from_env((char*)&ele.first[0]);
			LOG_INFO("%s : %s",results[0], &topics[0][0]);
			LOG_INFO("%s : %s",results[1], &topics[1][0]);
			EXPECT_EQ(topics[0], results[0]);
			EXPECT_EQ(topics[1], results[1]);
		}
	}

	LOG_INFO_0("========4. Test with wrong topic type========");
	results = env_config->get_topics_from_env("res");
	ASSERT_TRUE(results == NULL);
}

TEST(env_config_tests, get_messagebus_config) {
	test_setup();

	const char* topic_type = "sub";
	config_t* config = NULL;

	std::string topic = "camera1_stream_results";
	std::string result = topic + "_cfg";
	std::string sub_topic = "VideoIngestion/" + topic;
	char* c_sub_topics = new char[sub_topic.length() + 1];
	strcpy(c_sub_topics, sub_topic.c_str());

	std::string wrong_sub_topic = "VideoIngestion##" + topic;
	char* c_wrong_sub_topic = new char[wrong_sub_topic.length() + 1];
	strcpy(c_wrong_sub_topic, wrong_sub_topic.c_str());

	size_t num_of_sub_topics = 1;

	unsetenv(&result[0]);

	env_config_t* env_config = env_config_new();

	LOG_INFO_0("========1. Test absence of DEV_MODE env========");
	config = env_config->get_messagebus_config(NULL, &c_sub_topics, num_of_sub_topics, topic_type); 
	ASSERT_TRUE(config == NULL);

	LOG_INFO_0("========2. Test absence of AppName env========");
	setenv("DEV_MODE", "true", true);
	config = env_config->get_messagebus_config(NULL, &c_sub_topics, num_of_sub_topics, topic_type);
	ASSERT_TRUE(config == NULL);

	LOG_INFO_0("========3. Send wrong topic type========");
	setenv("AppName", "Sample", true);
	config = env_config->get_messagebus_config(NULL, &c_sub_topics, num_of_sub_topics, "");
	ASSERT_TRUE(config == NULL);

	LOG_INFO_0("========4. Test absence of [topic]_cfg env========");
	setenv("SubTopics", sub_topic.c_str(), true);
    config = env_config->get_messagebus_config(NULL, &c_sub_topics, num_of_sub_topics, topic_type);
	ASSERT_TRUE(config == NULL);

	LOG_INFO_0("========5. Test sending the wrong format of sub topic========");
	setenv((char*) &result[0], "zmq_tcp,127.0.0.1:65013", true);
	LOG_INFO("Topic: %s, topic cfg: %s", &topic[0], &result[0]);
	config = env_config->get_messagebus_config(NULL, &c_wrong_sub_topic, num_of_sub_topics, topic_type);

	ASSERT_TRUE(config == NULL);

	LOG_INFO_0("========6. Test the actual flow of api multiple times========");
	LOG_INFO("Sub topic: %s", sub_topic.c_str());
	config_value_t* value =NULL;
	for(int i = 0; i < 4; i++) {
		config = env_config->get_messagebus_config(NULL, &c_sub_topics, num_of_sub_topics, topic_type);
		value = config->get_config_value(config->cfg, "type");
		ASSERT_EQ(value->type, CVT_STRING);
		ASSERT_STRCASEEQ(value->body.string, "zmq_tcp");
	}
	
	topic_type = "server";
	char* Server_name[]= {"SampleServer"};

	LOG_INFO_0("========7. Test the api for server configuration ========");
	setenv("Server", "zmq_tcp,127.0.0.1:5669", true);
	config = env_config->get_messagebus_config(NULL, Server_name, 1, topic_type);
	value = config->get_config_value(config->cfg, "type");
	ASSERT_EQ(value->type, CVT_STRING);
	ASSERT_STRCASEEQ(value->body.string, "zmq_tcp");

	topic_type = "client";
	LOG_INFO_0("========8. Test the api for client configuration ========");
	char* RequestEP[] = {"SampleEP"};
	setenv("SampleEP_cfg", "zmq_tcp,127.0.0.1:5669", true);
	config = env_config->get_messagebus_config(NULL, RequestEP, 1, topic_type);
	value = config->get_config_value(config->cfg, "type");
	ASSERT_EQ(value->type, CVT_STRING);
	ASSERT_STRCASEEQ(value->body.string, "zmq_tcp");

	free(value);
	free(config);

	delete [] c_sub_topics;
	delete [] c_wrong_sub_topic;
	// TODO: Enable extensive tests to thoroughly test all possible scenarios
	// for both dev and prod mode
}

TEST(env_config_tests, get_topics_count) {
	test_setup();
	env_config_t* env_config = env_config_new();

	LOG_INFO_0("========1. Test if the number of topics are returned properly by API========");

	char* topics[4] = {"camera1_stream", "camera2_stream", "camera3_stream", NULL};
	size_t len = 3; // is equal to the number of topics in the string array topics

	size_t num_of_topics = env_config->get_topics_count(topics);

	LOG_INFO("num of topics = %d", num_of_topics);

	ASSERT_TRUE(num_of_topics == len);

}