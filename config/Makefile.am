MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = default standard mobile

include ../src/Makefile_Eet_Helper.am

EXTRA_DIST = profile.src

filesdir = $(datadir)/elementary/config

files_DATA = profile.cfg

%.cfg: %.src
	$(AM_V_EET) \
	$(MKDIR_P) $(top_builddir)/config/; \
	$(EET) -i $(top_builddir)/config/$@ config $< 0

clean-local:
	rm -rf *.cfg *~
