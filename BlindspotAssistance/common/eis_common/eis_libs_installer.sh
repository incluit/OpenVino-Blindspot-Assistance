#!/bin/bash -e

export CUR_DIR=$PWD

service_exists () {
    type "$1" &> /dev/null ;
}

EISMessageBus="$CUR_DIR/libs/EISMessageBus"
ConfigManager="$CUR_DIR/libs/ConfigManager"
CMAKE_BUILD_TYPE="Release"
RUN_TESTS="ON"

# Install EISMessageBus requirements
cd $EISMessageBus &&
   rm -rf deps && \
   ./install.sh --cython

cd $EISMessageBus/../IntelSafeString/ &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
   make install

cd $EISMessageBus/../EISMsgEnv/ &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake  -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests && \
   ./msg-envelope-tests  && \
   ./crc32-tests  && \
   cd .. ; fi && \
   make install

cd $EISMessageBus/../../util/c/ &&
   ./install.sh && \
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DWITH_TESTS=${RUN_TESTS} -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests  && \
   ./config-tests
   ./log-tests
   ./thp-tests
   ./tsp-tests
   cd .. ; fi  && \
   make install

cd $ConfigManager &&
   rm -rf build && \
   mkdir build && \
   cd build && \
   cmake -DWITH_GO_ENV_CONFIG=ON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .. && \
   cmake -DWITH_TESTS=${RUN_TESTS} -DWITH_PYTHON=ON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} .. && \
   make && \
   if [ "${RUN_TESTS}" = "ON" ] ; then cd ./tests && \
   ./env-config-tests && \
   cd .. ; fi && \
   make install

# Installing EISMessageBus C++ from DEB package
cd $EISMessageBus &&
   apt install ./eis-messagebus-2.3.0-Linux.deb
