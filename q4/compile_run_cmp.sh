#!/bin/bash

gcc q4_v1.c -o q4 -w -pthread
printf "\nTesting time with ONE thread: \n"
time ./q4 < 1
printf "\nTesting time with TWO threads: \n"
time ./q4 < 2
printf "\nTesting time with FOUR threads: \n"
time ./q4 < 4