import os
import platform
import shutil
import csv
from pathlib import Path

def ensure_directory_exists(directory):
    Path(directory).mkdir(parents=True, exist_ok=True)

def get_last_valid_value(csv_file):
    with open(csv_file, 'r') as f:
        reader = csv.reader(f)
        last_valid_row = None
        for row in reader:
            if row:
                last_valid_row = row
        if last_valid_row:
            return last_valid_row[-1].strip()
    return None

def main():
    runs = 5
    models = ["Cow", "Dragon", "Face", "Car"]
    for model in models:
        runtime_csv = "runtime.csv"
        onetime_csv = "oncetime/oncetime.csv"
        totaltime_csv = "oncetime/totaltime.csv"
        statistics_dir = "statistics"

        with open(runtime_csv, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["Index", "Model", "Construction Time(us)"])

        for i in range(1, runs + 1):
            if platform.system() == 'Windows':
                os.system(f"powershell -ExecutionPolicy Bypass -File run.ps1 {model}")
            else:
                os.system(f"./run.sh {model}")

            target_dir = os.path.join(statistics_dir, model, f"{i:03d}")
            ensure_directory_exists(target_dir)

            shutil.copy2(onetime_csv, os.path.join(target_dir, "oncetime.csv"))
            shutil.copy2(totaltime_csv, os.path.join(target_dir, "totaltime.csv"))

            time_value = get_last_valid_value(totaltime_csv)
            if time_value is None:
                print(f"ERROR: can't get valid time value from {totaltime_csv}(round {i}, model {model})")
                continue

            with open(runtime_csv, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([i, model, time_value])

            print(f"model {model}: round {i} in total {runs}")

if __name__ == "__main__":
    main()