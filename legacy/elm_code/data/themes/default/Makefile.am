AUTOMAKE_OPTIONS = subdir-objects
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
default.edc

include ../../../../Makefile_Edje_Helper.am

filesdir = $(datadir)/$(PACKAGE)/themes
files_DATA = elm_code.edj

elm_code.edj: Makefile $(EXTRA_DIST)
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
	-id ${top_srcdir}/elm_code/data/themes/default/images \
	-sd ${top_srcdir}/elm_code/data/themes/default/sounds \
	$(top_srcdir)/elm_code/data/themes/default/default.edc \
	$(top_builddir)/elm_code/data/themes/default/elm_code.edj

clean-local:
	rm -f *.edj
