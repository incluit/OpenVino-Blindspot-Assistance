FROM openvino/ubuntu18_dev:2020.3

USER root
WORKDIR /app

RUN apt-get update && apt-get -y upgrade && apt-get autoremove

RUN apt-get install -y --no-install-recommends \
        build-essential \
        git \
        gcc \
        make \
        cmake \
        cmake-gui\
        cmake-curses-gui \
        libssl-dev \
        sudo \
        gnutls-dev pkg-config

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

# Ros2
RUN apt-get install locales
RUN locale-gen en_US en_US.UTF-8
RUN update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
RUN export LANG=en_US.UTF-8
RUN curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
RUN sh -c 'echo "deb http://packages.ros.org/ros2/ubuntu `lsb_release -cs` main" > /etc/apt/sources.list.d/ros2-latest.list'
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ros-crystal-ros-base
RUN apt-get update && apt-get install -y --no-install-recommends\
        python3-colcon-common-extensions \
        ros-crystal-rosbag2-test-common \
        ros-crystal-rosbag2-storage-default-plugins \
        ros-crystal-rosbag2-storage
RUN apt-get install -y --no-install-recommends \
        ros-crystal-sqlite3-vendor \
        ros-crystal-ros2bag*

# ETS-ROS2
WORKDIR /app/BlindspotAssistance/
RUN git clone https://github.com/HernanG234/ets_ros2/
WORKDIR /opt/intel/openvino/deployment_tools/open_model_zoo/tools/downloader
RUN /bin/bash -c 'python3 -mpip install --user -r ./requirements.in'

ADD . /app
# Build ROS2 environment
WORKDIR /app/BlindspotAssistance/ets_ros2
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && source /opt/ros/crystal/setup.bash && colcon build --symlink-instal --parallel-workers 1 --cmake-args -DSIMULATOR=ON -DBUILD_DEPS=ON'
# Build Blindspot
WORKDIR /app/BlindspotAssistance/build
RUN /bin/bash -c 'source /opt/ros/crystal/setup.bash && source /app/BlindspotAssistance/ets_ros2/install/setup.bash && source /opt/intel/openvino/bin/setupvars.sh && source /app/BlindspotAssistance/scripts/setupenv.sh && cmake -DCMAKE_BUILD_TYPE=Release -DSIMULATOR=ON -DBUILD_DEPS=ON ../ && make'
# Download models
RUN /bin/bash -c 'source /opt/ros/crystal/setup.bash && source /app/BlindspotAssistance/ets_ros2/install/setup.bash && source /opt/intel/openvino/bin/setupvars.sh && source /app/BlindspotAssistance/scripts/download_models.sh'
# Set Makefile
COPY BlindspotAssistance/Makefile /app/BlindspotAssistance/build/intel64/Release

WORKDIR /app/BlindspotAssistance/build/intel64/Release
CMD ["/bin/bash"]