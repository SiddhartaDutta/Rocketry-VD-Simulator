import json

import pandas as pd

class Rocket:
    def __init__(self, json_path):
        with open(json_path) as file:
            json_data = json.load(file)

        self.engine_count = json_data['rocket']['engine']['engine_count']
        self.max_thrust = json_data['rocket']['engine']['max_thrust_kN']
        self.min_thrust = json_data['rocket']['engine']['min_thrust_kN']

        self.height = json_data['rocket']['dimensions']['height_m']
        self.diameter = json_data['rocket']['dimensions']['diameter_m']
        self.drag_coeff = json_data['rocket']['dimensions']['drag_coeff']

        self.fuel_density = json_data['rocket']['fuel']['fuel_density']
        self.OtF_ratio = json_data['rocket']['fuel']['OtF_ratio']
        self.ox_density = json_data['rocket']['fuel']['ox_density']

    def print(self):
        print(self.ox_density)
        
rocket = Rocket('../data/rocket.json')
rocket.print()