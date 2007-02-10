MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS     = 1.4 foreign

INCLUDES	     = -I. \
		       -I$(top_srcdir) \
		       -I$(top_builddir) \
                       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include 

lib_LTLIBRARIES      = libembryo.la
include_HEADERS      = Embryo.h
libembryo_la_SOURCES  = \
embryo_amx.c \
embryo_args.c \
embryo_float.c \
embryo_main.c \
embryo_rand.c \
embryo_str.c \
embryo_time.c \
embryo_private.h

libembryo_la_LIBADD       = -lm @fnmatch_libs@
libembryo_la_DEPENDENCIES = $(top_builddir)/config.h
libembryo_la_LDFLAGS      = @create_shared_lib@ -version-info 9:1:9
