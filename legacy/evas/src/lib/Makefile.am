MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = canvas data file engines imaging include

AUTOMAKE_OPTIONS     = 1.4 foreign

INCLUDES	     = -I. \
                       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/lib/include \
		       @FREETYPE_CFLAGS@ \
		       @eet_cflags@ \
		       @FONTCONFIG_CFLAGS@ \
		       @pthread_cflags@

lib_LTLIBRARIES      = libevas.la

include_HEADERS      = \
Evas.h 

libevas_la_SOURCES  = \
main.c

libevas_la_LIBADD       = \
                          canvas/libevas_canvas.la \
			  data/libevas_data.la \
			  file/libevas_file.la \
			  imaging/libevas_imaging.la \
			  engines/common/libevas_engine_common.la \
			  -lm \
			  @dlopen_libs@ \
			  @FREETYPE_LIBS@ \
			  @eet_libs@ \
			  @FONTCONFIG_LIBS@ \
			  @pthread_libs@

libevas_la_DEPENDENCIES = \
                          $(top_builddir)/config.h \
                          canvas/libevas_canvas.la \
			  data/libevas_data.la \
			  file/libevas_file.la \
			  imaging/libevas_imaging.la \
			  engines/common/libevas_engine_common.la 
libevas_la_LDFLAGS      = @create_shared_lib@ -version-info 1:0:0
