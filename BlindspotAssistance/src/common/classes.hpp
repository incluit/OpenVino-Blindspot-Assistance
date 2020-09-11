
class Vehicle
{
    private:
        bool parkingBrakeON = true;
        float speed = 0.0;
        bool engineON = true;
        bool trailerON = true;
        float cruiseControl = 0.0;

    public:
        bool getParkingBrake(){ return parkingBrakeON; }
        float getSpeed(){ return speed; }
        bool getEngine(){ return engineON; }
        bool getTrailer(){ return trailerON; }
        float getCruiseControl() {return cruiseControl; }
};