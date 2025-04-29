#ifndef MISC_H
#define MISC_H

#pragma once

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

namespace misc {
    std::string generate_data_file_path() {
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
        file << std::to_string(count + 1) << "-" << std::this_thread::get_id() << ".csv";
        std::string temp_file = file.str();
    
        // Return full file path
        return target_path.append(temp_file);
        
    }

    bool record_to_manifest(std::string output_csv, float lat_lon_start[], float lat_lon_end[], float altitude, bool feasible){
        // Locate manifest
        fs::path target_path = fs::canonical(fs::read_symlink("/proc/self/exe"));
        target_path = target_path.parent_path().parent_path().append("data/manifest.csv");

        try{
            std::ofstream manifest_csv(target_path);

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

    bool record_to_log_file(std::string output_csv, std::vector<float> downrange_dist, std::vector<float> altitude, std::vector<float> vehicle_speed, std::vector<float> thrust){        

        try {
            std::ofstream output(output_csv);

            // Write column titles
            output << "downrange_dist, alititude, vehicle_speed, thrust" << std::endl;

            for(int i = 0; i < downrange_dist.size(); i++){
                output << downrange_dist[i] << ',' << altitude[i] << ',' << "temp" << ',' << "temp" << std::endl;
            }

            output.close();
            return 0;
        } catch (const std::runtime_error& error){
            std::cerr << "Error: " << error.what() << std::endl;
            return 1; 
        }
        
    } 
    
}

#endif