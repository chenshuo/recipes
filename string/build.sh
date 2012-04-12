#!/bin/sh

set -x
g++ StringEager.cc main.cc -Wall -m64 -o e64 && ./e64
g++ StringEager.cc main.cc -Wall -m32 -o e32 && ./e32
g++ StringEager.cc main.cc -Wall -m64 -std=c++0x -o f64 && ./f64
g++ StringEager.cc main.cc -Wall -m32 -std=c++0x -o f32 && ./f32

g++ StringEager.cc test.cc -Wall -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework -m64 -o w64 && ./w64
g++ StringEager.cc test.cc -Wall -m32 -o w32 && ./w32
g++ StringEager.cc test.cc -Wall -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework -m64 -std=c++0x -o u64 && ./u64
g++ StringEager.cc test.cc -Wall -m32 -std=c++0x -o u32 && ./u32

