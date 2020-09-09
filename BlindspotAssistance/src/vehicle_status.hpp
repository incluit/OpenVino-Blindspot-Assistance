#include <iostream>
#include <string>
#include <memory>
#include "classes.hpp"
#include <boost/circular_buffer.hpp>
#include "std_msgs/msg/string.hpp"
#include "rclcpp/rclcpp.hpp"
//#include "ets_msgs/msg/truck.hpp"
/////////////////

#include <gflags/gflags.h>
#include <functional>
#include <iostream>
#include <ctime>
#include <fstream>
#include <random>
#include <memory>
#include <chrono>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <iterator>
#include <map>
#include <thread>
#include <mutex>
#include <math.h>
#include <sys/types.h>
#include <signal.h>

#include <inference_engine.hpp>

#include "classes.hpp"
//#include "rclcpp/rclcpp.hpp"
//#include "ets_msgs/msg/truck.hpp"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h> // Library to use ioctl function
//////////////////

Truck truck;

#ifdef SIMULATOR
void Truck::ros_callback(const ets_msgs::msg::Truck::SharedPtr msg)
{
    this->setSpeed(msg->speed);
    this->setAcc(msg->acc_x, msg->acc_y, msg->acc_z);
    this->setRpm(msg->rpm);
    this->setGear(msg->gear);
    this->setEngine(msg->engine_running);
    this->setTrailer(msg->trailer_connected);
    this->setPosition(msg->x, msg->y, msg->z, msg->heading, msg->pitch, msg->roll);
    this->setParkingBrake(msg->parking_brake);
}
#endif

void ros_client(Truck *truck)
{
	auto node = rclcpp::Node::make_shared("ets_client");

	auto sub = node->create_subscription<ets_msgs::msg::Truck>(
		"truck", std::bind(&Truck::ros_callback, truck, std::placeholders::_1), rmw_qos_profile_default);

	rclcpp::spin(node);
}


enum class Modes {
    unknown,
    parking,
    reverse,
    surveillance,
    urban_driving,
    highway
};

class vehicle_status
{
    private:
        Modes mode = Modes::surveillance;
        bool engine_on, trailer_on, cruise_control_on;
    public:
        vehicle_status(int argc, char *argv[]){
            rclcpp::init(argc, argv);
        	std::thread truck_data(ros_client, &truck);
        }
        Modes get_mode(){ return mode; }
        bool isEngineON(){ return engine_on; }
        bool isTrailerON(){ return trailer_on; }
        bool isCruiseControlON(){ return cruise_control_on; }
        void set_state(Truck vehicle){
            if (vehicle.getEngine())
                engine_on = true;
            else
                engine_on = false;

            if (vehicle.getParkingBrake())
                mode = Modes::parking;
            else if (vehicle.getSpeed() < -0.03)
                mode = Modes::urban_driving;
            else if (vehicle.getSpeed() >= -0.03 && vehicle.getSpeed() <= 0.03)
                mode = Modes::surveillance;
            else if (vehicle.getSpeed() > 0.03 && vehicle.getSpeed() <= 60)
                mode = Modes::urban_driving;
            else
                mode = Modes::highway;

            if (vehicle.getTrailer())
                trailer_on = true;
            else
                trailer_on = false;

            if (vehicle.getCruiseControl() > 0.03)
                cruise_control_on = true;
            else
                cruise_control_on = false;
        }
};