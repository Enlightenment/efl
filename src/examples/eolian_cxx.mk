EOL_CXX_IMPL = \
eolian_cxx/colourable.c \
eolian_cxx/colourable_cxx.cc \
eolian_cxx/colourablesquare.c \
eolian_cxx/colourablesquare_cxx.cc

EOL_CXX_SRCS = \
eolian_cxx/eolian_cxx_simple_01.cc \
eolian_cxx/eolian_cxx_callbacks_01.cc \
eolian_cxx/ns_colourable.eo \
eolian_cxx/ns_colourablesquare.eo \
$(IMPL)

if HAVE_CXX

EOL_CXX_GENERATED = \
eolian_cxx/ns_colourable.eo.c \
eolian_cxx/ns_colourable.eo.h \
eolian_cxx/ns_colourable.eo.hh \
eolian_cxx/ns_colourable.eo.impl.hh \
eolian_cxx/ns_colourablesquare.eo.c \
eolian_cxx/ns_colourablesquare.eo.h \
eolian_cxx/ns_colourablesquare.eo.hh \
eolian_cxx/ns_colourablesquare.eo.impl.hh

BUILT_SOURCES += $(EOL_CXX_GENERATED)

EXTRA_PROGRAMS += \
eolian_cxx_eolian_cxx_simple_01 \
eolian_cxx_eolian_cxx_simple_01_cxx_impl \
eolian_cxx_eolian_cxx_callbacks_01

eolian_cxx_eolian_cxx_simple_01_SOURCES = \
eolian_cxx/eolian_cxx_simple_01.cc \
eolian_cxx/colourable.c \
eolian_cxx/colourablesquare.c
eolian_cxx_eolian_cxx_simple_01_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_builddir)/src/examples/eolian_cxx

eolian_cxx_eolian_cxx_simple_01_cxx_impl_SOURCES = \
eolian_cxx/eolian_cxx_simple_01.cc \
eolian_cxx/colourable.c \
eolian_cxx/colourablesquare.c \
eolian_cxx/colourable_cxx.cc \
eolian_cxx/colourablesquare_stub.h \
eolian_cxx/colourable_stub.h \
eolian_cxx/colourablesquare_cxx.cc
eolian_cxx_eolian_cxx_simple_01_cxx_impl_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_builddir)/src/examples/eolian_cxx

eolian_cxx_eolian_cxx_callbacks_01_SOURCES = \
eolian_cxx/eolian_cxx_callbacks_01.cc
eolian_cxx_eolian_cxx_callbacks_01_CPPFLAGS = \
$(AM_CPPFLAGS) \
-I$(top_builddir)/src/examples/eolian_cxx

endif

EOL_CXX_DATA_FILES = eolian_cxx/Makefile.examples $(ECXX_EXAMPLE_EOS) $(EOL_CXX_SRCS)
DATA_FILES += $(EOL_CXX_DATA_FILES)

install-examples-eolcxx:
	$(MKDIR_P) $(DESTDIR)$(datadir)/eolian_cxx/examples
	cd $(abs_srcdir) && $(install_sh_DATA) -c $(EOL_CXX_SRCS) $(EOL_CXX_DATA_FILES) $(DESTDIR)$(datadir)/eolian_cxx/examples

uninstall-local-eolcxx:
	for f in $(EOL_CXX_SRCS) $(EOL_CXX_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/eolian_cxx/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-eolcxx
UNINSTALL_EXAMPLES += uninstall-local-eolcxx
