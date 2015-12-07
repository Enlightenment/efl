MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/eflat_xml_model \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/eflat_xml_model \
@EFLAT_XML_MODEL_CFLAGS@

EXAMPLES_LIBS = \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/eflat_xml_model/libeflat_xml_model.la \
$(top_builddir)/src/lib/efl/libefl.la \
@EFLAT_XML_MODEL_LDFLAGS@

SRCS = \
xml_model.c

EXTRA_PROGRAMS = \
xml_model

xml_model_SOURCES = xml_model.c
xml_model_LDADD = $(EXAMPLES_LIBS)

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	$(MKDIR_P) $(datadir)/eflat_xml_model/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eflat_xml_model/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eflat_xml_model/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
