MAINTAINERCLEANFILES = Makefile.in

include ../../../../src/Makefile_Eet_Helper.am

EXTRA_DIST = \
        profile.desktop \
        icon.png \
        base.src.in

filesdir = $(datadir)/elementary/config/default
files_DATA = \
        profile.desktop \
        icon.png \
        base.cfg

%.cfg: %.src
	$(AM_V_EET) \
	$(MKDIR_P) $(top_builddir)/data/elementary/config/default/; \
	$(EET) -e $(top_builddir)/data/elementary/config/default/$@ config $< 1

clean-local:
	rm -rf *.cfg *~

