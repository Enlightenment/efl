MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I. \
-I$(top_srcdir)/src/lib/eina \
-I$(top_builddir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eina_cxx \
-I$(top_builddir)/src/lib/eina_cxx

LDADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LDFLAGS@ @USE_BOOST_SYSTEM_LIBS@ @USE_BOOST_SYSTEM_LDFLAGS@
AM_CXXFLAGS = @USE_BOOST_CPPFLAGS@

SRCS = \
eina_cxx_list_01.cc

EXTRA_PROGRAMS = \
eina_cxx_list_01 \
eina_cxx_thread_01

eina_cxx_list_01_SOURCES = \
eina_cxx_list_01.cc

eina_cxx_thread_01_SOURCES = \
eina_cxx_thread_01.cc

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eina/examples
	$(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eina/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eina/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
