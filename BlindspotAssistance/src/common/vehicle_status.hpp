#include <iostream>
#include <string>
#include <../classes.hpp>

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