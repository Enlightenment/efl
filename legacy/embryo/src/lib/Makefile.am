## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS		     = 
INCLUDES	     = -I. \
                       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include 
					      

lib_LTLIBRARIES      = libembryo.la
include_HEADERS      = Embryo.h
libembryo_la_SOURCES  = \
embryo_amx.c \
embryo_float.c \
embryo_main.c \
embryo_private.h

libembryo_la_LIBADD       = $(LDFLAGS) -lm
libembryo_la_DEPENDENCIES = $(top_builddir)/config.h
libembryo_la_LDFLAGS      = -version-info 1:0:0
