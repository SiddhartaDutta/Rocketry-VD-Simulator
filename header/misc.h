#ifndef MISC_H
#define MISC_H

#pragma once

#include "json.hpp"

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <thread>

using json = nlohmann::json;
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
        string temp_file = file.str();
    
        // Return full file path
        return target_path.append(temp_file);
        
    }

    bool record_to_manifest(string output_csv, bool feasible, float downrange_dist){
        // Locate manifest
        fs::path target_path = fs::canonical(fs::read_symlink("/proc/self/exe"));
        target_path = target_path.parent_path().parent_path().append("data/manifest.csv");

        try{
            std::ofstream manifest_csv(target_path);

            // Store to manifest
            manifest_csv << output_csv << ',' << feasible << ',' << downrange_dist << endl; 

            return 0;
        } catch (const std::runtime_error& error) {
            std::cerr << "Error: " << error.what() << std::endl;
            return 1; 
        }

    }

    bool record_to_log_file(std::ofstream output_csv, vector<float> downrange_dist, vector<float> altitude, vector<float> vehicle_speed, vector<float> thrust){        

        try {
            // Write column titles
            output_csv << "alititude, downrange_dist, vehicle_speed, thrust" << endl;

            for(int i = 0; i < downrange_dist.size(); i++){
                output_csv << altitude[i] << ',' << downrange_dist[i] << ',' << vehicle_speed[i] << ',' << thrust[i] << endl;
            }

            // Write data
            // output_csv << 
            return 1;
        }
        

    } catch (){
        
    }
    
}

#endif