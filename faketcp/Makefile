CXXFLAGS=-g -Wall -pthread

BINARIES=icmpecho

all: $(BINARIES)

icmpecho: icmpecho.cc

$(BINARIES):
	g++ $(CXXFLAGS) $(LDFLAGS) $(filter %.cc,$^) -o $@

clean:
	rm -f $(BINARIES)

