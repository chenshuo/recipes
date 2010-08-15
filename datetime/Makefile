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

all: date_test date_unittest

doc: $(TEX_PNGS) $(PIC_PNGS)

$(PIC_PNGS): PNG_DPI=144

date_test: Date.cc Date_test.cc
	g++ -g -Wall $^ -o $@

date_unittest: Date.cc Date_unittest.cc
	g++ -g -Wall $^ -o $@

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
	rm -f date_unittest

clean-all: clean
	rm -f *.png
	rm -f date_test
