
CXXFLAGS = -std=c++11 -pthread -O2 -iquote include/ -I../
LDFLAGS = -L lib/ -ltpc

LIB_SRCS := $(wildcard lib/*.cc) ../datetime/Timestamp.cc
LIB_OBJS := $(LIB_SRCS:.cc=.o)
LIB := lib/libtpc.a

BIN_SRCS := $(wildcard bin/*.cc)
BINS := $(BIN_SRCS:.cc=)

all: $(BINS)

$(LIB_OBJS) : Makefile

$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

$(BINS): $(LIB)

bin/ttcp: LDFLAGS += -lboost_program_options

clean:
	rm -f $(LIB_OBJS) $(LIB) $(BINS)

