LIBDIR=/usr/local/lib

CXXFLAGS=-g -Wall -O2 -pthread
#LDFLAGS=-lprotobuf -lz -lpthread -Wl,-rpath -Wl,$(LIBDIR)

SRCS=x
BINARIES=cwc

all: $(BINARIES)

cwc: cwc.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

clean:
	rm -f $(BINARIES)

