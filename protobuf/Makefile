LIBDIR=/usr/local/lib

CXXFLAGS=-g -Wall -O0 -pthread
LDFLAGS=-lprotobuf -lz -lpthread -Wl,-rpath -Wl,$(LIBDIR)

BINARIES=codec_test descriptor_test
TARGETS=$(BINARIES)

# comment out following line if you have boost installed
TARGETS=codec_test descriptor_test

all: $(TARGETS)

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

codec_test: query.pb.h query.pb.cc codec.h codec_test.cc

descriptor_test: query.pb.h query.pb.cc codec.h descriptor_test.cc

query.pb.h query.pb.cc: query.proto
	protoc --cpp_out . $<

test: codec_test
	./codec_test

clean:
	rm -f query.pb.*
	rm -f $(BINARIES)

