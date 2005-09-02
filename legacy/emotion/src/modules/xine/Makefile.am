## Process this file with automake to produce Makefile.in

#AUTOMAKE_OPTIONS     = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	     = -I$(top_srcdir) \
		       -I$(top_srcdir)/src/lib \
		       -I$(top_srcdir)/src/modules \
		       -I$(top_srcdir)/src/modules/xine \
		       @my_cflags@ @xine_cflags@

pkgdir               = @xine_plugins@

pkg_LTLIBRARIES      = xineplug_vo_out_emotion.la

xineplug_vo_out_emotion_la_SOURCES      = emotion_xine_vo_out.c
xineplug_vo_out_emotion_la_LIBADD       = @xine_libs@
xineplug_vo_out_emotion_la_LDFLAGS      = -module -avoid-version \
					  -L$(top_builddir)/src/lib -L$(top_builddir)/src/lib/.libs
xineplug_vo_out_emotion_la_DEPENDENCIES = $(top_builddir)/config.h
