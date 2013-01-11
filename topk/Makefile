MUDUO_BUILD ?= release
MUDUO_DIRECTORY ?= $(HOME)/build/$(MUDUO_BUILD)-install
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib
SRC = ../src

CXXFLAGS = -g -O2 -Wall -Wextra -Werror \
		-Wno-unused-parameter -Wconversion\
		-Wold-style-cast -Woverloaded-virtual \
		-Wpointer-arith -Wshadow -Wwrite-strings \
		-march=native -rdynamic \
		-I$(MUDUO_INCLUDE)

LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread

all: sender

clean:
	rm -f sender core

sender: sender.cc
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: all clean
