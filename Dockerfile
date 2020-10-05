FROM openvino/ubuntu18_dev:2020.3

WORKDIR /app

USER root

RUN apt-get update
RUN apt-get -y upgrade && apt-get autoremove

RUN apt-get install -y --no-install-recommends \
        build-essential \
        git \
        gcc \
        make \
        wget \
        vim \
        libssl-dev \
        sudo \
        gnutls-dev pkg-config

RUN wget https://cmake.org/files/v3.15/cmake-3.15.0-Linux-x86_64.sh
RUN mkdir /opt/cmake
RUN bash cmake-3.15.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
RUN update-alternatives --install /usr/bin/cmake cmake /opt/cmake/bin/cmake 1 --force
#ZMQ
# Install ZQM lib
WORKDIR /root
RUN git clone https://github.com/zeromq/libzmq
WORKDIR /root/libzmq/build
RUN cmake ..
RUN make -j4 install
# Install cppzmq
WORKDIR /root
RUN git clone https://github.com/zeromq/cppzmq
WORKDIR /root/cppzmq/build
RUN cmake ..
RUN make -j4 install

# EIS message bus
ADD . /app
WORKDIR /app/BlindspotAssistance/common/eis_common
RUN bash /app/BlindspotAssistance/common/eis_common/eis_libs_installer.sh
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/

# Blindspot
WORKDIR /app/BlindspotAssistance
RUN mkdir -p build
WORKDIR /app/BlindspotAssistance/build
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && cmake -DCMAKE_BUILD_TYPE=Release -MULTICHANNEL_DEMO_USE_TBB=ON ../ && make'
COPY BlindspotAssistance/Makefile /app/BlindspotAssistance/build/intel64/Release
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash ../scripts/download_models.sh'

# Initial configuration
RUN lsb_release -a
WORKDIR /app/BlindspotAssistance/build/intel64/Release
CMD ["/bin/bash"]
