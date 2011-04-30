CXXFLAGS = -O0 -g  -Wall -I ../.. -pthread
LDFLAGS = -lpthread
BASE_SRC = ../../logging/Logging.cc ../../thread/Thread.cc ../../datetime/Timestamp.cc

LIB_SRC = EventLoop.cc
BINARIES = test1 test2

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

test1: test1.cc
test2: test2.cc

clean:
	rm $(BINARIES) core
