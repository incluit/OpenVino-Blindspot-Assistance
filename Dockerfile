FROM openvino/ubuntu18_dev:2020.2

ADD . /app
WORKDIR /app

USER root
RUN apt-get update && apt-get -y upgrade && apt-get autoremove

RUN pip3 install -r requirements.txt
RUN /bin/bash -c 'source /opt/intel/openvino/bin/setupvars.sh && bash scripts/download_models.sh'

CMD ["/bin/bash"]
