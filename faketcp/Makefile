CXXFLAGS=-g -Wall -pthread

BINARIES=icmpecho rejectall acceptall discardall connectmany

all: $(BINARIES)

icmpecho: icmpecho.cc faketcp.cc
rejectall: rejectall.cc faketcp.cc
acceptall: acceptall.cc faketcp.cc
discardall: discardall.cc faketcp.cc
echoall: echoall.cc faketcp.cc
connectmany: connectmany.cc faketcp.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

clean:
	rm -f $(BINARIES)

