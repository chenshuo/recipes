#!/bin/bash

SRCS="Acceptor.cc InetAddress.cc TcpStream.cc Socket.cc"

set -x

g++-4.7 -std=c++0x -Wall -Wextra -g -O2 $SRCS echo.cc -o echo

g++-4.7 -std=c++0x -Wall -Wextra -g -O2 $SRCS ttcp.cc -o ttcp -lboost_program_options
