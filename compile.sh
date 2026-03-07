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


CC_COMPILER="gcc"
CXX_COMPILER="g++"

if command -v clang >/dev/null 2>&1; then
    echo "[>] Clang detected, preferring Clang..."
    CC_COMPILER="clang"
    CXX_COMPILER="clang++"
else
    echo "[>] Clang not found, falling back to GCC..."
fi

echo "[>] Running cmake with Ninja using $CXX_COMPILER..."
cmake -G Ninja -DCMAKE_C_COMPILER=$CC_COMPILER -DCMAKE_CXX_COMPILER=$CXX_COMPILER ..

echo "[>] Building project with Ninja..."
ninja

if [ $? -eq 0 ]; then
    echo "[100%] Running calcium3d..."
    ./calcium3d
else
    echo "[ERROR] Build failed!"
    exit 1
fi
