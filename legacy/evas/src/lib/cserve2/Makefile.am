MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@EVAS_GENERAL_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@EET_CFLAGS@ \
@FONTCONFIG_CFLAGS@ \
@pthread_cflags@ \
@PIXMAN_CFLAGS@

if EVAS_CSERVE2

noinst_LTLIBRARIES = libevas_cserve2.la libevas_cserve2_utils.la

libevas_cserve2_la_SOURCES = \
evas_cs2.h \
evas_cs2_private.h \
evas_cs2_image_data.c \
evas_cs2_client.c

libevas_cserve2_utils_la_SOURCES = \
evas_cs2_utils.h \
evas_cs2_utils.c

libevas_cserve2_la_LIBADD = @EVAS_GENERAL_LIBS@ libevas_cserve2_utils.la

endif
