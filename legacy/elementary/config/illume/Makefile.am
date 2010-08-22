MAINTAINERCLEANFILES = Makefile.in

EET_EET = @eet_eet@

EXTRA_DIST = \
        profile.desktop \
        icon.png \
	base.src

filesdir = $(datadir)/elementary/config/illume
files_DATA = \
        profile.desktop \
        icon.png \
	base.cfg

%.cfg: %.src
	$(EET_EET) -e \
	$(top_builddir)/config/illume/$@ config \
	$< 1

clean-local:
	rm -rf *.cfg *~
