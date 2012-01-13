#!/bin/sh

set -x

CXXFLAGS="-O2 -g -Wall"
export CXXFLAGS
g++ -I .. $CXXFLAGS -o sort00 sort00.cc ../datetime/Timestamp.cc
g++ -I .. $CXXFLAGS -o sort01 sort01.cc ../datetime/Timestamp.cc
g++ -I .. $CXXFLAGS -o sort02 sort02.cc ../datetime/Timestamp.cc
g++ -I .. $CXXFLAGS -o sort03 sort03.cc ../datetime/Timestamp.cc ../thread/Thread.cc ../thread/ThreadPool.cc ../thread/Exception.cc -lpthread
g++ -I .. $CXXFLAGS -o sort04 sort04.cc ../datetime/Timestamp.cc ../thread/Thread.cc ../thread/ThreadPool.cc ../thread/Exception.cc -lpthread

g++ -I .. $CXXFLAGS -o sort10 sort10.cc ../datetime/Timestamp.cc
g++ -I .. $CXXFLAGS -o sort11 sort11.cc ../datetime/Timestamp.cc ../thread/Thread.cc ../thread/ThreadPool.cc ../thread/Exception.cc -lpthread
g++ -I .. $CXXFLAGS -o sort12 sort12.cc ../datetime/Timestamp.cc ../thread/Thread.cc ../thread/ThreadPool.cc ../thread/Exception.cc -lpthread
