# $1 --- id of the box to run the solution in
# $2 --- id of the box to check the output in

# Script to end the testing process
# Usage: end_testing.sh <box_id1> <box_id2>
# Cleans up the isolated boxes after testing is complete

#!/usr/bin/env bash

if [ "$1" = "" ] || [ "$2" = "" ]; then
    echo "Usage: $0 <box_id1> <box_id2>"
    exit 255
fi

isolate --cleanup --box-id=$1
isolate --cleanup --box-id=$2
