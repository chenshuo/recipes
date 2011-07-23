CXXFLAGS = -O0 -g  -Wall -I ../.. -pthread
LDFLAGS = -lpthread -lmuduo -L.
BASE_SRC = ../../logging/Logging.cc ../../logging/LogStream.cc ../../thread/Thread.cc ../../datetime/Timestamp.cc
MUDUO_SRC = $(notdir $(LIB_SRC) $(BASE_SRC))
OBJS = $(patsubst %.cc,%.o,$(MUDUO_SRC))

libmuduo.a: $(BASE_SRC) $(LIB_SRC)
	g++ $(CXXFLAGS) -c $^
	ar rcs $@ $(OBJS)

$(BINARIES): libmuduo.a
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) *.o *.a core

