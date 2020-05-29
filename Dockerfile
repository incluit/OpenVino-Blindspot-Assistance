FROM openvino/ubuntu18_dev:2020.2

ADD . /app
WORKDIR /app

USER root
RUN apt-get update && apt-get -y upgrade && apt-get autoremove
#####Comentado para reducir tiempo de build

RUN apt-get install -y --no-install-recommends \
        build-essential \
        git \
        gcc \
        make \
        cmake \
        cmake-gui\
        cmake-curses-gui \
        libssl-dev \
        sudo

RUN /bin/bash -c 'git clone https://github.com/eclipse/paho.mqtt.c.git && cd paho.mqtt.c && git checkout v1.3.1 && cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF && sudo cmake --build build/ --target install && sudo ldconfig'

COPY BlindspotAssistance/* app/
WORKDIR /app/BlindspotAssistance
RUN mkdir build
WORKDIR /app/BlindspotAssistance/build
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && cmake -DCMAKE_BUILD_TYPE=Release -MULTICHANNEL_DEMO_USE_TBB=ON ../ && make'

COPY BlindspotAssistance/Makefile /app/BlindspotAssistance/build/intel64/Release
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash ../scripts/download_models.sh'

WORKDIR /app/BlindspotAssistance/build/intel64/Release
CMD ["/bin/bash"]
