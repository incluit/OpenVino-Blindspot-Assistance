# Copyright (c) 2019 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# Configure and copy over IPC configuration file
set(IPC_CONFIG_IN "${CMAKE_CURRENT_SOURCE_DIR}/configs/ipc_example_config.json.in")
set(IPC_CONFIG    "${CMAKE_CURRENT_BINARY_DIR}/configs/ipc_example_config.json")
set(IPC_CONFIG_MULTI_IN "${CMAKE_CURRENT_SOURCE_DIR}/configs/ipc_example_config_multi_topics.json.in")
set(IPC_CONFIG_MULTI    "${CMAKE_CURRENT_BINARY_DIR}/configs/ipc_example_config_multi_topics.json")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.socks")
configure_file(${IPC_CONFIG_IN} ${IPC_CONFIG})
configure_file(${IPC_CONFIG_MULTI_IN} ${IPC_CONFIG_MULTI})

# Copy over TCP configuration files
file(GLOB CONFIGS "configs/*.json")

foreach(CONFIG ${CONFIGS})
    file(COPY "${CONFIG}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/configs")
endforeach()

# Copy over configuration tool
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/gen_tcp_sub_conf.py"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Add publisher example
add_executable(publisher "publisher.c")
target_link_libraries(publisher eismsgbus
                                eisutils
                                eismsgenv
                                pthread)

# Add subscriber example
add_executable(subscriber "subscriber.c")
target_link_libraries(subscriber eismsgbus
                                 eisutils
                                 eismsgenv
                                 pthread)

# Add echo service example
add_executable(echo-service "echo_service.c")
target_link_libraries(echo-service eismsgbus
                                   eisutils
                                   eismsgenv
                                   pthread)

# Add echo service client example
add_executable(echo-client "echo_client.c")
target_link_libraries(echo-client eismsgbus
                                  eisutils
                                  eismsgenv
                                  pthread)

# Add publisher many example
add_executable(publisher-many "publisher_many.c")
target_link_libraries(publisher-many eismsgbus
                                     eisutils
                                     eismsgenv
                                     pthread)

# Add pubsub c++ example
add_executable(pubsub-threads "pubsub_threads.cpp")
target_link_libraries(pubsub-threads eismsgbus
                                     eisutils
                                     eismsgenv
                                     pthread)

# Add subscriber thread C++ example
add_executable(thread-subscriber "subscriber_thread.cpp")
target_link_libraries(thread-subscriber eismsgbus
                                        eisutils
                                        eismsgenv
                                        pthread)
