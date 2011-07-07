AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS_VERBOSE_ =
EDJE_FLAGS_VERBOSE_0 =
EDJE_FLAGS_VERBOSE_1 = -v
EDJE_FLAGS = $(EDJE_FLAGS_$(V)) -id $(top_srcdir)/data/objects -fd $(top_srcdir)/data/objects

filesdir = $(datadir)/elementary/objects

files_DATA = test.edj multip.edj colorpreview.edj cursors.edj font_preview.edj postit_ent.edj

EXTRA_DIST = \
test.edc \
multip.edc \
colorpreview.edc \
cursors.edc \
font_preview.edc \
postit_ent.edc \
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

cursors.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/cursors.edc \
	$(top_builddir)/data/objects/cursors.edj

font_preview.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/font_preview.edc \
	$(top_builddir)/data/objects/font_preview.edj

postit_ent.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/objects/postit_ent.edc \
	$(top_builddir)/data/objects/postit_ent.edj

clean-local:
	rm -f *.edj
