#!/bin/bash

g++-4.7 -std=c++0x -Wall -Wextra -g Acceptor.cc InetAddress.cc TcpStream.cc Socket.cc echo.cc -o echo
