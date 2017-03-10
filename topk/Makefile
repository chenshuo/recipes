MUDUO_BUILD ?= release
MUDUO_DIRECTORY ?= $(HOME)/build/$(MUDUO_BUILD)-install
MUDUO_INCLUDE = $(MUDUO_DIRECTORY)/include
MUDUO_LIBRARY = $(MUDUO_DIRECTORY)/lib

CXXFLAGS = -g -Og -Wall -Wextra -Werror \
	-Wno-unused-parameter -Wconversion\
	-Wold-style-cast -Woverloaded-virtual \
	-Wpointer-arith -Wshadow -Wwrite-strings \
	-march=native -rdynamic \
	-I$(MUDUO_INCLUDE)

LDFLAGS = -L$(MUDUO_LIBRARY) -lmuduo_net -lmuduo_base -lpthread

BINARIES = sender merger word_freq word_freq_shards word_freq_sort

all: $(BINARIES)

clean:
	rm -f $(BINARIES) core

sender: sender.cc
	g++ $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

merger: merger.cc
	g++ $(CXXFLAGS) -o $@ $^ -lboost_system -lpthread

word_freq: word_freq.cc
	g++ $(CXXFLAGS) -std=c++1y -o $@ $^

word_freq_shards: word_freq_shards.cc
	g++ $(CXXFLAGS) -std=c++11 -o $@ $^

word_freq_sort: word_freq_sort.cc
	g++ $(CXXFLAGS) -std=c++11 -o $@ $^

.PHONY: all clean
