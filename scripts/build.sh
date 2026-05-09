#!/bin/bash
set -e

echo "Building Voxel Engine..."

# Create build directory
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

echo "Build complete! Run ./build/voxel_engine to start"
