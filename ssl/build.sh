#!/bin/bash

set -x
CXXFLAGS="-I /usr/local/include/ -I $HOME/muduo"
LDFLAGS="-L /usr/local/lib/ -l tls -l ssl -l crypto"
LIB="TlsConfig.cc TlsStream.cc ../logging/Logging.cc ../logging/LogStream.cc ../datetime/Timestamp.cc ../thread/Thread.cc"
g++ $CXXFLAGS -iquote ../tpc/include -I../ -std=c++11 -pthread $LIB client.cc -o client $LDFLAGS

g++ $CXXFLAGS loop-libressl.cc -o loop-libressl $LDFLAGS
g++ $CXXFLAGS benchmark-libressl.cc -o benchmark-libressl $LDFLAGS
