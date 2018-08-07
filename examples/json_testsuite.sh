#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "Usage: ./json_testsuite.sh <path to store_parser executable>"
    exit 1
fi

mapfile -t Y_TEST_FILES < <(find -L json_testsuite_data -name y_\*.json | sort)
mapfile -t N_TEST_FILES < <(find -L json_testsuite_data -name n_\*.json | sort)
mapfile -t I_TEST_FILES < <(find -L json_testsuite_data -name i_\*.json | sort)

echo "Running positive test cases..."
$@ "${Y_TEST_FILES[@]}"

echo
echo "Running negative test cases..."
$@ "${N_TEST_FILES[@]}"

echo
echo "Running undefined test cases..."
$@ "${I_TEST_FILES[@]}"

