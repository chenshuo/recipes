#!/bin/sh

set -x

mkdir -p bin
g++ test.cc uint.cc -o bin/test-dbg -Wall -Wextra -g -O0 \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework
g++ test.cc uint.cc -o bin/test-opt -Wall -Wextra -g -O2 -DNDEBUG \
        -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

g++ bench.cc uint.cc -o bin/bench-dbg -Wall -Wextra -g -O0
g++ bench.cc uint.cc -o bin/bench-opt -Wall -Wextra -g -O2 -DNDEBUG

g++ exact.cc uint.cc -o bin/exact-dbg -Wall -Wextra -g -O0

g++ factorial.cc uint.cc -o bin/factorial-opt -Wall -Wextra -g -O2 -DNDEBUG
g++ fibonacci.cc uint.cc -o bin/fibonacci-opt -Wall -Wextra -g -O2 -DNDEBUG
g++ combination.cc uint.cc -o bin/combination-opt -Wall -Wextra -g -O2 -DNDEBUG
g++ partitions.cc uint.cc -o bin/partition-opt -Wall -Wextra -g -O2 -DNDEBUG
g++ numheaps.cc uint.cc -o bin/numheaps-opt -std=c++0x -Wall -Wextra -g -O2 -DNDEBUG
