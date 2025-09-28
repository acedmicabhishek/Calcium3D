#!/bin/bash

# Get the script's directory (root of the project)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Go to project root
cd "$PROJECT_ROOT" || exit

echo "[>] Cleaning build directory..."
rm -rf build

echo "[>] Creating new build directory..."
mkdir build && cd build || exit

echo "[>] Running cmake..."
cmake ..

echo "[>] Building project..."
make

echo "[100%] Running calcium3d..."
./calcium3d
