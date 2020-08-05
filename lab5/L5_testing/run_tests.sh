#!/bin/bash

TIMEOUT=5

make
make -f Grading.mk

# Q1: Testing if the read and write operations are buffered
{ timeout $TIMEOUT ./read_buffered; } &> q1_read
{ timeout $TIMEOUT  ./write_buffered; } &> q1_write

# Q2: Testing the correctness of read operations. Results are considered only if reads are buffered
{ timeout $TIMEOUT  ./read_operations; } &> q2_op
{ timeout $TIMEOUT  ./read_misc 1; } &> q2_misc_1
{ timeout $TIMEOUT  ./read_misc 2; } &> q2_misc_2
{ timeout $TIMEOUT  ./read_misc 3; } &> q2_misc_3

# Q3: Testing the correctness of write operations. Results are considered only if writes are buffered
{ timeout $TIMEOUT  ./write_operations; } &> q3_op
{ timeout $TIMEOUT  ./write_misc; } &> q3_misc

# Q4: Testing correctness for my_fflush() and my_fseek()
# Considered if reads are buffered
{ timeout $TIMEOUT  ./fflush 1; } &> q4_fflush_1
{ timeout $TIMEOUT  ./fseek 1; } &> q4_fseek_1
# Considered if writes are buffered
{ timeout $TIMEOUT  ./fflush 2; } &> q4_fflush_2
{ timeout $TIMEOUT  ./fseek 2; } &> q4_fseek_2