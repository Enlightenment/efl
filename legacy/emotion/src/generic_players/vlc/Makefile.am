MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/modules \
-I$(top_srcdir)/src/modules/generic \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@GENERIC_VLC_CFLAGS@

internal_bindir = $(libdir)/emotion/utils
internal_bin_PROGRAMS = em_generic_vlc

em_generic_vlc_SOURCES = emotion_generic_vlc.c
em_generic_vlc_DEPENDENCIES = $(top_srcdir)/src/modules/generic/Emotion_Generic_Plugin.h
em_generic_vlc_LDADD = @GENERIC_VLC_LIBS@
