#!/bin/bash

cd build
if [ $? -ne 0 ]; then
    echo "[❌] Failed to change directory to build."
    echo "-----make sure you compiled the project successfully."
    echo "-----run chmod +x compile.sh"
    echo "-----and then run ./compile.sh"
    echo "-----after that run ./run.sh"
    echo "[❌][❌][❌][❌][❌]"
    exit 1
fi
NV_PRIME_RENDER_OFFLOAD=1 GLX_VENDOR_LIBRARY_NAME=nvidia ./calcium3d