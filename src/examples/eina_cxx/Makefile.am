MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/efl/interfaces \
@EINA_CXX_CFLAGS@ @CHECK_CFLAGS@ @EINA_CFLAGS@ @EO_CFLAGS@

LDADD = $(top_builddir)/src/lib/eina/libeina.la @EINA_LDFLAGS@ @EFL_PTHREAD_LIBS@

SRCS = \
eina_cxx_list_01.cc \
eina_cxx_thread_01.cc

EXTRA_PROGRAMS = \
eina_cxx_list_01 \
eina_cxx_thread_01

eina_cxx_list_01_SOURCES = \
eina_cxx_list_01.cc

eina_cxx_thread_01_SOURCES = \
eina_cxx_thread_01.cc

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eina_cxx/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eina_cxx/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eina_cxx/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
