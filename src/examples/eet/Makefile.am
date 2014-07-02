MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eet \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eet \
@EET_CFLAGS@

EXTRA_PROGRAMS = eet_basic eet_file eet_data_simple eet_data_nested eet_data_file_descriptor_01 eet_data_file_descriptor_02 eet_data_cipher_decipher

eet_basic_SOURCES = eet-basic.c
eet_basic_LDADD = $(top_builddir)/src/lib/eet/libeet.la @EET_LDFLAGS@

eet_file_SOURCES = eet-file.c
eet_file_LDADD = $(top_builddir)/src/lib/eet/libeet.la @EET_LDFLAGS@

eet_data_simple_SOURCES = eet-data-simple.c
eet_data_simple_LDADD = $(top_builddir)/src/lib/eet/libeet.la $(top_builddir)/src/lib/eina/libeina.la @EET_LDFLAGS@

eet_data_nested_SOURCES = eet-data-nested.c
eet_data_nested_LDADD = $(top_builddir)/src/lib/eet/libeet.la $(top_builddir)/src/lib/eina/libeina.la @EET_LDFLAGS@

eet_data_file_descriptor_01_SOURCES = eet-data-file_descriptor_01.c
eet_data_file_descriptor_01_LDADD = $(top_builddir)/src/lib/eet/libeet.la $(top_builddir)/src/lib/eina/libeina.la @EET_LDFLAGS@

eet_data_file_descriptor_02_SOURCES = eet-data-file_descriptor_02.c
eet_data_file_descriptor_02_LDADD = $(top_builddir)/src/lib/eet/libeet.la $(top_builddir)/src/lib/eina/libeina.la @EET_LDFLAGS@

eet_data_cipher_decipher_SOURCES = eet-data-cipher_decipher.c
eet_data_cipher_decipher_LDADD = $(top_builddir)/src/lib/eet/libeet.la @EET_LDFLAGS@

SRCS = \
eet-basic.c \
eet-file.c \
eet-data-simple.c \
eet-data-nested.c \
eet-data-file_descriptor_01.c \
eet-data-file_descriptor_02.c \
eet-data-cipher_decipher.c

DATA_FILES = Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/eet/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(datadir)/eet/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(datadir)/eet/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
