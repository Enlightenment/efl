AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/objects -fd $(top_srcdir)/data/objects

filesdir = $(datadir)/elementary/objects

files_DATA = test.edj multip.edj colorpreview.edj

EXTRA_DIST = \
test.edc \
multip.edc \
colorpreview.edc \
grid_bg.png \
over.png \
under.png \
sky.jpg \
clo.png \
cross.png

test.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/test.edc \
	$(top_builddir)/data/objects/test.edj
			
multip.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/multip.edc \
	$(top_builddir)/data/objects/multip.edj

colorpreview.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/colorpreview.edc \
	$(top_builddir)/data/objects/colorpreview.edj
			
clean-local:
	rm -f *.edj
