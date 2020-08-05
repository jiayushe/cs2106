#!/bin/bash

# Expects test files in ./tests/<test_group>/

$(g++ -std=c++11 -O3 gen.cpp -o gen)
for testfolder in ./tests/*
do
    if [[ -d $testfolder ]]
    then
        for testcase in $testfolder/*
        do
            if [[ -f $testcase && ${testcase##*.} == "gen" ]]
            then
                yes | rm "${testcase%.*}.in" > /dev/null
                yes | rm "${testcase%.*}.flags" > /dev/null
                $(./gen < "$testcase" > "${testcase%.*}.in")
                $(./gen < "$testcase" flags > "${testcase%.*}.flags")
            elif [[ -f $testcase && ${testcase##*.} == "cpp" ]]
            then
                yes | rm "${testcase%.*}.in" > /dev/null
                yes | rm "${testcase%.*}.flags" > /dev/null
                $(g++ -std=c++11 -O3 "$testcase" -o tmp)
                $(./tmp > "${testcase%.*}.in")
                $(./tmp flags > "${testcase%.*}.flags")
                yes | rm "tmp" > /dev/null
            fi
        done
    fi
done
