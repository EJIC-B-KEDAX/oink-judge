#!/usr/bin/bash

# $1 --- time_limit in seconds
# $2 --- memory_limit in kilobytes
# $3 --- idle_timeout in seconds
# $4 --- input file
# $5 --- box_id

# Script to run a command in an isolated environment with specified limits
# Usage: run_in_isolate.sh <time_limit> <memory_limit> <idle_timeout> <input_file> <box_id>
# Result is written to a file named "meta.txt" in the current directory
# After running, check.sh script may be used to verify the output (output generated in isolate environment)
# Before running this script, ensure that the isolate environment is set up correctly (or you ran prepare_for_testing.sh).

echo run_in_isolate

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ] || [ "$4" = "" ] || [ "$5" = "" ]; then
    echo "Usage: $0 <time_limit> <memory_limit> <idle_timeout> <input_file> <box_id>"
    exit 255
fi

ISOLATE_PATH=/var/local/lib/isolate

cp "$4" "$ISOLATE_PATH/$5/box/input.txt"

isolate --run --box-id=$5 \
    --stdin=input.txt \
    --stdout=output.txt \
    --stderr=err.txt \
    --time=$1 \
    --mem=$2 \
    --wall-time=$3 \
    --meta=logs/meta.txt \
    ./solution

exit $?
