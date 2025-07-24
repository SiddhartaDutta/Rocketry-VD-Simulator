import math
import json

import pandas as pd

class Rocket:
    def __init__(self, altitude, json_path):
        with open(json_path) as file:
            json_data = json.load(file)

        self.engine_count = json_data['rocket']['engine']['engine_count']   # number
        self.max_thrust = json_data['rocket']['engine']['max_thrust_kN']    # kN
        self.min_thrust = json_data['rocket']['engine']['min_thrust_kN']    # kN

        self.height = json_data['rocket']['dimensions']['height_m']         # m
        self.diameter = json_data['rocket']['dimensions']['diameter_m']     # m
        self.drag_coeff = json_data['rocket']['dimensions']['drag_coeff']   # ratio
        self.dry_weight = json_data['rocket']['dimensions']['dry_weight']   # kg

        self.fuel_density = json_data['rocket']['fuel']['fuel_density']     # 
        self.OtF_ratio = json_data['rocket']['fuel']['OtF_ratio']           # ratio
        self.ox_density = json_data['rocket']['fuel']['ox_density']         # 

        self.altitude = altitude    # m
        self.acceleration = 0.0     # m/s^2
        self.velocity = 0.0         # m/s
        self.thrust = 0.0           # N

        self.fuel = 0.0             # kg
        self.oxidizer = 0.0         # kg
        self.burnrate = 0.0         # %
        self.total_fuel_mass = (self.fuel * self.fuel_density) + (self.fuel * self.OtF_ratio * self.ox_density)

    def gravity_at_altitude(self) -> float:
        g0 = 9.80665
        R = 6.371e6
        return g0 * (R / (R + self.altitude)) ** 2
    
    def air_density_at_altitude(self) -> float:
        rho0 = 1.225  # kg/m^3 at sea level
        H = 8500.0    # scale height in meters
        return rho0 * math.exp(-self.altitude / H)

    def dynamic_pressure(self) -> float:
        rho = self.air_density_at_altitude(self.altitude)
        return 0.5 * rho * self.velocity**2

    def step(self):
        pass

    def get_state(self):
        return self.altitude, self.acceleration, self.velocity

    def print(self):
        print(self.ox_density)
        
rocket = Rocket('../data/rocket.json')
rocket.print()