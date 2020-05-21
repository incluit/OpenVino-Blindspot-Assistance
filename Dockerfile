FROM openvino/ubuntu18_dev:2020.2

ADD . /app
WORKDIR /app

USER root
#RUN apt-get update && apt-get -y upgrade && apt-get autoremove  
#####Comentado para reducir tiempo de build

COPY BlindspotAssistance/* app/
WORKDIR /app/BlindspotAssistance
RUN mkdir build
WORKDIR /app/BlindspotAssistance/build
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && cmake -DCMAKE_BUILD_TYPE=Release ../ && make'

COPY BlindspotAssistance/Makefile /app/BlindspotAssistance/build/intel64/Release
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash ../scripts/download_models.sh'

WORKDIR /app/BlindspotAssistance/build/intel64/Release
CMD ["/bin/bash"]
