MAINTAINERCLEANFILES = Makefile.in elementary.dox

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if EFL_BUILD_DOC

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/bin \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_WINCE_CFLAGS@ \
@ELEMENTARY_EDBUS_CFLAGS@ \
@ELEMENTARY_EFREET_CFLAGS@ \
@ELEMENTARY_EWEATHER_CFLAGS@ \
@ELEMENTARY_ETHUMB_CFLAGS@ \
@EIO_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

noinst_PROGRAMS = \
widget_preview_button1 \
widget_preview_button2 \
widget_preview_button3 \
widget_preview_check1 \
widget_preview_check2 \
widget_preview_check3 \
widget_preview_bubble1 \
widget_preview_bubble2 \
widget_preview_bubble3 \
widget_preview_fileselector_button1 \
widget_preview_fileselector_button2 \
widget_preview_fileselector_button3 \
widget_preview_colorselector

LDADD = $(top_builddir)/src/lib/libelementary.la @ELEMENTARY_EWEATHER_LIBS@ @ELEMENTARY_EDBUS_LIBS@ @ELEMENTARY_EFREET_LIBS@ @ELEMENTARY_LIBS@ @EIO_LIBS@ @my_libs@

doc-clean:
	rm -rf img/widget html/ latex/ man/ xml/ $(top_builddir)/$(PACKAGE_DOCNAME).tar*

doc: all doc-clean
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/button preview-00.png $(top_builddir)/doc/widget_preview_button1 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/button preview-01.png $(top_builddir)/doc/widget_preview_button2 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/button preview-02.png $(top_builddir)/doc/widget_preview_button3 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/fileselector_button preview-00.png $(top_builddir)/doc/widget_preview_fileselector_button1 200 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/fileselector_button preview-01.png $(top_builddir)/doc/widget_preview_fileselector_button2 200 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/fileselector_button preview-02.png $(top_builddir)/doc/widget_preview_fileselector_button3 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/check preview-00.png $(top_builddir)/doc/widget_preview_check1 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/check preview-01.png $(top_builddir)/doc/widget_preview_check2 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/check preview-02.png $(top_builddir)/doc/widget_preview_check3 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/bubble preview-00.png $(top_builddir)/doc/widget_preview_bubble1 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/bubble preview-01.png $(top_builddir)/doc/widget_preview_bubble2 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/bubble preview-02.png $(top_builddir)/doc/widget_preview_bubble3 160 50
	$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/colorselector preview-00.png $(top_builddir)/doc/widget_preview_colorselector 160 200
	$(efl_doxygen)
	cp -r $(srcdir)/img/* html/
	cp -r $(srcdir)/img/* latex/
	rm -rf $(PACKAGE_DOCNAME).tar*
	mkdir -p $(PACKAGE_DOCNAME)/doc
	cp -R html/ latex/ man/ $(PACKAGE_DOCNAME)/doc
	tar cf $(PACKAGE_DOCNAME).tar $(PACKAGE_DOCNAME)/
	bzip2 -9 $(PACKAGE_DOCNAME).tar
	rm -rf $(PACKAGE_DOCNAME)/
	mv $(PACKAGE_DOCNAME).tar.bz2 $(top_builddir)

clean-local: doc-clean

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

EXTRA_DIST = Doxyfile.in $(wildcard img/*.*) e.css head.html foot.html elementary.dox.in widget_preview_tmpl_head.c widget_preview_tmpl_foot.c
