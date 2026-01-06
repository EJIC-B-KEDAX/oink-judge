#!/usr/bin/bash

# $1 --- id of the box to run the solution in
# $2 --- id of the box to check the output in
# $3 --- problem id

# Script to prepare the environment for testing isolated solutions
# Usage: prepare_for_testing.sh <box_id1> <box_id2> <problem_id>
# May be used to set up the environment before running solutions in isolated boxes

echo prepare_for_testing

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]; then
    echo "Usage: $0 <box_id1> <box_id2> <problem_id>"
    exit 255
fi

ISOLATE_PATH=/var/local/lib/isolate

isolate --init --box-id=$1
isolate --init --box-id=$2
