AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

include ../../Makefile_Elm_Helpers.am

EDJE_CC_FLAGS += -id $(top_srcdir)/data/objects -fd $(top_srcdir)/data/objects

filesdir = $(datadir)/elementary/objects

files_DATA = test.edj test_external.edj multip.edj cursors.edj font_preview.edj postit_ent.edj multibuttonentry.edj test_prefs.edj test_prefs.epb test_focus_style.edj

EXTRA_DIST = \
test.edc \
test_external.edc \
test_prefs.edc \
test_prefs.epc \
multip.edc \
cursors.edc \
font_preview.edc \
postit_ent.edc \
multibuttonentry.edc \
test_focus_style.edc \
over.png \
under.png \
sky.jpg \
clo.png \
cross.png \
border.png \
border2.png \
border3.png \
border4.png \
border5.png \
border6.png

test.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/test.edc \
	$(top_builddir)/data/objects/test.edj

test_external.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/test_external.edc \
	$(top_builddir)/data/objects/test_external.edj

multip.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/multip.edc \
	$(top_builddir)/data/objects/multip.edj

colorpreview.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/colorpreview.edc \
	$(top_builddir)/data/objects/colorpreview.edj

cursors.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/cursors.edc \
	$(top_builddir)/data/objects/cursors.edj

font_preview.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/font_preview.edc \
	$(top_builddir)/data/objects/font_preview.edj

postit_ent.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/postit_ent.edc \
	$(top_builddir)/data/objects/postit_ent.edj
multibuttonentry.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/multibuttonentry.edc \
	$(top_builddir)/data/objects/multibuttonentry.edj

test_prefs.edj: Makefile test_prefs.edc
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	$(top_srcdir)/data/objects/test_prefs.edc \
	$(top_builddir)/data/objects/test_prefs.edj

test_prefs.epb: Makefile test_prefs.epc
	$(AM_V_EPB)$(ELM_PREFS_CC) $(ELM_PREFS_FLAGS) \
	$(top_srcdir)/data/objects/test_prefs.epc \
	$(top_builddir)/data/objects/test_prefs.epb

test_focus_style.edj: Makefile test_focus_style.edc
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	-id $(top_srcdir)/data/themes/img \
	$(top_srcdir)/data/objects/test_focus_style.edc \
	$(top_builddir)/data/objects/test_focus_style.edj

clean-local:
	rm -f *.edj
	rm -f test_prefs.epb
