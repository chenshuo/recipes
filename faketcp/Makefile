CXXFLAGS=-g -Wall -pthread

BINARIES=icmpecho rejectall acceptall

all: $(BINARIES)

icmpecho: icmpecho.cc faketcp.cc
rejectall: rejectall.cc faketcp.cc
acceptall: acceptall.cc faketcp.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

clean:
	rm -f $(BINARIES)

