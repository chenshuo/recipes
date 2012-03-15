#!/bin/sh

set -x
g++ main.cc -m64 -o e64 && ./e64
g++ main.cc -m32 -o e32 && ./e32
g++ main.cc -m64 -std=c++0x -o f64 && ./f64
g++ main.cc -m32 -std=c++0x -o f32 && ./f32


