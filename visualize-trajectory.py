import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt

# Ensure correct path
file_location = os.path.abspath(__file__)
file_location = os.path.dirname(file_location)
os.chdir(file_location)
print(file_location)

# Run test file
# exit_code = subprocess.run(["./build/test"])
# if exit_code:
#     print('[ERROR] Error while running .exe, quitting...')
#     quit()

data = pd.read_csv("data/t1.csv")

# Plot data
plt.plot(data["x"], data["y"], marker="o", linestyle="dotted", color="blue")
plt.xlabel("Absolute Distance from Apogee (Lat/Lon)")
plt.ylabel("Altitude")
plt.title("Estimated Landing Trajectory")
plt.grid(True)

# Show plot
plt.show()