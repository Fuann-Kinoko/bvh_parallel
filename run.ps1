# Build the project
cmake --build build

# Copy the executable
Copy-Item -Path "build/Debug/BVHVisualization.exe" -Destination "./BVHVisualization.exe" -Force

# Select model, if no arg then the model is Cow
$DEFAULT_MODEL = "Cow"
$MODEL = $DEFAULT_MODEL

# Select model by arg
if ($args.Count -ge 1) {
    switch ($args[0]) {
        "Dragon" { $MODEL = "Dragon" }
        "Cow" { $MODEL = "Cow" }
        "Face" { $MODEL = "Face" }
        "Car" { $MODEL = "Car" }
        default {
            Write-Host "error: invalid arg '$($args[0])'"
            Write-Host "available: Dragon, Cow, Face, Car"
            exit 1
        }
    }
}

# Run the visualization
./BVHVisualization.exe --no_gui $MODEL