MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = edje_cc
EDJE_FLAGS = -v \
	-id $(top_srcdir)/data/frames/images

filesdir = $(themesdir)
files_DATA = default.edj

EXTRA_DIST = \
	default.edc

default.edj: Makefile $(EXTRADIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
		$(top_srcdir)/data/frames/default.edc \
		$(top_builddir)/data/frames/default.edj

clean-local:
	rm -f *.edj
