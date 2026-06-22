#!/usr/bin/env bash

mkdir -p build
cd build || exit 1

cmake -S .. -B .

cmake --build . &&
cmake --build . --target Shaders &&
./voxels

cd ..
