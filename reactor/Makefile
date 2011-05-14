SUBDIRS = s00 s01 s02 s03 s04 s05 s06 s07 s08

all: $(SUBDIRS)

clean:
	for d in $(SUBDIRS); do cd $$d && make clean && cd ..; done

$(SUBDIRS):
	cd $@ && make

.PHONY: all clean $(SUBDIRS)
