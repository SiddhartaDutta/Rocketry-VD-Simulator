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

        // member variables
        const float R;
        const int tangent_factor;

        vector<float> x_trajectory_values;
        vector<float> y_trajectory_values;

            // user provided
        float height;
        float lat_start;
        float lon_start;
        float alt;

            // calculated
        float downrange_distance;
        float min_distance;
        float true_distnce;
        float bearing;
        float line_of_sight_angle;
        float sim_time_elapsed;
        int number_of_points;


}

#endif