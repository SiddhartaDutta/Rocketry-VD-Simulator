#include "../header/trajectory.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
// #include <iterator>
#include <thread>

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

// Constructor
trajectory::trajectory():R(6371000), alt_interval(50), tangent_factor(10){
    true_distance = 0;
}

float trajectory::evaluate_sigmoid(float x){

    float sigmoid_factor = 1/(1 + exp(-(x-0.5*height)/(height/tangent_factor)));
    return downrange_distance * sigmoid_factor;

}

void trajectory::calculate_trajectory(){
    int temp_count = 0;
    
    for(int i = 0; i <= (int)height; i += alt_interval){
        temp_count++;

        x_trajectory_values.push_back(evaluate_sigmoid((float)i));
        y_trajectory_values.push_back(-(float)i + height);

        if(temp_count >= 2){
            true_distance += sqrt(pow(x_trajectory_values[temp_count-1]-x_trajectory_values[temp_count-2], 2) + pow(y_trajectory_values[temp_count-1]-y_trajectory_values[temp_count-2], 2));
        }
    }

    if(y_trajectory_values.back() != 0){
        x_trajectory_values.push_back(evaluate_sigmoid((float)height));
        y_trajectory_values.push_back(0);
    }

    number_of_points = x_trajectory_values.size();

    // Store data
    // Generate File
    std::ofstream output_csv(generate_data_file_path());

    if (!output_csv.is_open()) {
        std::cerr << "Error generating file." << std::endl;
        return;
    }

    output_csv << "x,y" << endl;
    for(int i = 0; i < number_of_points; i++){
        output_csv << x_trajectory_values[i] << "," << y_trajectory_values[i] << endl;
    }

    output_csv.close();

}

void trajectory::reset_trajectory(){
    x_trajectory_values.clear();
    y_trajectory_values.clear();
    number_of_points = true_distance = height = downrange_distance = 0;
}

bool trajectory::is_valid_trajectory(){
    float phi1 = lat_start * (M_PI / 180.0);
    float phi2 = lat_end * (M_PI / 180.0);
    float lam1 = lon_start * (M_PI / 180.0);
    float lam2 = lon_end * (M_PI / 180.0);
    float dphi = phi2 - phi1;
    float dlam = lam2 - lam1;

    float a = pow(sin(dphi / 2), 2) + cos(phi1) * cos(phi2) * pow(sin(dlam / 2), 2);
    downrange_distance = R * atan2(sqrt(a), sqrt(1 - a));
    // downrange_distance = 2 * R * asin(sqrt(a));

    line_of_sight_angle = atan(height / downrange_distance) * (180 / M_PI);

    bearing = (atan2(sin(dlam) * cos(phi2), cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(dlam))) * (180 / M_PI);

    min_distance = sqrt(pow(height, 2) + pow(downrange_distance, 2));

    if (line_of_sight_angle < 0.0) {
        return false;
    }

    return true;
}