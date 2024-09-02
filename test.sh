##!/bin/bash
cmake --build build

if [ $? -ne 0 ]; then
    echo "Error: Failed to build project"
    exit 1
fi

cp ./build/compile_commands.json .
ctest --test-dir build
