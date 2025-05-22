#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <vector>
#include <math.h>

using namespace std;

#pragma once

class Trajectory{
    public:
        // constructor
        Trajectory();

        // methods
        float evaluateSigmoid(float x);
        void calculateTrajectory();
        bool isValidTrajectory();
        void resetTrajectory();

        // data loggers
        std::string generateOutputPath();
        bool logToManifest(std::string output_csv, float lat_lon_start[], float lat_lon_end[], float altitude, bool feasible);
        bool recordParquetLog(std::string output_csv, std::vector<float> downrange_dist, std::vector<float> altitude, std::vector<float> vehicle_speed, std::vector<float> thrust);

        // member variables
        const float R;
        const int tangent_factor;
        const int alt_interval;

        vector<float> x_trajectory_values;
        vector<float> y_trajectory_values;
        vector<float> vehicle_speed;
        vector<float> thrust;

            // user provided
        float lat_start;
        float lon_start;
        float alt;

            // calculated
        float lat_end;
        float lon_end;
        float downrange_distance;
        float min_distance;
        float true_distance;
        float bearing;
        float line_of_sight_angle;
        float sim_time_elapsed;
        int number_of_points;

};

#endif