## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/local/lib
INCLUDES	     = \
                       -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       -I$(includedir) \
                       -I/usr/local/include \
		       @freetype_cflags@ @x_cflags@

if BUILD_ENGINE_SOFTWARE_X11

noinst_LTLIBRARIES      = libevas_engine_software_x11.la
libevas_engine_software_x11_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_x_buffer.c \
evas_x_color.c \
evas_x_main.c

libevas_engine_software_x11_la_LIBADD       = \
@x_libs@ $(LDFLAGS)

libevas_engine_software_x11_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_x_buffer.c \
evas_x_color.c \
evas_x_main.c
