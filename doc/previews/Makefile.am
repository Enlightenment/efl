if EFL_BUILD_DOC

.PHONY: doc

AM_CPPFLAGS = \
	-I$(abs_top_srcdir)/src/lib/eo \
	-I$(abs_top_srcdir)/src/lib/efl \
	-I$(abs_top_srcdir)/src/lib/eina \
	-I$(abs_top_srcdir)/src/lib/evas \
	-I$(abs_top_srcdir)/src/lib/ecore \
	-I$(abs_top_srcdir)/src/lib/ecore_evas \
	-DEFL_BETA_API_SUPPORT=1 \
	-DEFL_EO_API_SUPPORT=1 \
	-D_GNU_SOURCE

LDADD = \
	-L$(abs_top_builddir)/src/lib/eo/.libs \
	-L$(abs_top_builddir)/src/lib/eina/.libs \
	-L$(abs_top_builddir)/src/lib/ecore/.libs \
	-L$(abs_top_builddir)/src/lib/evas/.libs \
	-L$(abs_top_builddir)/src/lib/ecore_evas/.libs \
	-leo \
	-leina \
	-levas \
	-lecore \
	-lecore_evas

if HAVE_WIN32
AM_CPPFLAGS += -I$(abs_top_srcdir)/src/lib/evil @EVIL_CFLAGS@
LDADD += -L$(abs_top_builddir)/src/lib/evil/.libs \
	-levil \
	-ldl
endif

noinst_PROGRAMS = preview_text_filter

DATADIR = ${abs_top_srcdir}/doc/previews/img
HTMLDIR = ${abs_top_srcdir}/doc/html

PREVIEWS = blend blur bump curve grow mask transform \
	example_1

previews-data:
	$(MKDIR_P) $(DATADIR)
	$(MKDIR_P) $(HTMLDIR)
	@for a in $(PREVIEWS) ; do \
		echo "  GEN      filter_$${a}.png" ; \
		FONT="Sans" ; \
		SIZE=24 ; \
		FILTER=`cat ${abs_top_srcdir}/src/examples/evas/filters/filter_$${a}.txt` ; \
		if [ -e ${abs_top_srcdir}/doc/previews/filter_$${a}.sh ] ; then \
			source ${abs_top_srcdir}/doc/previews/filter_$${a}.sh ; \
		fi ; \
		$(abs_top_srcdir)/doc/previews/preview_text_filter "$${TEXT}" "$${FILTER}" "$(DATADIR)/filter_$${a}.png" "$${FONT}" "$${SIZE}" ; \
		cp "$(DATADIR)/filter_$${a}.png" "$(HTMLDIR)/" ; \
	done

doc: Makefile preview_text_filter previews-data

endif

EXTRA_DIST = preview_text_filter.c

clean-local:
	rm -rf $(DATADIR)

