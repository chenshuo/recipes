#!/bin/sh

g++ test.cc uint.cc -o test-dbg -Wall -Wextra -g -O0 \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ test.cc uint.cc -o test-opt -Wall -Wextra -g -O2 -DNDEBUG \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

g++ bench.cc uint.cc -o bench-dbg -Wall -Wextra -g -O0
g++ bench.cc uint.cc -o bench-opt -Wall -Wextra -g -O2 -DNDEBUG
