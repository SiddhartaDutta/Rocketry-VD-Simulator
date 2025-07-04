#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <string>
#include <vector>
#include <math.h>

#include <arrow/status.h>

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
        bool logToManifest(const std::string& output_csv,
                           const float* lat_lon_start,
                           const float* lat_lon_end,
                           const float& altitude,
                           const bool& feasible);
        arrow::Status recordParquetLog(const std::string& output_csv, 
                              const std::vector<float>& downrange_dist,
                              const std::vector<float>& altitude,
                              const std::vector<float>& vehicle_speed,
                              const std::vector<float>& thrust);

        // member variables
        const float R;
        const int tangentFactor;
        const int altInterval;

        std::vector<float> xTrajectoryValues;
        std::vector<float> yTrajectoryValues;
        std::vector<float> vehicleSpeed;
        std::vector<float> thrust;

            // user provided
        float latStart;
        float lonStart;
        float alt;

            // calculated
        float latEnd;
        float lonEnd;
        float downrangeDistance;
        float minDistance;
        float trueDistance;
        float bearing;
        float lineOfSightAngle;
        float simTimeElapsed;
        int numberOfPoints;

};

#endif