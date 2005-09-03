## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ @xcb_cflags@
		       
if BUILD_ENGINE_SOFTWARE_XCB

noinst_LTLIBRARIES      = libevas_engine_software_xcb.la
libevas_engine_software_xcb_la_SOURCES  = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c

libevas_engine_software_xcb_la_LIBADD       = @xcb_libs@

libevas_engine_software_xcb_la_DEPENDENCIES = \
$(top_builddir)/config.h

endif

EXTRA_DIST = \
evas_engine.h \
evas_engine.c \
evas_outbuf.c \
evas_xcb_buffer.c \
evas_xcb_color.c \
evas_xcb_main.c
