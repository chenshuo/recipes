CXXFLAGS=-g -Wall -pthread

BINARIES=icmpecho rejectall acceptall discardall discardall2 echoall echoall2 connectmany

all: $(BINARIES)

icmpecho: icmpecho.cc faketcp.cc
rejectall: rejectall.cc faketcp.cc
acceptall: acceptall.cc faketcp.cc
discardall: discardall.cc faketcp.cc
discardall2: discardall2.cc faketcp.cc
discardall2: CXXFLAGS += -std=c++0x
echoall: echoall.cc faketcp.cc
echoall2: echoall2.cc faketcp.cc
echoall2: CXXFLAGS += -std=c++0x
connectmany: connectmany.cc faketcp.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

clean:
	rm -f $(BINARIES)

