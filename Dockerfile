FROM openvino/ubuntu18_dev:2020.2

ADD . /app
WORKDIR /app

USER root
##RUN apt-get update && apt-get -y upgrade && apt-get autoremove

COPY BlindspotAssistance/* app/
WORKDIR /app/BlindspotAssistance
RUN mkdir build
WORKDIR /app/BlindspotAssistance/build
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && cmake -DCMAKE_BUILD_TYPE=Release ../ && make'

COPY BlindspotAssistance/Makefile app/BlindspotAssistance/build/intel64/Release
##RUN pip3 install -r requirements.txt
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash ../scripts/download_models.sh'

WORKDIR /app/BlindspotAssistance/build/intel64/Release
CMD ["/bin/bash"]


### docker build -t blindspot-assistance . --rm
### xhost +
### docker run --net=host --env="DISPLAY" -it --device /dev/dri:/dev/dri --device-cgroup-rule='c 189:* rmw' -v /dev/bus/usb:/dev/bus/usb --device=/dev/video0 --volume="$HOME/.Xauthority:/root/.Xauthority:rw" blindspot-assistance /bin/bash

<<<<<<< HEAD
### ./blindspot-assistance -m ../../../models/FP16/pedestrian-and-vehicle-detector-adas-0001.xml -d CPU -i ../../../data/blindspot1.mp4 ../../../data/blindspot3.mp4 ../../../data/blindspot4.mp4 ../../../data/blindspot2.mp4 -show-stats -t 0.5
=======
### Run into the docker (With YOLO v3)
# ./multi_channel_object_detection_demo_yolov3 -m ../../../models/frozen_darknet_yolov3_model.xml -d CPU -nc 1
>>>>>>> 2559f51c43b22ff2c628e8c7c9e05db58fa563de
