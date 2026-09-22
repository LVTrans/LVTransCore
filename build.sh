#!/bin/bash
set -e

BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Ninja" .
fi

cmake --build "$BUILD_DIR" -- -j "$(nproc)"

echo "Finished building"
