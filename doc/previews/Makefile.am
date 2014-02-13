if EFL_BUILD_DOC

.PHONY: doc

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib/eo \
	-I$(top_srcdir)/src/lib/efl \
	-I$(top_srcdir)/src/lib/eina \
	-I$(top_srcdir)/src/lib/evas \
	-I$(top_srcdir)/src/lib/ecore \
	-I$(top_srcdir)/src/lib/ecore_evas \
	-DEFL_BETA_API_SUPPORT=1 \
	-DEFL_EO_API_SUPPORT=1 \
	-D_GNU_SOURCE

LDADD = \
	-L$(top_srcdir)/src/lib/eo/.libs \
	-L$(top_srcdir)/src/lib/eina/.libs \
	-L$(top_srcdir)/src/lib/ecore/.libs \
	-L$(top_srcdir)/src/lib/evas/.libs \
	-L$(top_srcdir)/src/lib/ecore_evas/.libs \
        -leo \
	-leina \
	-levas \
	-lecore \
	-lecore_evas

noinst_PROGRAMS = preview_text_filter

DATADIR = ${top_srcdir}/doc/previews/img
HTMLDIR = ${top_srcdir}/doc/html

PREVIEWS = blend blur bump curve grow mask transform \
	example_1

previews-data:
	$(MKDIR_P) $(DATADIR)
	$(MKDIR_P) $(HTMLDIR)
	@for a in $(PREVIEWS) ; do \
		echo "  GEN      filter_$${a}.png" ; \
		FONT="Sans" ; \
		SIZE=24 ; \
		FILTER=`cat ${top_srcdir}/src/examples/evas/filters/filter_$${a}.txt` ; \
		if [ -e ${top_srcdir}/doc/previews/filter_$${a}.sh ] ; then \
			source ${top_srcdir}/doc/previews/filter_$${a}.sh ; \
		fi ; \
		$(top_srcdir)/doc/previews/preview_text_filter "$${TEXT}" "$${FILTER}" "$(DATADIR)/filter_$${a}.png" "$${FONT}" "$${SIZE}" ; \
		cp "$(DATADIR)/filter_$${a}.png" "$(HTMLDIR)/" ; \
	done

doc: Makefile preview_text_filter previews-data

endif

EXTRA_DIST = preview_text_filter.c

clean-local:
	rm -rf $(DATADIR)

