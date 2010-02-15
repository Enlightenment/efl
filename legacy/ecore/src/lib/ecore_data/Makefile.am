MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@ \
@WIN32_CPPFLAGS@ \
@EFL_ECORE_BUILD@

if BUILD_ECORE_DATA

lib_LTLIBRARIES = libecore_data.la
include_HEADERS = \
Ecore_Data.h \
Ecore_Str.h

libecore_data_la_SOURCES = \
ecore_hash.c \
ecore_list.c \
ecore_path.c \
ecore_plugin.c \
ecore_sheap.c \
ecore_strbuf.c \
ecore_str.c \
ecore_tree.c \
ecore_value.c

libecore_data_la_LIBADD = @dlopen_libs@ @EINA_LIBS@ @EVIL_LIBS@ @WIN32_LIBS@ @LTLIBINTL@ @EFL_PTHREAD_LIBS@ -lm
libecore_data_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @ecore_release_info@

endif
