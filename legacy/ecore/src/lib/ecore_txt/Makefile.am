## Process this file with automake to produce Makefile.in

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
@iconv_cflags@

libecore_txt_la_LDFLAGS = -version-info 1:0:0 \
-L$(top_builddir)/src/lib/ecore/.libs

if BUILD_ECORE_TXT

lib_LTLIBRARIES = libecore_txt.la
include_HEADERS = \
Ecore_Txt.h

libecore_txt_la_SOURCES = \
ecore_txt.c \
ecore_txt_private.h

# FIXME: may have to link with -liconv
libecore_txt_la_LIBADD = \
@iconv_libs@


libecore_txt_la_DEPENDENCIES =


endif

EXTRA_DIST = \
Ecore_Txt.h \
ecore_txt.c \
ecore_txt_private.h
