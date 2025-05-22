#include "../header/trajectory.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv){

    // Check input format
    if(argc != 6){
        printf("[ERROR] Invalid input format/length (lat_start, lon_start, alt, lat_end, lon_end)\n");
        return 1;
    }

    Trajectory flight_path = Trajectory();
    
    flight_path.latStart = std::stof(argv[1]);
    flight_path.lonStart = std::stof(argv[2]);
    flight_path.alt = std::stof(argv[3]);
    flight_path.latEnd = std::stof(argv[4]);
    flight_path.lonEnd = std::stof(argv[5]);

    if(flight_path.isValidTrajectory() && flight_path.alt > 0){
        flight_path.calculateTrajectory();
    } else {
        cout << flight_path.isValidTrajectory() << endl;
        return 1;
    }

    return 0;
}