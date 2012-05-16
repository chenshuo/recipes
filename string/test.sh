#!/bin/sh
g++ StringEager.cc test.cc -Wall -g -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework -m64 -std=c++0x -o u64
./u64

