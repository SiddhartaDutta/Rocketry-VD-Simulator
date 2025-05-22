#include "../header/misc.h"
#include "../header/trajectory.h"

#include <iostream>
#include <fstream>
#include <vector>

// Constructor
Trajectory::Trajectory():R(6371000), altInterval(0.025), tangentFactor(10){
    trueDistance = 0;
}

float Trajectory::evaluateSigmoid(float x){

    float sigmoid_factor = 1/(1 + exp(-(x-0.5*alt)/(alt/tangentFactor)));
    return downrangeDistance * sigmoid_factor;

}

void Trajectory::calculateTrajectory(){
    int temp_count = 0;
    
    for(int i = 0; i <= (int)alt; i += altInterval){
        temp_count++;

        xTrajectoryValues.push_back(evaluateSigmoid((float)i));
        yTrajectoryValues.push_back(-(float)i + alt);

        if(temp_count >= 2){
            trueDistance += sqrt(pow(xTrajectoryValues[temp_count-1]-xTrajectoryValues[temp_count-2], 2) + pow(yTrajectoryValues[temp_count-1]-yTrajectoryValues[temp_count-2], 2));
        }
    }

    if(yTrajectoryValues.back() != 0){
        xTrajectoryValues.push_back(evaluateSigmoid((float)alt));
        yTrajectoryValues.push_back(0);
    }

    numberOfPoints = xTrajectoryValues.size();

    // Store data
    // Generate File
    string run_data_path = misc::generate_data_file_path();
    std::ofstream output_csv(run_data_path);
    if (!output_csv.is_open()) {
        std::cerr << "Error generating file." << std::endl;
        return;
    }

    // Store to run-specific csv
    misc::record_to_log_file(run_data_path, xTrajectoryValues, yTrajectoryValues, vehicleSpeed, thrust);

    // Store to manifest
    float lls[2] = {latStart, lonStart};
    float lle[2] = {latEnd, lonEnd};
    misc::record_to_manifest(run_data_path, lls, lle, alt, isValidTrajectory());

    output_csv.close(); 

}

void Trajectory::resetTrajectory(){
    xTrajectoryValues.clear();
    yTrajectoryValues.clear();
    numberOfPoints = trueDistance = alt = downrangeDistance = 0;
}

bool Trajectory::isValidTrajectory(){
    float phi1 = latStart * (M_PI / 180.0);
    float phi2 = latEnd * (M_PI / 180.0);
    float lam1 = lonStart * (M_PI / 180.0);
    float lam2 = lonEnd * (M_PI / 180.0);
    float dphi = phi2 - phi1;
    float dlam = lam2 - lam1;

    float a = pow(sin(dphi / 2), 2) + cos(phi1) * cos(phi2) * pow(sin(dlam / 2), 2);
    downrangeDistance = R * atan2(sqrt(a), sqrt(1 - a));
    // downrange_distance = 2 * R * asin(sqrt(a));

    lineOfSightAngle = atan(alt / downrangeDistance) * (180 / M_PI);

    bearing = (atan2(sin(dlam) * cos(phi2), cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(dlam))) * (180 / M_PI);

    minDistance = sqrt(pow(alt, 2) + pow(downrangeDistance, 2));

    if (lineOfSightAngle < 0.0) {
        return false;
    }

    return true;
}