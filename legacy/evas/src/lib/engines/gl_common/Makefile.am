## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/local/lib
INCLUDES	     = @freetype_cflags@ @gl_cflags@ \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(includedir) \
                       -I/usr/local/include

if BUILD_ENGINE_GL_COMMON

noinst_LTLIBRARIES      = libevas_engine_gl_common.la
libevas_engine_gl_common_la_SOURCES  = \
evas_gl_image.c

libevas_engine_gl_common_la_LIBADD       = \
@gl_libs@ $(LDFLAGS)

libevas_engine_gl_common_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_gl_image.c
