#!/usr/bin/bash

# $1 --- output executable name
# $2 --- file to write compilation errors to
# $3 --- source file name

# Script to compile C++17 solutions
# Usage: cpp17.sh <output_executable> <error_file> <source_file>

g++ -std=c++17 -O2 -o "$1" "$3" 2> "$2"
if [ $? -ne 0 ]; then
    exit 1
fi