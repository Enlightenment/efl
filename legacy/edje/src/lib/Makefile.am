## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = \
-I$(top_srcdir) \
-I$(top_builddir) \
@eet_cflags@ \
@embryo_cflags@ \
@evas_cflags@ \
@ecore_cflags@

lib_LTLIBRARIES      = \
libedje.la 

include_HEADERS      =  \
Edje.h 

libedje_la_SOURCES  = \
edje_calc.c \
edje_callbacks.c \
edje_data.c \
edje_embryo.c \
edje_load.c \
edje_main.c \
edje_misc.c \
edje_program.c \
edje_smart.c \
edje_text.c \
edje_util.c \
edje_var.c \
edje_container.c \
edje_container.h \
edje_message_queue.c \
edje_private.h

libedje_la_LIBADD       = -lm @evas_libs@ @ecore_libs@ @eet_libs@ @embryo_libs@ @fnmatch_libs@
libedje_la_CPPFLAGS     =
libedje_la_DEPENDENCIES = $(top_builddir)/config.h
libedje_la_LDFLAGS      = -version-info 5:0:5

