#!/bin/bash

if [ "$1" = "nb" ]; then
    cd build/bin || exit 1
    ./QuarkEngine
else
    rm -rf build
    mkdir build
    cd build || exit 1
    cmake ..
    make
    cd bin || exit 1
    ./QuarkEngine
fi