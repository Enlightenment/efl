if BUILD_GUI
if EFL_BUILD_DOC

.PHONY: doc

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/efl \
-I. \
-I$(builddir) \
-I$(top_srcdir)/src/lib/evil \
-I$(top_builddir)/src/lib/evil \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_builddir)/src/lib/eo \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eet \
-I$(top_srcdir)/src/lib/evas \
-I$(top_builddir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_evas \
-DEFL_BETA_API_SUPPORT=1 \
-DEFL_EO_API_SUPPORT=1 \
@ECORE_EVAS_LDFLAGS@

LDADD = \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eet/libeet.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
@ECORE_EVAS_LDFLAGS@

noinst_PROGRAMS = preview_text_filter

DATADIR = ${abs_top_srcdir}/doc/previews/img
HTMLDIR = ${abs_top_srcdir}/doc/html

PREVIEWS = blend blur bump curve grow mask transform \
example_1 padding

previews-data: preview_text_filter
	$(mkdir_p) $(DATADIR)
	$(mkdir_p) $(HTMLDIR)
	@for a in $(PREVIEWS) ; do \
		echo "  GEN      filter_$${a}.png" ; \
		FONT="Sans" ; \
		SIZE=24 ; \
		TEXT="TEXT" ; \
		FILTER=`cat ${top_srcdir}/src/examples/evas/filters/filter_$${a}.lua` ; \
		if [ -e ${top_srcdir}/doc/previews/filter_$${a}.sh ] ; then \
			source ${top_srcdir}/doc/previews/filter_$${a}.sh ; \
		fi ; \
		$(top_builddir)/doc/previews/preview_text_filter "$${TEXT}" "$${FILTER}" "$(DATADIR)/filter_$${a}.png" "$${FONT}" "$${SIZE}" ; \
		cp "$(DATADIR)/filter_$${a}.png" "$(HTMLDIR)/" ; \
	done

if BUILD_EFL_NATIVE
doc: Makefile previews-data
endif


endif
endif

EXTRA_DIST = preview_text_filter.c

clean-local:
	rm -rf $(DATADIR)
