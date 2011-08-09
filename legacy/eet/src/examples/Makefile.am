MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
@EINA_CFLAGS@

pkglib_PROGRAMS = eet_basic eet_file eet_data_simple eet_data_nested eet_data_file_descriptor_01 eet_data_file_descriptor_02 eet_data_cipher_decipher

eet_basic_SOURCES = eet-basic.c
eet_basic_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_file_SOURCES = eet-file.c
eet_file_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_simple_SOURCES = eet-data-simple.c
eet_data_simple_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_nested_SOURCES = eet-data-nested.c
eet_data_nested_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_file_descriptor_01_SOURCES = eet-data-file_descriptor_01.c
eet_data_file_descriptor_01_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_file_descriptor_02_SOURCES = eet-data-file_descriptor_02.c
eet_data_file_descriptor_02_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

eet_data_cipher_decipher_SOURCES = eet-data-cipher_decipher.c
eet_data_cipher_decipher_LDADD = $(top_builddir)/src/lib/libeet.la @EINA_LIBS@

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

if INSTALL_EXAMPLES

files_DATA += \
eet-basic.c \
eet-file.c \
eet-data-simple.c \
eet-data-nested.c \
eet-data-file_descriptor_01.c \
eet-data-file_descriptor_02.c \
eet-data-cipher_decipher.c

endif
