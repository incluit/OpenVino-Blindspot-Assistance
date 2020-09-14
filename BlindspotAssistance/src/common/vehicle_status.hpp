#include <iostream>
#include <string>
#include "classes.hpp"

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
        Vehicle vehicle;
        void calc_mode(){
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
        }
    public:
        Modes get_mode(){
            calc_mode();
            return mode;
        }
        std::string get_mode_to_string(){
            vehicle.calc_mocked_status();
            calc_mode();
            if( mode == Modes::parking )
                return "Parking";
            if( mode == Modes::reverse )
                return "Reverse";
            if( mode == Modes::surveillance )
                return "Surveillance";
            if( mode == Modes::urban_driving )
                return "Urban Driving";
            if( mode == Modes::highway )
                return "Highway";
            return "unknown";
        }
        bool isEngineON(){ 
            if (vehicle.getEngine())
                engine_on = true;
            else
                engine_on = false;
            return engine_on;
        }
        bool isTrailerON(){ 
            if (vehicle.getTrailer())
                trailer_on = true;
            else
                trailer_on = false;
            return trailer_on;
        }
        bool isCruiseControlON(){ 
            if (vehicle.getCruiseControl() > 0.03)
                cruise_control_on = true;
            else
                cruise_control_on = false;
            return cruise_control_on;
        }
};