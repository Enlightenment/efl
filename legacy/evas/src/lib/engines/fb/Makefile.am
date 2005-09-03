## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@
		       

if BUILD_ENGINE_FB

noinst_LTLIBRARIES      = libevas_engine_fb.la
libevas_engine_fb_la_SOURCES  = \
evas_engine.h \
evas_fb.h \
evas_engine.c \
evas_fb_main.c \
evas_outbuf.c

libevas_engine_fb_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_fb.h \
evas_engine.c \
evas_fb_main.c \
evas_outbuf.c
