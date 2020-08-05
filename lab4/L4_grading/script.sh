#!/bin/bash

# Expects zip files in a folder ./submissions/
# Expects grader files in ./grader/
# Expects model solution files in ./solution/


# nullglob, so that empty directories will not be iterated
shopt -s nullglob

# Print a header line
echo "Name,1_1,2_1,2_2,2_3,2_4,2_5,2_6,2_7,2_8,3_1,3_2,3_3,3_4,3_5,3_6,4_1,4_2,"

function tester()
{
    return 5
}

# Function to do grading for a single exercises
# Returns 0 on success, nonzero on error
function grade()
{
    local ex=$1
    local test_id=$2
    local compiler_flags=$3
    local test_cases=$4
    
    #TODO: string split multiple groups of test cases
    for test_set in $test_cases
    do
        for tc in ./../tests/$test_set/*
        do
            if [[ -f $tc && ${tc##*.} == "in" ]]
            then
                # Remove the old files
                yes | rm ex_sol 1>&2
                yes | rm ex 1>&2
                yes | rm ex_sol.out 1>&2
                yes | rm ex.out 1>&2
                # Determine the flags to set FRAME_BITS and PAGE_BITS
                bits_flags=$(cat "${tc%.*}.flags")
                # Compile the code
                echo "gcc $compiler_flags $bits_flags $ex.c runner_grading.c -o ex -w -lpthread" 1>&2
                if [[ -z $(gcc $compiler_flags $bits_flags $ex.c runner_grading.c -o ex -w -lpthread 2>&1) && -f ex ]]
                then
                    # Compile the model solution
                    #echo "gcc $compiler_flags $bits_flags ex_sol.c runner_grading.c -o ex_sol -w -lpthread"
                    $(gcc $compiler_flags $bits_flags ex_sol.c runner_grading.c -o ex_sol -w -lpthread 2>&1)
                    
                    #echo "./ex_sol < $tc > ex_sol.out"
                    $(./ex_sol < $tc > ex_sol.out 2>/dev/null)
                    # If the model solution has timeout, then we have a serious problem
                    if [[ "$(tail -1 ex_sol.out)" == "OS timeout" ]]
                    then
                        return 7 # error (timeout)
                    fi
                    
                    #echo "./ex < $tc > ex.out"
                    $(./ex < $tc > ex.out 2>/dev/null)
                    $(diff ex_sol.out ex.out >/dev/null)
                    if [[ $? -ne 0 ]]
                    then
                        if [[ "$(tail -1 ex.out)" == "OS timeout" ]]
                        then
                            return 3 # error (timeout)
                        else
                            return 2 # error (diff failed)
                        fi
                    fi
                else
                    return 1 # error (compilation failed)
                fi
            fi
        done
    done
    return 0 # success
}

# Clean and generate the test inputs
$(./prepare.sh)

# Loop through all the student submissions
for f in ./submissions/*
do
    # If it is a zip file
    if [[ $f == *.zip ]]
    then
        # Echo the student name
        FILE_NAME_ONLY=$(basename "$f")
        STUDENT_NAME=${FILE_NAME_ONLY%% - *}
        echo -n "\"$STUDENT_NAME\","
        echo "\"$STUDENT_NAME\"" 1>&2
        # Create an empty directory for compilation and evaluation
        yes | rm -r stage 1>&2
        mkdir stage
        # Unzip the student directory
        unzip "$f" -d ./stage > /dev/null
        # If the code is in a nested directory, flatten the directory structure
        while true
        do
            for f2 in ./stage/*
            do
                if [[ -d $f2 ]]
                then
                    set -- "$f2/"*
                    if [[ $# -gt 0 ]]
                    then
                        mv -v "${f2}/"* ./stage/ 1>&2
                    fi
                    yes | rm -r "$f2" 1>&2
                    continue 2
                fi
            done
            break
        done
        # Remove anything that isn't ex*.c
        for f2 in ./stage/*
        do
            if [[ $f2 != */ex*.c ]]
            then
                yes | rm "$f2" 1>&2
            fi
        done
        # Copy the grader files
        cp ./grader/* ./stage
        # Copy the model solution
        cp ./solution/ex_sol.c ./stage
        # Go into the stage directory
        cd stage
        
        # Test 1.1
        grade "ex1" "1_1" "-DGRADING_READ" "read"
        TEST_1_1_RESULT=$?
        echo -n "$TEST_1_1_RESULT,"
        
        # Test 2.1
        if [[ $TEST_1_1_RESULT != 0 ]]
        then
            TEST_2_1_RESULT=9
        else
            grade "ex2" "2_1" "-DGRADING_READ" "write"
            TEST_2_1_RESULT=$?
        fi
        echo -n "$TEST_2_1_RESULT,"
        
        # Test 2.2
        if [[ $TEST_2_1_RESULT != 0 ]]
        then
            TEST_2_2_RESULT=9
        else
            grade "ex2" "2_2" "-DGRADING_READ -DGRADING_WRITE" "write"
            TEST_2_2_RESULT=$?
        fi
        echo -n "$TEST_2_2_RESULT,"
        
        # Test 2.3
        grade "ex2" "2_3" "-DGRADING_READ -DGRADING_EFFICIENCY" "read write write_eff"
        TEST_2_3_RESULT=$?
        echo -n "$TEST_2_3_RESULT,"
        
        # Test 2.4
        if [[ $TEST_2_2_RESULT != 0 || $TEST_2_3_RESULT != 0 ]]
        then
            TEST_2_4_RESULT=9
        else
            TEST_2_4_RESULT=0
        fi
        echo -n "$TEST_2_4_RESULT,"
        
        # Test 2.5
        grade "ex2" "2_5" "-DGRADING_READ -DGRADING_WRITE -DGRADING_EFFICIENCY" "read write write_eff"
        TEST_2_5_RESULT=$?
        echo -n "$TEST_2_5_RESULT,"
        
        # Test 2.6
        if [[ $TEST_1_1_RESULT != 0 ]]
        then
            TEST_2_6_RESULT=9
        else
            grade "ex2" "2_6" "-DGRADING_READ" "read_segfault"
            TEST_2_6_RESULT=$?
        fi
        echo -n "$TEST_2_6_RESULT,"
        
        # Test 2.7
        if [[ $TEST_2_2_RESULT != 0 || $TEST_2_6_RESULT != 0 ]]
        then
            TEST_2_7_RESULT=9
        else
            TEST_2_7_RESULT=0
        fi
        echo -n "$TEST_2_7_RESULT,"
        
        # Test 2.8
        if [[ $TEST_2_7_RESULT != 0 ]]
        then
            TEST_2_8_RESULT=9
        else
            grade "ex2" "2_8" "-DGRADING_READ -DGRADING_WRITE" "write_segfault"
            TEST_2_8_RESULT=$?
        fi
        echo -n "$TEST_2_8_RESULT,"
        
        # Test 3.1
        if [[ $TEST_1_1_RESULT != 0 ]]
        then
            TEST_3_1_RESULT=9
        else
            grade "ex3" "3_1" "-DGRADING_READ -DGRADING_MMAP_MUNMAP" "mmap_addonly"
            TEST_3_1_RESULT=$?
        fi
        echo -n "$TEST_3_1_RESULT,"
        
        # Test 3.2
        if [[ $TEST_3_1_RESULT != 0 ]]
        then
            TEST_3_2_RESULT=9
        else
            grade "ex3" "3_2" "-DGRADING_READ -DGRADING_MMAP_MUNMAP -DGRADING_DELETE" "munmap"
            TEST_3_2_RESULT=$?
        fi
        echo -n "$TEST_3_2_RESULT,"
        
        # Test 3.3
        if [[ $TEST_2_8_RESULT != 0 || $TEST_3_2_RESULT != 0 ]]
        then
            TEST_3_3_RESULT=9
        else
            grade "ex3" "3_3" "-DGRADING_READ -DGRADING_WRITE -DGRADING_MMAP_MUNMAP -DGRADING_DELETE" "munmap_full"
            TEST_3_3_RESULT=$?
        fi
        echo -n "$TEST_3_3_RESULT,"
        
        # Test 3.4
        if [[ $TEST_3_2_RESULT != 0 ]]
        then
            TEST_3_4_RESULT=9
        else
            grade "ex3" "3_4" "-DGRADING_READ -DGRADING_MMAP_MUNMAP -DGRADING_DELETE -DGRADING_EFFICIENCY" "read mmap_addonly munmap munmap_eff"
            TEST_3_4_RESULT=$?
        fi
        echo -n "$TEST_3_4_RESULT,"
        
        # Test 3.5
        if [[ $TEST_3_3_RESULT != 0 || $TEST_3_4_RESULT != 0 ]]
        then
            TEST_3_5_RESULT=9
        else
            TEST_3_5_RESULT=0
        fi
        echo -n "$TEST_3_5_RESULT,"
        
        # Test 3.6
        grade "ex3" "3_6" "-DGRADING_READ -DGRADING_WRITE -DGRADING_MMAP_MUNMAP -DGRADING_DELETE -DGRADING_EFFICIENCY" "read write write_eff read_segfault write_segfault mmap_addonly munmap munmap_eff munmap_full munmap_full_eff"
        TEST_3_6_RESULT=$?
        echo -n "$TEST_3_6_RESULT,"
        
        # Test 4.1
        grade "ex4" "4_1" "-DGRADING_READ -DGRADING_WRITE -DGRADING_MMAP_MUNMAP -DGRADING_DELETE -DGRADING_CREATE" "read write read_segfault write_segfault mmap_addonly munmap munmap_full"
        TEST_4_1_RESULT=$?
        echo -n "$TEST_4_1_RESULT,"
        
        # Test 4.2
        if [[ $TEST_4_1_RESULT != 0 ]]
        then
            TEST_4_2_RESULT=9
        else
            grade "ex4" "4_2" "-DGRADING_READ -DGRADING_WRITE -DGRADING_MMAP_MUNMAP -DGRADING_DELETE -DGRADING_CREATE -DGRADING_EFFICIENCY" "read write write_eff read_segfault write_segfault mmap_addonly munmap munmap_eff munmap_full munmap_full_eff"
            TEST_4_2_RESULT=$?
        fi
        echo -n "$TEST_4_2_RESULT,"

        # Get out of the stage directory
        cd ..
        
        # Print newline
        echo ""
    fi
done
yes | rm -r ./stage > /dev/null
