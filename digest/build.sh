#!/bin/sh

set -x

g++ DigestOOP.cc test_oop.cc -std=c++17 -lcrypto -o oop
g++ DigestOOP2.cc test_oop.cc -std=c++17 -lcrypto -o oop2
g++ test_evp.cc -std=c++17 -lcrypto -o evp
