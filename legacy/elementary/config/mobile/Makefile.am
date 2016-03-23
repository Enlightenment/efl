MAINTAINERCLEANFILES = Makefile.in

EET_EET = @eet_eet@

EXTRA_DIST = \
        profile.desktop \
        icon.png \
        base.src.in

filesdir = $(datadir)/elementary/config/mobile
files_DATA = \
        profile.desktop \
        icon.png \
        base.cfg

%.cfg: %.src
	$(EET_EET) -e \
	$(top_builddir)/config/mobile/$@ config \
	$< 1

clean-local:
	rm -rf *.cfg *~
