## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LIBS_X        = @x_ldflags@ @x_libs@
LIBS_IMLIB2   = @imlib2_libs@
#LIBS_DB       = -ledb
LIBS_GL       = @gl_ldflags@ @gl_libs@
LIBS_TTF      = @ttf_libs@
LIBS_EXTRA    = -L/usr/local/lib

CFLAGS_X      = @x_cflags@
CFLAGS_IMLIB2 = @imlib2_includes@
#CFLAGS_DB     = 
CFLAGS_GL     = @gl_includes@
CFLAGS_TTF    = @ttf_includes@
CFLAGS_EXTRA  = -I$(includedir) -I$(top_srcdir) -I/usr/local/include

LIBS_ALL      = $(LIBS_X) $(LIBS_IMLIB2) $(LIBS_GL) $(LIBS_TTF) $(LIBS_EXTRA)
CFLGAS_ALL    = $(CFLAGS_X) $(CFLAGS_IMLIB2) $(CFLAGS_GL) $(CFLAGS_TTF) $(CFLAGS_EXTRA)

INCLUDES	     = $(CFLGAS_ALL) \
		       -DLIBDIR=\"$(libdir)\" \
		       -DBINDIR=\"$(bindir)\" \
		       -DDATADIR=\"$(datadir)\"

lib_LTLIBRARIES      = libevas.la
include_HEADERS      = \
Evas.h

libevas_la_SOURCES   = \
evas_callbacks.c \
evas_events.c \
evas_gl_routines.c \
evas_gradient.c \
evas_image.c \
evas_image_routines.c \
evas_imlib_routines.c \
evas_line.c \
evas_list.c \
evas_misc.c \
evas_object.c \
evas_rectangle.c \
evas_render.c \
evas_text.c \
evas_gl_routines.h \
evas_x11_routines.c \
evas_image_routines.h \
evas_imlib_routines.h \
evas_x11_routines.h \
Evas.h \
Evas_private.h

libevas_la_LIBADD       = $(LIBS_ALL)
libevas_la_DEPENDENCIES = $(top_builddir)/config.h
libevas_la_LDFLAGS      = -version-info 0:1:0
