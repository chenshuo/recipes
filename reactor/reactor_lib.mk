CXXFLAGS = -O0 -g  -Wall -I ../.. -pthread
LDFLAGS = -lpthread -lmuduo -L.
BASE_SRC = ../../logging/Logging.cc ../../thread/Thread.cc ../../datetime/Timestamp.cc

libmuduo.a: $(BASE_SRC) $(LIB_SRC)
	g++ $(CXXFLAGS) -c $^
	ar rcs $@ $(wildcard *.o)

$(BINARIES): libmuduo.a
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) *.o *.a core

