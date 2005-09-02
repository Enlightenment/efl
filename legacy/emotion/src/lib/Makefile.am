## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I$(top_srcdir) \
		       -I$(top_srcdir)/src/lib \
		       @my_cflags@

lib_LTLIBRARIES      = libemotion.la
include_HEADERS      = Emotion.h
libemotion_la_SOURCES  = \
emotion_smart.c \
emotion_private.h

libemotion_la_LIBADD       = @my_libs@ @dlopen_libs@
libemotion_la_DEPENDENCIES = $(top_builddir)/config.h
libemotion_la_LDFLAGS      = -version-info 0:1:0
