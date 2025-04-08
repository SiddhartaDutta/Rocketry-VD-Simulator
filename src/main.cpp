#include "../header/trajectory.h"

#include <iostream>

using namespace std;

int main(int argc, char **argv){

    // Check input format
    if(argc != 6){
        printf("[ERROR] Invalid input format/length (lat_start, lon_start, alt, lat_end, lon_end)\n");
        return 1;
    }

    trajectory flight_path = trajectory();
    
    flight_path.lat_start = std::stof(argv[1]);
    flight_path.lon_start = std::stof(argv[2]);
    flight_path.alt = std::stof(argv[3]);
    flight_path.lat_start = std::stof(argv[4]);
    flight_path.lon_start = std::stof(argv[5]);

    if(flight_path.is_valid_trajectory() && flight_path.alt > 0){
        flight_path.calculate_trajectory();
    } else {
        cout << flight_path.is_valid_trajectory() << endl;
        return 1;
    }

    return 0;
}