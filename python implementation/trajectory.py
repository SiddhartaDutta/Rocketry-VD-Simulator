import os
import math
import threading
import inspect
from pathlib import Path

import pyarrow as pa
import pyarrow.parquet as pq


class Trajectory:
    def __init__(self, alt_interval: int = 50, tangent_factor: int = 10, R: float = 6_371_000):
        # Earth radius, sampling interval, and curve factor
        self.R = R
        self.alt_interval = alt_interval
        self.tangent_factor = tangent_factor

        # Trajectory state
        self.true_distance = 0.0
        self.downrange_distance = 0.0
        self.line_of_sight_angle = 0.0
        self.bearing = 0.0
        self.min_distance = 0.0
        self.x_trajectory_values: list[float] = []
        self.y_trajectory_values: list[float] = []
        self.number_of_points = 0

        # User-defined parameters (set before calling calculate_trajectory)
        self.alt = 0.0  # peak altitude
        self.vehicle_speed = 0.0
        self.thrust = 0.0
        self.lat_start = 0.0
        self.lon_start = 0.0
        self.lat_end = 0.0
        self.lon_end = 0.0

    def evaluate_sigmoid(self, x: float) -> float:
        """
        Compute the downrange distance scaled by a sigmoid curve of altitude.
        """
        factor = 1 / (1 + math.exp(-(x - 0.5 * self.alt) / (self.alt / self.tangent_factor)))
        return self.downrange_distance * factor

    def calculate_trajectory(self) -> None:
        """
        Sample the trajectory curve, compute true distance, write a Parquet log,
        and append an entry to the manifest CSV.
        """
        # Reset
        self.x_trajectory_values.clear()
        self.y_trajectory_values.clear()
        self.true_distance = 0.0

        # Sample down-and-back curve
        count = 0
        for h in range(0, int(self.alt) + 1, self.alt_interval):
            count += 1
            x_val = self.evaluate_sigmoid(float(h))
            y_val = -float(h) + self.alt
            self.x_trajectory_values.append(x_val)
            self.y_trajectory_values.append(y_val)
            if count >= 2:
                dx = self.x_trajectory_values[-1] - self.x_trajectory_values[-2]
                dy = self.y_trajectory_values[-1] - self.y_trajectory_values[-2]
                self.true_distance += math.hypot(dx, dy)

        # Ensure it ends at ground level
        if self.y_trajectory_values[-1] != 0.0:
            x_val = self.evaluate_sigmoid(self.alt)
            self.x_trajectory_values.append(x_val)
            self.y_trajectory_values.append(0.0)

        self.number_of_points = len(self.x_trajectory_values)

        # Write data files
        out_path = self._generate_output_path()
        self._record_parquet_log(out_path)
        self._log_to_manifest(out_path)

    def reset_trajectory(self) -> None:
        """Clear all computed and parameter values."""
        self.__init__(self.alt_interval, self.tangent_factor, self.R)

    def is_valid_trajectory(self) -> bool:
        """
        Validate based on line-of-sight angle and compute geometrical metrics.
        """
        phi1 = math.radians(self.lat_start)
        phi2 = math.radians(self.lat_end)
        dphi = phi2 - phi1
        dlam = math.radians(self.lon_end - self.lon_start)

        a = (math.sin(dphi / 2) ** 2
             + math.cos(phi1) * math.cos(phi2) * math.sin(dlam / 2) ** 2)
        self.downrange_distance = self.R * math.atan2(math.sqrt(a), math.sqrt(1 - a))
        self.line_of_sight_angle = math.degrees(math.atan(self.alt / self.downrange_distance))
        self.bearing = math.degrees(math.atan2(
            math.sin(dlam) * math.cos(phi2),
            math.cos(phi1) * math.sin(phi2)
            - math.sin(phi1) * math.cos(phi2) * math.cos(dlam)
        ))
        self.min_distance = math.hypot(self.alt, self.downrange_distance)

        return self.line_of_sight_angle >= 0.0

    def _generate_output_path(self) -> str:
        """
        Create a unique parquet filename in the ../data directory next to this script.
        """
        # Locate data folder two levels up
        script_file = inspect.getsourcefile(lambda: None)
        base_dir = Path(script_file).resolve().parent.parent / "data"
        base_dir.mkdir(parents=True, exist_ok=True)

        # Count existing files
        existing = [f for f in base_dir.iterdir() if f.is_file()]
        filename = f"{len(existing) + 1}-{threading.get_ident()}.parquet"
        return str(base_dir / filename)

    def _record_parquet_log(self, path: str) -> None:
        """
        Write trajectory samples and constants to a Parquet file.
        """
        n = len(self.x_trajectory_values)
        table = pa.Table.from_pydict({
            "x": self.x_trajectory_values,
            "y": self.y_trajectory_values,
            "vehicle_speed": [self.vehicle_speed] * n,
            "thrust": [self.thrust] * n
        })
        pq.write_table(table, path)

    def _log_to_manifest(self, path: str) -> None:
        """
        Append a CSV entry summarizing the run parameters and file path.
        """
        script_file = inspect.getsourcefile(lambda: None)
        manifest = Path(script_file).resolve().parent.parent / "data" / "manifest.csv"
        manifest.parent.mkdir(exist_ok=True)

        feasible = self.is_valid_trajectory()
        line = (
            f"{self.lat_start},{self.lon_start},"
            f"{self.lat_end},{self.lon_end},"
            f"{self.alt},{feasible},{path}\n"
        )
        with open(manifest, "a", newline="") as f:
            f.write(line)
