CXXFLAGS=-g -Wall -O0

PNG_DPI=300
TEXES=$(wildcard *.tex)
PICS=$(wildcard *.pic)
TEX_PNGS=$(subst tex,png,$(TEXES))
PIC_PNGS=$(subst pic,png,$(PICS))

%.eps : %.pic
	gpic $< | groff | ps2eps --loose --gsbbox > $@

%.eps : %.tex
	latex $<
	dvips $(basename $<)
	ps2eps --loose --gsbbox < $(basename $<).ps > $(basename $<).eps

%.png : %.eps
	eps2png --png16m -resolution $(PNG_DPI) $<

BINARIES=date_test date_unittest timestamp_unittest timezone_dump timezone_unittest

all: $(BINARIES)

doc: $(TEX_PNGS) $(PIC_PNGS)

$(PIC_PNGS): PNG_DPI=144

$(BINARIES):
	g++ $(CXXFLAGS) $(filter %.cc,$^) -o $@

date_test: Date.h Date.cc Date_test.cc

date_unittest: Date.h Date.cc Date_unittest.cc

timestamp_unittest: Timestamp.h Timestamp.cc Timestamp_unittest.cc

timezone_dump: TimeZone.h TimeZone.cc Date.cc TimeZone_dump.cc

timezone_unittest: TimeZone.h TimeZone.cc Date.cc TimeZone_unittest.cc

test: date_unittest
	./date_unittest

clean:
	rm -f *.aux
	rm -f *.dvi
	rm -f *.eps
	rm -f *.log
	rm -f *.mps
	rm -f *.mpx
	rm -f *.ps
	rm -f *_unittest

clean-all: clean
	rm -f *.png
	rm -f $(BINARIES)
