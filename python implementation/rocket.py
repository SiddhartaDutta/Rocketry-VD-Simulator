import math
import json

import pandas as pd

class Rocket:
    def __init__(self, json_path, R: float = 6_371_000):
        # earth radius
        self.R = R

        with open(json_path) as file:
            json_data = json.load(file)

        # read values
        self.lat_start = json_data['rocket']['init']['lat_start']
        self.lon_start = json_data['rocket']['init']['lon_start']
        self.lat_end = json_data['rocket']['init']['lat_end']
        self.lon_end = json_data['rocket']['init']['lon_end']
        self.altitude = json_data['rocket']['init']['altitude']

        self.engine_count = json_data['rocket']['engine']['engine_count']   # number
        self.max_thrust = json_data['rocket']['engine']['max_thrust_N']     # N
        self.min_throttle = json_data['rocket']['engine']['min_throttle']   # fraction (0.0-1.0)
        self.burn_rate = json_data['rocket']['engine']['burn_rate']

        self.height = json_data['rocket']['dimensions']['height_m']         # m
        self.diameter = json_data['rocket']['dimensions']['diameter_m']     # m
        self.drag_coeff = json_data['rocket']['dimensions']['drag_coeff']   # ratio
        self.dry_weight = json_data['rocket']['dimensions']['dry_weight']   # kg
        self.max_q = json_data['rocket']['dimensions']['max_q']

        self.fuel_density = json_data['rocket']['fuel']['fuel_density']     # 
        self.OtF_ratio = json_data['rocket']['fuel']['OtF_ratio']           # ratio
        self.ox_density = json_data['rocket']['fuel']['ox_density']         # 

        self.velocity_h = json_data['rocket']['physics']['h_vel_at_apogee'] # m/s
        self.d_time = json_data['rocket']['physics']['d_time']

        # init runtime vars
        self._init()

    def _init(self):
        # generated values
        self.gravity = 0.0                                                  # N
        self.drag_v = 0.0                                                   # N
        self.drag_h = 0.0                                                   # N
        self.thrust_v = 0.0                                                 # N
        self.thrust_h = 0.0                                                 # N
        self.dynamic_pressure = 0.0
        self.exceeded_maxq = False                                          # boolean
        self.acceleration_v = 0.0                                           # m/s^2
        self.acceleration_h = 0.0
        self.velocity_v = 0.0                                               # m/s

        # direct positioning
        self.downrange_distance = 0.0                                       # m
        self.x_position = 0.0
        self.angle = 0.0                                                    # vertical
        self.AoA = 0.0
        self.LoS = 0.0
            # triangulated distance (alt |__ downrange)
        self.direct_distance = 0.0

        # fuel
        self.fuel = 0.0                                                     # kg
        self.oxidizer = 0.0                                                 # kg
        self.throttle = 0.0                                                 # fraction (0.0-1.0)
        self.total_fuel_mass = (self.fuel * self.fuel_density) + (self.fuel * self.OtF_ratio * self.ox_density)
        self.total_mass = self.dry_weight + self.total_fuel_mass            # kg

        # step data list
        self.step_data: list[dict] = []

        # route validity
        self.valid_route = self._validate_setup_initial_positions()

    def _validate_setup_initial_positions(self) -> bool:
        phi1 = math.radians(self.lat_start)
        phi2 = math.radians(self.lat_end)
        dphi = phi2 - phi1
        dlam = math.radians(self.lon_end - self.lon_start)

        a = (math.sin(dphi / 2) ** 2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlam / 2) ** 2)

        self.downrange_distance = self.R * math.atan2(math.sqrt(a), math.sqrt(1 - a))
        self.LoS = math.degrees(math.atan(self.altitude / self.downrange_distance))
        self.direct_distance = math.hypot(self.altitude, self.downrange_distance)

        return self.LoS >= 0.0

    def reset(self):
        self.init()

    def _burn_fuel(self):
        fuel_used = self.burn_rate * self.throttle * self.engine_count * self.d_time

        if fuel_used > self.fuel:
            fuel_used = self.fuel
            self.throttle = 0

        self.fuel -= fuel_used
        self.oxidizer -= fuel_used * self.OtF_ratio

        self.total_fuel_mass = (self.fuel * self.fuel_density) + (self.fuel * self.OtF_ratio * self.ox_density)
        self.total_mass = self.dry_weight + self.total_fuel_mass

    def _compute_air_density_at_altitude(self) -> float:
        rho0 = 1.225  # kg/m^3 at sea level
        H = 8500.0    # scale height in meters
        return rho0 * math.exp(-self.altitude / H)

    def _compute_dynamic_pressure(self) -> float:
        rho = self._compute_air_density_at_altitude()
        velocity_total = math.sqrt(self.velocity_h**2 + self.velocity_v**2)
        return 0.5 * rho * velocity_total**2

    def _compute_air_temp(self) -> float:
        """
        Estimate air temperature using the International Standard Atmosphere (ISA) model.

        Returns:
            float: Temperature in Kelvin at the current altitude.
        """        
        h = self.altitude
        if h < 11000:
            return 288.15 - 0.0065 * h
        elif h < 20000:
            return 216.65
        elif h < 32000:
            return 216.65 + 0.001 * (h - 20000)
        elif h < 47000:
            return 228.65 + 0.0028 * (h - 32000)
        elif h < 51000:
            return 270.65
        elif h < 71000:
            return 270.65 - 0.0028 * (h - 51000)
        elif h <= 86000:
            return 214.65 - 0.002 * (h - 71000)
        else:
            return 186.65

    def _compute_speed_of_sound(self) -> float:
        """Compute speed of sound at current altitude (m/s)."""
        gamma = 1.4
        R = 287.05  # J/(kg·K)
        T = self._compute_air_temp()
        return math.sqrt(gamma * R * T)
        
    def _compute_mach_number(self) -> float:
        v_total = math.sqrt(self.velocity_v**2 + self.velocity_h**2)
        a = self._compute_speed_of_sound()
        return v_total / a if a > 0 else 0

    def _compute_air_viscosity(self) -> float:
        T0 = 288.15  # K
        mu0 = 1.716e-5  # Reference viscosity
        C = 110.4  # Sutherland's constant
        T = self._compute_air_temp()
        return mu0 * ((T / T0) ** 1.5) * (T0 + C) / (T + C)

    def _compute_reynolds_number(self) -> float:
        rho = self._compute_air_density_at_altitude()
        mu = self._compute_air_viscosity()
        v = math.sqrt(self.velocity_v**2 + self.velocity_h**2)
        L = self.diameter  # characteristic length (could use length or diameter)
        return (rho * v * L) / mu if mu > 0 else 0

    def _compute_angle_of_attack(self) -> float:
        if self.velocity_h == 0 and self.velocity_v == 0:
            return 0.0
        vel_angle = math.atan2(self.velocity_h, self.velocity_v)
        aoa = self.angle - vel_angle
        return math.atan2(math.sin(aoa), math.cos(aoa))  # normalize to [-π, π]

    def _compute_projected_area(self) -> float:
        frontal_area = math.pi * (self.diameter / 2) ** 2
        side_area = math.pi * self.diameter * self.height

        aoa = abs(self.AoA)
        return frontal_area * math.cos(aoa) ** 2 + side_area *math.sin(aoa) ** 2

    def _compute_Cd(self) -> float:
        Re = self._compute_reynolds_number()
        mach = self._compute_mach_number()
        aoa = self.AoA

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

    def _compute_accel_gravity_at_altitude(self) -> float:
        g0 = 9.80665
        R = 6.371e6
        return g0 * (R / (R + self.altitude)) ** 2
    
    def _compute_F_gravity(self) -> float:
        return self.total_mass * self._compute_accel_gravity_at_altitude()
    
    def _compute_F_thrust(self) -> float:
        return self.throttle * self.max_thrust * self.engine_count
    
    def _compute_F_drag(self) -> float:
        rho = self._compute_air_density_at_altitude()
        v = math.sqrt(self.velocity_h ** 2 + self.velocity_v ** 2)
        Cd = self._compute_Cd()
        A = self._compute_projected_area()
        return 0.5 * rho * v**2 * Cd * A

    def step(self):

        # ensure min throttle
        if self.throttle < self.min_throttle and self.throttle != 0.0:
            self.throttle = self.min_throttle

        # update fuel/mass
        self._burn_fuel()

        # update AoA
        self.AoA = self._compute_angle_of_attack()

        # update dynamic pressure on vehicle
        self.dynamic_pressure = self._compute_dynamic_pressure()

        # validate maxQ
        if self.dynamic_pressure > self.max_q:
            self.exceeded_maxq = True

        # update gravity
        self.gravity = self._compute_F_gravity()

        # update thrust
        thrust_total = self._compute_F_thrust()
        self.thrust_v = thrust_total * math.cos(self.angle)
        self.thrust_h = thrust_total * math.sin(self.angle)

        # update drag
        angle_velocity = math.atan2(self.velocity_h, self.velocity_v)
        drag_total = self._compute_F_drag()
        self.drag_v = -drag_total * math.cos(angle_velocity)
        self.drag_h = -drag_total * math.sin(angle_velocity)

        # update acceleration
        self.acceleration_v = (self.thrust_v + self.drag_v - self.gravity) / self.total_mass
        self.acceleration_h = (self.thrust_h + self.drag_h) / self.total_mass

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

        # save each step
        self.step_data.append(self.get_state())

    def get_debug(self):
        return self.__dict__

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
    
temp = Rocket('../data/rocket.json')
temp.step()
print(json.dumps(temp.get_debug(), indent= 4))