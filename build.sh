#!/bin/bash

set -e

function report_error_and_exit {
    echo "Insufficient or invalid args"
    echo "To build try build.sh compile"
    echo "To clean try build.sh clean"
    exit -1
}

if [ -z $1 ]
then
    report_error_and_exit
fi

for i in "$@"
do
case $i in
    compile)
    COMPILE="true"
    shift
    ;;
    clean)
    CLEAN="true"
    shift
    ;;
    verbose)
    VERBOSE="true"
    shift
    ;;
    *)
    shift
    ;;
esac
done

# Run CMake to generate make files
cmake -S . -B build

if [[ $COMPILE == "true" ]]
then
    # Compile the project
    if [[ $VERBOSE == "true" ]]
    then
        make -C build --no-print-directory VERBOSE=1
    else
        make -C build --no-print-directory
    fi
elif [[ $CLEAN == "true" ]]
then
    # Clean the project
    make clean -C build --no-print-directory
fi
