## Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(REMIX_PLUGIN_DIR)

AM_CPPFLAGS = \
-I. \
-DPACKAGE_LIB_DIR=\"$(libdir)/remix\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/remix\" \
@EDJE_CFLAGS@ \
@REMIX_CFLAGS@ \
@ALSA_CFLAGS@

pkgdir = $(REMIX_PLUGIN_DIR)
pkg_LTLIBRARIES = libalsa_snd_player.la

libalsa_snd_player_la_SOURCES = alsa_snd_player.c
libalsa_snd_player_la_LDFLAGS =  -no-undefined @lt_enable_auto_import@ -module -avoid-version @EDJE_LIBS@ @REMIX_LIBS@ @ALSA_LIBS@
libalsa_snd_player_la_LIBTOOLFLAGS = --tag=disable-static
