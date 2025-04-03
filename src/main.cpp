#include <iostream>

#include "trajectory.h"

using namespace std;

int main(){
    trajectory new_trajectory = trajectory();

    new_trajectory.lat_start = 28.660;
    new_trajectory.lon_start = -80.167;
    new_trajectory.height = 35500;
    new_trajectory.lat_end = 28.870;
    new_trajectory.lon_end = -79.969;

    if(new_trajectory.is_valid_trajectory()){
        new_trajectory.calculate_trajectory();
    } else {
        cout << new_trajectory.is_valid_trajectory() << endl;
    }

}