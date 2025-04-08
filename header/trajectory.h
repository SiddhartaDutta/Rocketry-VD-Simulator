#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <vector>
#include <math.h>

using namespace std;

#pragma once

class trajectory{
    public:
        // constructor
        trajectory();

        // methods
        float evaluate_sigmoid(float x);
        void calculate_trajectory();
        bool is_valid_trajectory();
        void reset_trajectory();

        // member variables
        const float R;
        const int tangent_factor;
        const int alt_interval;

        vector<float> x_trajectory_values;
        vector<float> y_trajectory_values;

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