import subprocess
import pandas as pd
import matplotlib.pyplot as plt

# Run test file
subprocess.run(["./build/test"])

data = pd.read_csv("t1.csv")

# Plot data
plt.plot(data["x"], data["y"], marker="o", linestyle="dotted", color="blue")
plt.xlabel("Distance from Flight Apex Lat/Lon")
plt.ylabel("Altitude")
plt.title("Plot from CSV Data")
plt.grid(True)

# Show plot
plt.show()