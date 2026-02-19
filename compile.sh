#!/bin/bash

# Get the script's directory (root of the project)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Go to project root
cd "$PROJECT_ROOT" || exit

# Create build directory if it doesn't exist (incremental build)
if [ ! -d "build" ]; then
    echo "[>] Creating build directory..."
    mkdir build
fi

cd build || exit

echo "[>] Running cmake..."
cmake ..

echo "[>] Building project..."
make -j16 # 16 logical cpu

if [ $? -eq 0 ]; then
    echo "[100%] Running calcium3d..."
    ./calcium3d
else
    echo "[ERROR] Build failed!"
    exit 1
fi
