#!/bin/sh
g++ main.cc -m64 -o e64
g++ main.cc -m32 -o e32
g++ main.cc -m64 -std=c++0x -o f64
g++ main.cc -m32 -std=c++0x -o f32
