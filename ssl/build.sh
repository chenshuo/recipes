#!/bin/bash

set -x
CXXFLAGS="-Wall -Wextra -Wno-unused-parameter -I /usr/local/include/ -I $HOME/muduo -O2 -g"
LDFLAGS="-L /usr/local/lib/ -l tls -l ssl -l crypto -lrt"
LIB="TlsAcceptor.cc TlsConfig.cc TlsStream.cc ../tpc/lib/InetAddress.cc ../tpc/lib/Socket.cc ../logging/Logging.cc ../logging/LogStream.cc ../datetime/Timestamp.cc ../thread/Thread.cc"

g++ $CXXFLAGS -iquote ../tpc/include -I../ -std=c++11 -pthread $LIB server.cc -o server $LDFLAGS
g++ $CXXFLAGS -iquote ../tpc/include -I../ -std=c++11 -pthread $LIB client.cc -o client $LDFLAGS

g++ $CXXFLAGS benchmark-libressl.cc -o benchmark-libressl $LDFLAGS
g++ -Wall -O2 -g benchmark-openssl.cc -o benchmark-openssl -lssl -lcrypto -lrt

g++ -Wall -Wno-deprecated-declarations -O2 -g footprint-openssl.cc -o footprint-openssl -lssl -lcrypto -lrt

g++ $CXXFLAGS loop-libressl.cc -o loop-libressl -iquote ../ -pthread $LDFLAGS ../thread/Thread.cc


