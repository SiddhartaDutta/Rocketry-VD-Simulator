#!/usr/bin/env python3
import sys
from trajectory import Trajectory

def main():
    # Expect five arguments: lat_start, lon_start, alt, lat_end, lon_end
    if len(sys.argv) != 6:
        print("[ERROR] Invalid input format/length (lat_start, lon_start, alt, lat_end, lon_end)")
        sys.exit(1)
    try:
        lat_start = float(sys.argv[1])
        lon_start = float(sys.argv[2])
        alt = float(sys.argv[5])
        lat_end = float(sys.argv[3])
        lon_end = float(sys.argv[4])
    except ValueError:
        print("[ERROR] Invalid numeric value provided.")
        sys.exit(1)

    # Instantiate and configure trajectory
    flight_path = Trajectory()
    flight_path.lat_start = lat_start
    flight_path.lon_start = lon_start
    flight_path.alt = alt
    flight_path.lat_end = lat_end
    flight_path.lon_end = lon_end

    # Validate and compute
    if flight_path.is_valid_trajectory() and flight_path.alt > 0:
        flight_path.calculate_trajectory()
    else:
        # Print False if invalid
        print(flight_path.is_valid_trajectory())
        sys.exit(1)

if __name__ == "__main__":
    main()
