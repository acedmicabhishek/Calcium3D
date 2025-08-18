#!/bin/bash

cd build
if [ $? -ne 0 ]; then
    echo "[❌] Failed to change directory to build."
    exit 1
fi
./calcium3d