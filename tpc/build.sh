#!/bin/bash

# Require gcc 4.7+

SRCS="Acceptor.cc InetAddress.cc TcpStream.cc Socket.cc"

set -x

g++ -std=c++11 -Wall -Wextra -g -O2 $SRCS echo.cc -o echo -lpthread

g++ -std=c++11 -Wall -Wextra -g -O2 $SRCS ttcp.cc -o ttcp -lboost_program_options
