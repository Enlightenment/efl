MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
@EINA_CFLAGS@

pkglib_PROGRAMS = eet_data_simple eet_data_nested eet_data_file_descriptor

eet_data_simple_SOURCES = eet-data-simple.c
eet_data_simple_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_nested_SOURCES = eet-data-nested.c
eet_data_nested_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_file_descriptor_SOURCES = eet-data-file_descriptor.c
eet_data_file_descriptor_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@


filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if INSTALL_EXAMPLES

files_DATA += \
eet-data-simple.c \
eet-data-nested.c \
eet-data-file_descriptor.c

endif
