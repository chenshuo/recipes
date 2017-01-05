#!/bin/bash

# Require gcc 4.7+

SRCS="Acceptor.cc InetAddress.cc TcpStream.cc Socket.cc"

set -x
CC=${CC:-g++}
CXXFLAGS="-std=c++11 -Wall -Wextra -g -O2"

$CC $CXXFLAGS $SRCS chargen.cc ../datetime/Timestamp.cc -o chargen -lpthread

$CC $CXXFLAGS $SRCS discard.cc ../datetime/Timestamp.cc -o discard -lpthread

$CC $CXXFLAGS $SRCS echo.cc -o echo -lpthread

$CC $CXXFLAGS $SRCS echo_client.cc -o echo_client

$CC $CXXFLAGS $SRCS netcat.cc -o netcat -lpthread

$CC $CXXFLAGS $SRCS nodelay.cc -o nodelay

$CC $CXXFLAGS $SRCS nodelay_server.cc -o nodelay_server

$CC $CXXFLAGS $SRCS roundtrip_udp.cc -o roundtrip_udp -lpthread

$CC $CXXFLAGS $SRCS sender.cc -o sender -lpthread

$CC $CXXFLAGS $SRCS sudoku_stress.cc ../datetime/Timestamp.cc -o sudoku_stress -lpthread

$CC $CXXFLAGS $SRCS ttcp.cc -o ttcp -lboost_program_options

