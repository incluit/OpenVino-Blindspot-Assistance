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

##RUN pip3 install -r requirements.txt
##RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash BlindspotAssistance/scripts/download_models.sh'

CMD ["/bin/bash"]


### docker build -t blindspot-incluit .

### docker run --net=host --env="DISPLAY" -it --device /dev/dri:/dev/dri --device-cgroup-rule='c 189:* rmw' -v /dev/bus/usb:/dev/bus/usb --device=/dev/video0 --volume="$HOME/.Xauthority:/root/.Xauthority:rw" blindspot-incluit /bin/bash

