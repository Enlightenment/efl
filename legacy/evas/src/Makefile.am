## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = -L/usr/X11R6/lib -L/usr/local/lib
INCLUDES	     = -I/usr/X11R6/include -I/usr/local/include \
			$(X_CFLAGS) -I$(includedir) \
			-DLIBDIR=\"$(libdir)\" \
			-DBINDIR=\"$(bindir)\" \
			-DDATADIR=\"$(datadir)\"

lib_LTLIBRARIES      = libevas.la
include_HEADERS      = \
Evas.h

libevas_la_SOURCES   = \
evas_bits.c evas_callbacks.c evas_events.c evas_gradient.c evas_image.c \
evas_line.c evas_list.c evas_main.c evas_misc.c evas_object.c \
evas_rectangle.c evas_render.c evas_text.c \
Evas.h

libevas_la_LIBADD       = -lX11 -lXext -lttf -ldl -lm -lImlib2 $(LDFLAGS)
libevas_la_DEPENDENCIES = $(top_builddir)/config.h
libevas_la_LDFLAGS      = -version-info 0:1:0
