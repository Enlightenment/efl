AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

AM_MAKEFLAGS = --no-print-directory
AM_CFLAGS = \
@EFL_CFLAGS@ \
@EMOTION_CFLAGS@

AM_LDFLAGS = \
@EFL_LDFLAGS@

EXTRA_DIST = \
README \
AUTHORS \
COPYING \
m4/efl_compiler.m4 \
m4/emotion_generic_players.m4

MAINTAINERCLEANFILES = \
aclocal.m4 \
compile \
config.guess \
config.h.in \
config.sub \
configure \
depcomp \
install-sh \
ltmain.sh \
Makefile.in \
missing \
mkinstalldirs \
m4/libtool.m4 \
m4/lt~obsolete.m4 \
m4/ltoptions.m4 \
m4/ltsugar.m4 \
m4/ltversion.m4


# this won't pass distcheck :(
#emotiongenericplayersdir = $(EMOTION_GENERIC_PLAYERS_DIR)

emotiongenericplayersdir = \
$(libdir)/emotion/generic_players/$(MODULE_ARCH)

emotiongenericplayers_PROGRAMS =

if HAVE_VLC
emotiongenericplayers_PROGRAMS += src/vlc/vlc
src_vlc_vlc_SOURCES = src/vlc/emotion_generic_vlc.c
src_vlc_vlc_CPPFLAGS = @VLC_CFLAGS@
src_vlc_vlc_LDADD = @VLC_LIBS@
endif

.PHONY: doc

doc:
	@echo "Empty doc target until we have real documentation."
