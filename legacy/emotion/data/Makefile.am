
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data -fd $(top_srcdir)/data

filesdir = $(datadir)/$(PACKAGE)/data

files_DATA =

if BUILD_EMOTION_TEST
files_DATA += theme.edj
endif

if ENABLE_EDJE_EXTERNAL
files_DATA += icon.edj
endif

EXTRA_DIST = \
bpause.png \
bplay.png \
bstop.png \
e_logo.png \
fr1.png \
fr2.png \
fr3.png \
fr4.png \
fr5.png \
fr6.png \
fr7.png \
h_slider.png \
knob.png \
orb.png \
pnl.png \
sl.png \
tiles.png \
video_frame_bottom.png \
video_frame_left.png \
video_frame_right.png \
video_frame_top.png \
whb.png \
window_inner_shadow.png \
theme.edc \
icon.edc

theme.edj: Makefile theme.edc
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/theme.edc \
	$(top_builddir)/data/theme.edj

icon.edj: Makefile icon.edc
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/icon.edc \
	$(top_builddir)/data/icon.edj

clean-local:
	rm -f *.edj
