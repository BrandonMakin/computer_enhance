import json
import random
from math import radians, sin, cos, sqrt, asin, floor

out_file = "haversine.json"

pairs = []

def generate(count):
    global pairs
    switch_up = count//10
    for groups in range(10):
        print(f"\rGenerating JSON: \033[93m", end="")
        for i in range(groups):
            print(end="█")
        for i in range(10-groups):
            print(end="─")
        print(f"\033[0m {groups*10}%",end="")
        x_bias_min = random.random()
        x_bias_max = random.random()
        if x_bias_max < x_bias_min:
            x_bias_min, x_bias_max = x_bias_max, x_bias_min
        
        y_bias_min = random.random()
        y_bias_max = random.random()
        if y_bias_max < y_bias_min:
            y_bias_min, y_bias_max = y_bias_max, y_bias_min
        
        x_min = floor(360 * x_bias_min - 180)
        x_max = floor(360 * x_bias_max - 180)
        y_min = floor(180 * y_bias_min - 90)
        y_max = floor(180 * y_bias_max - 90)

        # print(" -- ", x_min, x_max, "|", y_min, y_max, end="              ")

        for c in range(switch_up):
            pairs.append({
                "x0": random.uniform(-180, 180),
                "y0": random.uniform(-90, 90),
                "x1": random.uniform(-180, 180),
                "y1": random.uniform(-90, 90)
            })
    print("\rGenerating JSON: \033[93m██████████\033[0m 100%")
    print(f"Writing JSON to {out_file}... ",end="")
    with open(out_file, "w") as f:
        json.dump({"pairs":pairs}, f)
        print("Done.")

def HaversineOfDegrees(X0, Y0, X1, Y1, R):
  dY = radians(Y1 - Y0)
  dX = radians(X1 - X0)
  Y0 = radians(Y0)
  Y1 = radians(Y1)

  RootTerm = (sin(dY/2)**2) + cos(Y0)*cos(Y1)*(sin(dX/2)**2)
  Result = 2*R*asin(sqrt(RootTerm))

  return Result

def average_of_haversines():
    print("\033[42;1mResult: ...\033[0m",end="")
    global pairs
    EarthRadiuskm = 6371
    Sum = sum(HaversineOfDegrees(Pair['x0'], Pair['y0'], Pair['x1'], Pair['y1'], EarthRadiuskm) for Pair in pairs)
    Count = len(pairs)
    Average = Sum / Count

    print("\r\033[42;1mResult: " + str(Average) + "\033[0m")


generate(1_000_000)
# generate(10_000_000)
average_of_haversines()