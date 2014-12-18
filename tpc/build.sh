#!/bin/bash

# Require gcc 4.7+

SRCS="Acceptor.cc InetAddress.cc TcpStream.cc Socket.cc"

set -x
CC=${CC:-g++}

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS chargen.cc ../datetime/Timestamp.cc -o chargen -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS discard.cc ../datetime/Timestamp.cc -o discard -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS echo.cc -o echo -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS echo_client.cc -o echo_client

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS netcat.cc -o netcat -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS nodelay.cc -o nodelay

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS nodelay_server.cc -o nodelay_server

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS roundtrip_udp.cc -o roundtrip_udp -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS sender.cc -o sender -lpthread

$CC -std=c++11 -Wall -Wextra -g -O2 $SRCS ttcp.cc -o ttcp -lboost_program_options

