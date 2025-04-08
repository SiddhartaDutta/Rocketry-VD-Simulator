#ifndef MISC_H
#define MISC_H

#pragma once

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

string generate_data_file_path() {

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

#endif