AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = edje_cc
EDJE_FLAGS = -v -id $(top_srcdir)/data -fd $(top_srcdir)/data

filesdir = $(datadir)/elementary/objects

files_DATA = test.edj

EXTRA_DIST = \
test.edc over.png under.png

test.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/test.edc \
	$(top_builddir)/data/objects/test.edj
			
clean-local:
	rm -f *.edj
