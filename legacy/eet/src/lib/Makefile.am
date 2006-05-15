## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I. \
                       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include 
					      

lib_LTLIBRARIES      = libeet.la
include_HEADERS      = Eet.h
libeet_la_SOURCES  = \
eet_lib.c \
eet_data.c \
eet_image.c \
eet_memfile.c \
Eet_private.h

libeet_la_LIBADD       = -lz -ljpeg @fnmatch_libs@ @winsock_libs@
libeet_la_DEPENDENCIES = $(top_builddir)/config.h
libeet_la_LDFLAGS      = -version-info 9:10:9
