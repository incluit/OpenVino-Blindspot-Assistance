run: 
	./blindspot-assistance -m ../../../models/FP32/pedestrian-and-vehicle-detector-adas-0001.xml -d HETERO:CPU,GPU -i ../../../data/BlindspotFront.mp4 ../../../data/BlindspotLeft.mp4 ../../../data/BlindspotRear.mp4 ../../../data/BlindspotRight.mp4 -show-stats -t 0.5

run-alerts: 
	./blindspot-assistance -m ../../../models/FP32/pedestrian-and-vehicle-detector-adas-0001.xml -d HETERO:CPU,GPU -i ../../../data/BlindspotFront.mp4 ../../../data/BlindspotLeft.mp4 ../../../data/BlindspotRear.mp4 ../../../data/BlindspotRight.mp4 -show-stats -t 0.5 -calibration -alerts

