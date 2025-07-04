#include "../header/trajectory.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>

#include "arrow/api.h"
#include "arrow/io/api.h"
#include <parquet/arrow/writer.h>

// Constructor
Trajectory::Trajectory():R(6371000), altInterval(50), tangentFactor(10){
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
    std::string run_data_path = generateOutputPath();
    std::ofstream output_csv(run_data_path);
    if (!output_csv.is_open()) {
        std::cerr << "Error generating file." << std::endl;
        return;
    }

    // Store to run-specific csv
    arrow::Status returnStatus = recordParquetLog(run_data_path, xTrajectoryValues, yTrajectoryValues, vehicleSpeed, thrust);
    if (!returnStatus.ok()) {
        std::cerr << "Failed to record Parquet log: " << returnStatus.ToString() << std::endl;
        return;
    }
    

    // Store to manifest
    float lls[2] = {latStart, lonStart};
    float lle[2] = {latEnd, lonEnd};
    logToManifest(run_data_path, lls, lle, alt, isValidTrajectory());

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

// -----

namespace fs = std::filesystem;

std::string Trajectory::generateOutputPath() {
    // Locate data folder
    fs::path target_path = fs::canonical(fs::read_symlink("/proc/self/exe"));
    target_path = target_path.parent_path().parent_path().append("data");

    // Count number of files
    int count;
    try {
        count = std::count_if(
            fs::directory_iterator(target_path),
            fs::directory_iterator(),
            [](const auto& entry) {
                return entry.is_regular_file();
            }
        );
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 0;
    }

    // Create file name
    std::stringstream file;
    file << std::to_string(count + 1) << "-" << std::this_thread::get_id() << ".parquet";
    std::string temp_file = file.str();

    // Return full file path
    return target_path.append(temp_file);
    
}

bool Trajectory::logToManifest(const std::string& output_csv,
                               const float* lat_lon_start,
                               const float* lat_lon_end,
                               const float& altitude,
                               const bool& feasible){
    // Locate manifest
    fs::path target_path = fs::canonical(fs::read_symlink("/proc/self/exe"));
    target_path = target_path.parent_path().parent_path().append("data/manifest.csv");

    try{
        std::ofstream manifest_csv(target_path, std::ios::app);

        // Store to manifest
        manifest_csv 
        << lat_lon_start[0] 
        << ',' 
        << lat_lon_start[1] 
        << ',' 
        << lat_lon_end[0]
        << ','
        << lat_lon_end[1]
        << ','
        << altitude
        << ','
        << feasible
        << ','
        << output_csv 
        << std::endl; 

        manifest_csv.close();
        return 0;
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1; 
    }

}

// bool Trajectory::recordParquetLog(std::string output_csv, std::vector<float> downrange_dist, std::vector<float> altitude, std::vector<float> vehicle_speed, std::vector<float> thrust){        

//     try {
//         std::ofstream output(output_csv);

//         // Write column titles
//         output << "downrange_dist, alititude, vehicle_speed, thrust" << std::endl;

//         for(int i = 0; i < downrange_dist.size(); i++){
//             output << downrange_dist[i] << ',' << altitude[i] << ',' << "temp" << ',' << "temp" << std::endl;
//         }

//         output.close();
//         return 0;
//     } catch (const std::runtime_error& error){
//         std::cerr << "Error: " << error.what() << std::endl;
//         return 1; 
//     }

// }
    
// bool Trajectory::recordParquetLog(const std::string& output_path,
//                                   const std::vector<float>& downrange_dist,
//                                   const std::vector<float>& altitude,
//                                   const std::vector<float>& vehicle_speed,
//                                   const std::vector<float>& thrust) {
//     try {
//         arrow::FloatBuilder downrange_builder;
//         arrow::FloatBuilder altitude_builder;
//         arrow::FloatBuilder speed_builder;
//         arrow::FloatBuilder thrust_builder;

//         // Build Arrow arrays
//         for (size_t i = 0; i < downrange_dist.size(); ++i) {
//             downrange_builder.Append(downrange_dist[i]);
//             altitude_builder.Append(altitude[i]);
//             speed_builder.Append(vehicle_speed[i]);
//             thrust_builder.Append(thrust[i]);
//         }

//         std::shared_ptr<arrow::Array> downrange_array;
//         std::shared_ptr<arrow::Array> altitude_array;
//         std::shared_ptr<arrow::Array> speed_array;
//         std::shared_ptr<arrow::Array> thrust_array;

//         downrange_builder.Finish(&downrange_array);
//         altitude_builder.Finish(&altitude_array);
//         speed_builder.Finish(&speed_array);
//         thrust_builder.Finish(&thrust_array);

//         auto schema = arrow::schema({
//             arrow::field("downrange_dist", arrow::float32()),
//             arrow::field("altitude", arrow::float32()),
//             arrow::field("vehicle_speed", arrow::float32()),
//             arrow::field("thrust", arrow::float32())
//         });

//         auto table = arrow::Table::Make(schema, {
//             downrange_array, altitude_array, speed_array, thrust_array
//         });

//         std::shared_ptr<arrow::io::FileOutputStream> outfile;
//         ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(output_path));

//         PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(
//             *table, arrow::default_memory_pool(), outfile, 1024
//         ));

//         return true;
//     } catch (const std::exception& e) {
//         std::cerr << "Error writing Parquet: " << e.what() << std::endl;
//         return false;
//     }
// }

arrow::Status Trajectory::recordParquetLog(
    const std::string& output_path,
    const std::vector<float>& downrange_dist,
    const std::vector<float>& altitude,
    const std::vector<float>& vehicle_speed,
    const std::vector<float>& thrust)
{
    arrow::FloatBuilder downrange_builder;
    arrow::FloatBuilder altitude_builder;
    arrow::FloatBuilder speed_builder;
    arrow::FloatBuilder thrust_builder;

    for (size_t i = 0; i < downrange_dist.size(); ++i) {
        ARROW_RETURN_NOT_OK(downrange_builder.Append(downrange_dist[i]));
        ARROW_RETURN_NOT_OK(altitude_builder.Append(altitude[i]));
        ARROW_RETURN_NOT_OK(speed_builder.Append(vehicle_speed[i]));
        ARROW_RETURN_NOT_OK(thrust_builder.Append(thrust[i]));
    }

    std::shared_ptr<arrow::Array> downrange_array;
    std::shared_ptr<arrow::Array> altitude_array;
    std::shared_ptr<arrow::Array> speed_array;
    std::shared_ptr<arrow::Array> thrust_array;

    ARROW_RETURN_NOT_OK(downrange_builder.Finish(&downrange_array));
    ARROW_RETURN_NOT_OK(altitude_builder.Finish(&altitude_array));
    ARROW_RETURN_NOT_OK(speed_builder.Finish(&speed_array));
    ARROW_RETURN_NOT_OK(thrust_builder.Finish(&thrust_array));

    auto schema = arrow::schema({
        arrow::field("downrange_dist", arrow::float32()),
        arrow::field("altitude", arrow::float32()),
        arrow::field("vehicle_speed", arrow::float32()),
        arrow::field("thrust", arrow::float32())
    });

    auto table = arrow::Table::Make(schema, {
        downrange_array,
        altitude_array,
        speed_array,
        thrust_array
    });

    ARROW_ASSIGN_OR_RAISE(auto outfile,
        arrow::io::FileOutputStream::Open(output_path));

    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(
        *table, arrow::default_memory_pool(), outfile, 1024));

    return arrow::Status::OK();
}

