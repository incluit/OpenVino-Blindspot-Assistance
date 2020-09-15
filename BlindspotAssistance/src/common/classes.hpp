#include <ctime>
#include <iostream>

class Vehicle
{
    private:
        bool parkingBrakeON = true;
        float speed = 0.0;
        bool engineON = true;
        bool trailerON = true;
        float cruiseControl = 0.0;
        time_t current_time = time(NULL);

    public:
        bool getParkingBrake(){ return parkingBrakeON; }
        float getSpeed(){ return speed; }
        bool getEngine(){ return engineON; }
        bool getTrailer(){ return trailerON; }
        float getCruiseControl() {return cruiseControl; }
        void calc_mocked_status(){
            int elapsed_time = time(NULL) - current_time;
            if (elapsed_time % 10 == 0) 
                std::cout << elapsed_time << " seconds has passed." << std::endl;

            if (elapsed_time < 12)
                parkingBrakeON = true;
            else if (elapsed_time < 24){
                parkingBrakeON = false;
                speed = 0;
            }
            else if (elapsed_time < 36)
                speed = 20;
            else if (elapsed_time < 48)
                speed = 100;
            else if (elapsed_time < 60)
                speed = 20;
            else if (elapsed_time < 72)
                speed = 0;
            else
                parkingBrakeON = true;
        }
};