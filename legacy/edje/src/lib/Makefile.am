## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     =
INCLUDES	     = \
-I$(top_srcdir) \
-I$(top_builddir) \
-I$(includedir) \
@eet_cflags@ \
@evas_cflags@ \
@ecore_cflags@

lib_LTLIBRARIES      = \
libedje.la \
libedje_edit.la

include_HEADERS      =  \
Edje.h \
Edje_Edit.h

libedje_la_SOURCES  = \
edje_calc.c \
edje_callbacks.c \
edje_data.c \
edje_load.c \
edje_main.c \
edje_program.c \
edje_smart.c \
edje_text.c \
edje_util.c \
edje_private.h

libedje_la_LIBADD       = $(LDFLAGS) -lm @evas_libs@ @ecore_libs@ @eet_libs@
libedje_la_CPPFLAGS     = @edje_def@
libedje_la_DEPENDENCIES = $(top_builddir)/config.h
libedje_la_LDFLAGS      = -version-info 0:1:0

libedje_edit_la_SOURCES  = \
edje_edit.c

libedje_edit_la_LIBADD       = $(LDFLAGS) -lm @evas_libs@ @ecore_libs@ @eet_libs@ $(top_builddir)/src/lib/libedje.la
libedje_edit_la_DEPENDENCIES = $(top_builddir)/config.h libedje.la
libedje_edit_la_LDFLAGS      = -version-info 0:1:0
