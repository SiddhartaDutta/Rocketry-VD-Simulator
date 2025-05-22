#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <string>
#include <vector>
#include <math.h>

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