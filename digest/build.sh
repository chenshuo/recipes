#!/bin/sh

set -x

g++ DigestOOP.cc test_oop.cc -std=c++17 -lcrypto -o oop
g++ DigestOOP2.cc test_oop.cc -std=c++17 -lcrypto -o oop2
g++ DigestTMP.cc test_oop.cc -std=c++17 -lcrypto -o tmp
g++ test_evp.cc -std=c++17 -lcrypto -o evp
g++ test_evp2.cc -std=c++17 -lcrypto -o evp2
g++ bench.cc DigestOOP.cc -std=c++17 -lbenchmark -lpthread -lcrypto -O2 -o bench

