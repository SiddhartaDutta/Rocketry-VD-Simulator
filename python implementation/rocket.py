import math
import json

import pandas as pd

class Rocket:
    def __init__(self, altitude, json_path):
        with open(json_path) as file:
            json_data = json.load(file)

        self.engine_count = json_data['rocket']['engine']['engine_count']   # number
        self.max_thrust = json_data['rocket']['engine']['max_thrust_N']    # N
        self.min_throttle = json_data['rocket']['engine']['min_throttle']    # fraction (0.0-1.0)
        self.burn_rate = json_data['rocket']['engine']['burn_rate']

        self.height = json_data['rocket']['dimensions']['height_m']         # m
        self.diameter = json_data['rocket']['dimensions']['diameter_m']     # m
        self.drag_coeff = json_data['rocket']['dimensions']['drag_coeff']   # ratio
        self.dry_weight = json_data['rocket']['dimensions']['dry_weight']   # kg
        self.max_q = json_data['rocket']['dimensions']['max_q']

        self.fuel_density = json_data['rocket']['fuel']['fuel_density']     # 
        self.OtF_ratio = json_data['rocket']['fuel']['OtF_ratio']           # ratio
        self.ox_density = json_data['rocket']['fuel']['ox_density']         # 

        self.gravity = 0.0
        self.thrust_v = 0.0           # N
        self.thrust_h = 0.0     # N
        self.dynamic_pressure = 0.0
        self.acceleration_v = 0.0     # m/s^2
        self.acceleration_h = 0.0
        self.velocity_v = 0.0        # m/s
        self.velocity_h = json_data['rocket']['physics']['h_vel_at_apogee']        # m/s
        self.altitude = altitude    # m
        self.x_position = 0.0
        self.angle = 0.0 # vertical

        self.d_time = json_data['rocket']['physics']['d_time']

        self.fuel = 0.0             # kg
        self.oxidizer = 0.0         # kg
        self.throttle = 0.0         # fraction (0.0-1.0)
        self.total_fuel_mass = (self.fuel * self.fuel_density) + (self.fuel * self.OtF_ratio * self.ox_density)
        self.total_mass = self.dry_weight + self.total_fuel_mass

    def burn_fuel(self):
        fuel_used = self.burn_rate * self.throttle * self.engine_count * self.d_time

        if fuel_used > self.fuel:
            fuel_used = self.fuel
            self.throttle = 0

        self.fuel -= fuel_used
        self.oxidizer -= fuel_used * self.OtF_ratio

        self.total_fuel_mass = (self.fuel * self.fuel_density) + (self.fuel * self.OtF_ratio * self.ox_density)
        self.total_mass = self.dry_weight + self.total_fuel_mass

    def compute_air_density_at_altitude(self) -> float:
        rho0 = 1.225  # kg/m^3 at sea level
        H = 8500.0    # scale height in meters
        return rho0 * math.exp(-self.altitude / H)

    def compute_dynamic_pressure(self) -> float:
        rho = self.compute_air_density_at_altitude()
        velocity_total = math.sqrt(self.velocity_h**2 + self.velocity_v**2)
        return 0.5 * rho * velocity_total**2

    def compute_air_temp(self) -> float:
        """Estimate air temperature at current altitude using ISA model."""
        if self.altitude < 11000:
            return 288.15 - 0.0065 * self.altitude  # troposphere
        else:
            return 216.65  # lower stratosphere

    def compute_speed_of_sound(self) -> float:
        """Compute speed of sound at current altitude (m/s)."""
        gamma = 1.4
        R = 287.05  # J/(kg·K)
        T = self.compute_air_temperature()
        return math.sqrt(gamma * R * T)
        

    def compute_mach_number(self) -> float:
        v_total = math.sqrt(self.velocity_v**2 + self.velocity_h**2)
        a = self.compute_speed_of_sound()
        return v_total / a if a > 0 else 0

    def compute_air_viscosity(self) -> float:
        T0 = 288.15  # K
        mu0 = 1.716e-5  # Reference viscosity
        C = 110.4  # Sutherland's constant
        T = self.compute_air_temperature()
        return mu0 * ((T / T0) ** 1.5) * (T0 + C) / (T + C)

    def compute_reynolds_number(self) -> float:
        rho = self.compute_air_density_at_altitude()
        mu = self.compute_dynamic_viscosity()
        v = math.sqrt(self.velocity_v**2 + self.velocity_h**2)
        L = self.diameter  # characteristic length (could use length or diameter)
        return (rho * v * L) / mu if mu > 0 else 0

    def compute_angle_of_attack(self) -> float:
        vel_angle = math.atan2(self.velocity_h, self.velocity_v)
        aoa = self.angle - vel_angle
        return math.atan2(math.sin(aoa), math.cos(aoa))  # normalize to [-π, π]


    def compute_Cd(self) -> float:
        Re = self.compute_reynolds_number()
        mach = self.compute_mach_number()
        aoa = abs(self.compute_angle_of_attack())

        # --- Viscous drag model (empirical) ---
        if Re > 0:
            Cd_viscous = 24 / Re + 6 / (1 + math.sqrt(Re)) + 0.4
        else:
            Cd_viscous = 1.0
        Cd_viscous = min(max(Cd_viscous, 0.2), 1.5)

        # --- Compressibility factor ---
        if mach < 1:
            compress_factor = 1
        elif mach < 5:
            compress_factor = 1 + 0.2 * (mach - 1)
        else:
            compress_factor = 2.0

        # --- AoA effect (quadratic bump for larger AoA) ---
        aoa_deg = math.degrees(aoa)
        aoa_factor = 1 + 0.02 * aoa_deg**2

        # Final drag coefficient
        Cd_total = Cd_viscous * compress_factor * aoa_factor
        return Cd_total

    def compute_accel_gravity_at_altitude(self) -> float:
        g0 = 9.80665
        R = 6.371e6
        return g0 * (R / (R + self.altitude)) ** 2
    
    def compute_F_gravity(self) -> float:
        return self.total_mass * self.compute_accel_gravity_at_altitude()
    
    def compute_F_thrust(self) -> float:
        return self.throttle * self.max_thrust * self.engine_count
    

    def step(self):

        # ensure min throttle
        if self.throttle < self.min_throttle and self.throttle != 0.0:
            self.throttle = self.min_throttle

        # update fuel/mass
        self.burn_fuel()

        # update gravity
        self.gravity = self.compute_F_gravity()

        # update thrust
        thrust_total = self.compute_F_thrust()
        self.thrust_v = thrust_total * math.cos(self.angle)
        self.thrust_h = thrust_total * math.sin(self.angle)


        # update dynamic pressure on vehicle
        self.dynamic_pressure = self.compute_dynamic_pressure()

        # validate maxQ
        if self.dynamic_pressure > self.max_q:
            pass

        # update acceleration
        self.acceleration_v = (self.thrust_v - self.gravity) / self.total_mass
        self.acceleration_h = self.thrust_h / self.total_mass

        # update velocity
        self.velocity_v += self.acceleration_v * self.d_time
        self.velocity_h += self.acceleration_h * self.d_time

        # update position
        self.altitude += self.velocity_v * self.d_time
        self.x_position += self.velocity_h * self.d_time



            # verify position
        if self.altitude < 0:
            self.altitude = 0
            self.velocity_v = 0
            self.velocity_h = 0
        
        pass

    def get_state(self):
        return {
            "altitude": self.altitude,
            "x_pos": self.x_position,
            "velocity_v": self.velocity_v,
            "velocity_h": self.velocity_h,
            "accel_v": self.acceleration_v,
            "accel_h": self.acceleration_h,
            "angle": self.angle,
            "dynamic_pressure": self.dynamic_pressure,
            "total_mass": self.total_mass,
            "fuel_remaining": self.fuel,
            "throttle": self.throttle
        }

        