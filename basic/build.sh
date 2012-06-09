#!/bin/sh

mkdir -p bin
g++ test.cc uint.cc -o bin/test-dbg -Wall -Wextra -g -O0 \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ test.cc uint.cc -o bin/test-opt -Wall -Wextra -g -O2 -DNDEBUG \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

g++ bench.cc uint.cc -o bin/bench-dbg -Wall -Wextra -g -O0
g++ bench.cc uint.cc -o bin/bench-opt -Wall -Wextra -g -O2 -DNDEBUG
