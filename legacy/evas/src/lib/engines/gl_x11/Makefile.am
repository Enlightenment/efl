## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = @freetype_cflags@ @x_cflags@ @gl_cflags@ \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include

if BUILD_ENGINE_GL_X11

noinst_LTLIBRARIES      = libevas_engine_gl_x11.la
libevas_engine_gl_x11_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_x_main.c

libevas_engine_gl_x11_la_LIBADD       = @x_libs@ @gl_libs@ $LDFLAGS

libevas_engine_gl_x11_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_x_main.c
