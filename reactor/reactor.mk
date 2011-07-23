CXXFLAGS = -O0 -g  -Wall -I ../.. -pthread
LDFLAGS = -lpthread
BASE_SRC = ../../logging/Logging.cc ../../logging/LogStream.cc ../../thread/Thread.cc ../../datetime/Timestamp.cc

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

