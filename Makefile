docker-build:
	docker build -t blindspot-incluit .

docker-run: 
	xhost +
	docker run --name blindspotcont --net=host --env="DISPLAY" -it --device /dev/dri:/dev/dri --device-cgroup-rule='c 189:* rmw' -v /dev/bus/usb:/dev/bus/usb --device=/dev/video0 --volume="$$HOME/.Xauthority:/root/.Xauthority:rw" blindspot-incluit /bin/bash

docker-remove:
	docker rm -f blindspotcont

