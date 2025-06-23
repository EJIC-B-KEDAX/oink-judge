# $1 --- id of the box to run the solution in
# $2 --- id of the box to check the output in
# $3 --- problem id
# $4 --- submission id
# $5 --- language

# Script to prepare the environment for testing isolated solutions
# Usage: prepare_for_testing.sh <box_id1> <box_id2> <problem_id> <submission_id> <language>
# May be used to set up the environment before running solutions in isolated boxes

#!/usr/bin/env bash
if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ] || [ "$4" = "" ] || [ "$5" = "" ]; then
    echo "Usage: $0 <box_id1> <box_id2> <problem_id> <submission_id> <language>"
    exit 255
fi

ISOLATE_PATH=/var/local/lib/isolate

isolate --init --box-id=$1
isolate --init --box-id=$2

SOURCES=""

for file in $(../problems/$3/get_source_names.sh); do
    if [ -f "../submissions/$4/$file" ]; then
        SOURCES="$SOURCES ../submissions/$4/$file"
    fi
done

../problems/$3/compile_$5.sh $SOURCES $ISOLATE_PATH/$1/box/solution > ../submissions/$4/compile_output.txt 2>&1

if [ $? -ne 0 ]; then
    echo "Compilation failed for submission $4 in box $1, for more details see submissions/$4/compile_output.txt"

    isolate --cleanup --box-id=$1
    isolate --cleanup --box-id=$2

    exit 2
fi

cp ../problems/$3/checker $ISOLATE_PATH/$2/box/checker
