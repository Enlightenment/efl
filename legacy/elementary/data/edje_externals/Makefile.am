AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/edje_externals -fd $(top_srcdir)/data/edje_externals

filesdir = $(datadir)/elementary/edje_externals
files_DATA = icons.edj

EXTRA_DIST = \
icons.edc \
ico_bubble.png \
ico_button.png \
ico_check.png \
ico_notepad.png \
ico_radio.png \
ico_scrolled_entry.png \
ico_slider.png \
ico_toggle.png

icons.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/edje_externals/icons.edc \
	$(top_builddir)/data/edje_externals/icons.edj

clean-local:
	rm -f *.edj
