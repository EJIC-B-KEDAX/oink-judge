#!/usr/bin/bash

# $1 answer file
# $2 box_id where the solution is run
# $3 box_id where output may be checked
# $4 optional testset
# $5 optional group

# Script to check the output of a test case
# Usage: check.sh <answer_file> <box_id1> <box_id2> [<testset>] [<group>]
# Result is written to a file named "meta.txt" in the current directory
# Run this script after running the solution in an isolated environment using run_in_isolate.sh
# Before running this script, ensure that the isolate environment is set up correctly (or you ran prepare_for_testing.sh).

echo check

if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]; then
    echo "Usage: $0 <answer_file> <box_id1> <box_id2> [<testset>] [<group>]"
    exit 255
fi

ISOLATE_PATH=/var/local/lib/isolate

mv "$ISOLATE_PATH/$2/box/input.txt" "$ISOLATE_PATH/$3/box/input.txt"
cp "$1" "$ISOLATE_PATH/$3/box/answer.txt"
mv "$ISOLATE_PATH/$2/box/output.txt" "$ISOLATE_PATH/$3/box/output.txt"

if [ "$4" == "" ]; then

    isolate --run --box-id=$3 \
        --stdout=checker_output.txt \
        --stderr=err.txt \
        -- ./checker input.txt output.txt answer.txt

        mv "$ISOLATE_PATH/$3/box/err.txt" logs/checker_err.txt
        mv "$ISOLATE_PATH/$3/box/checker_output.txt" logs/checker_output.txt

    exit $?
fi

if [ "$5" == "" ]; then

    isolate --run --box-id=$3 \
        --stdout=checker_output.txt \
        --stderr=err.txt \
        -- ./checker --testset $4 input.txt output.txt answer.txt

        mv "$ISOLATE_PATH/$3/box/err.txt" logs/checker_err.txt
        mv "$ISOLATE_PATH/$3/box/checker_output.txt" logs/checker_output.txt

    exit $?
fi

isolate --run --box-id=$3 \
    --stdout=checker_output.txt \
    --stderr=err.txt \
    -- ./checker --testset $4 --group $5 input.txt output.txt answer.txt

    mv "$ISOLATE_PATH/$3/box/err.txt" logs/checker_err.txt
    mv "$ISOLATE_PATH/$3/box/checker_output.txt" logs/checker_output.txt

exit $?