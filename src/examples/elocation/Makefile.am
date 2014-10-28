MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/eldbus \
-I$(top_srcdir)/src/lib/elocation \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/eldbus \
-I$(top_builddir)/src/lib/elocation

EXTRA_PROGRAMS = \
elocation

elocation_SOURCES = elocation.c
elocation_LDADD = \
$(top_builddir)/src/lib/elocation/libelocation.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eldbus/libeldbus.la \
$(top_builddir)/src/lib/eina/libeina.la \
@ELOCATION_LDFLAGS@

SRCS = \
elocation.c

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/elocation/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/elocation/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/elocation/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
