AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS_VERBOSE_ =
EDJE_FLAGS_VERBOSE_0 =
EDJE_FLAGS_VERBOSE_1 = -v
EDJE_FLAGS = $(EDJE_FLAGS_$(V)) -id $(top_srcdir)/data/edje_externals -fd $(top_srcdir)/data/edje_externals

filesdir = $(datadir)/elementary/edje_externals
files_DATA = icons.edj

EXTRA_DIST = \
icons.edc \
ico_anchorblock.png \
ico_anchorview.png \
ico_bubble.png \
ico_button.png \
ico_check.png \
ico_clock.png \
ico_fileselector.png \
ico_fileselector_button.png \
ico_genlist.png \
ico_hoversel.png \
ico_list.png \
ico_map.png \
ico_notepad.png \
ico_photocam.png \
ico_progressbar.png \
ico_radio.png \
ico_scrolled_entry.png \
ico_slider.png \
ico_slideshow.png \
ico_spinner.png \
ico_thumb.png \
ico_toggle.png \
ico_toolbar.png

icons.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/edje_externals/icons.edc \
	$(top_builddir)/data/edje_externals/icons.edj

clean-local:
	rm -f *.edj
