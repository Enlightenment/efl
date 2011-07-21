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
widget_preview_colorselector \
widget_preview_layout \
widget_preview_conformant \
widget_preview_index \
widget_preview_clock \
widget_preview_label \
widget_preview_frame \
widget_preview_ctxpopup \
widget_preview_icon \
widget_preview_image \
widget_preview_calendar \
widget_preview_bg \
widget_preview_fileselector \
widget_preview_actionslider \
widget_preview_entry1 \
widget_preview_entry2 \
widget_preview_entry3 \
widget_preview_entry4
widget_preview_diskselector \
widget_preview_fileselector_entry \
widget_preview_flipselector \
widget_preview_hoversel \
widget_preview_hover \
widget_preview_anchorview \
widget_preview_anchorblock \
widget_preview_flip

LDADD = $(top_builddir)/src/lib/libelementary.la @ELEMENTARY_EWEATHER_LIBS@ @ELEMENTARY_EDBUS_LIBS@ @ELEMENTARY_EFREET_LIBS@ @ELEMENTARY_LIBS@ @EIO_LIBS@ @my_libs@

# dir:file name:executable:width:height
WGT_PREVIEW = \
	flip:preview-00.png:widget_preview_flip:100:100 \
	button:preview-00.png:widget_preview_button1:160:50 \
	button:preview-01.png:widget_preview_button2:160:50 \
	button:preview-02.png:widget_preview_button3:160:50 \
	fileselector_button:preview-00.png:widget_preview_fileselector_button1:200:50 \
	fileselector_button:preview-01.png:widget_preview_fileselector_button2:200:50 \
	fileselector_button:preview-02.png:widget_preview_fileselector_button3:160:50 \
	check:preview-00.png:widget_preview_check1:160:50 \
	check:preview-01.png:widget_preview_check2:160:50 \
	check:preview-02.png:widget_preview_check3:160:50 \
	bubble:preview-00.png:widget_preview_bubble1:160:50 \
	bubble:preview-01.png:widget_preview_bubble2:160:50 \
	bubble:preview-02.png:widget_preview_bubble3:160:50 \
	colorselector:preview-00.png:widget_preview_colorselector:160:200 \
	layout:preview-00.png:widget_preview_layout:200:160 \
	conformant:preview-00.png:widget_preview_conformant:200:400 \
	index:preview-00.png:widget_preview_index:300:500 \
	frame:preview-00.png:widget_preview_frame:100:50 \
	label:preview-00.png:widget_preview_label:70:30 \
	clock:preview-00.png:widget_preview_clock:200:100 \
	ctxpopup:preview-00.png:widget_preview_ctxpopup:200:130 \
	icon:preview-00.png:widget_preview_icon:50:50 \
	image:preview-00.png:widget_preview_image:50:50 \
	calendar:preview-00.png:widget_preview_calendar:300:300 \
	bg:preview-00.png:widget_preview_bg:50:50 \
	fileselector:preview-00.png:widget_preview_fileselector:300:300 \
	actionslider:preview-00.png:widget_preview_actionslider:120:30 \
	entry:preview-00.png:widget_preview_entry1:160:50 \
	entry:preview-01.png:widget_preview_entry2:160:50 \
	entry:preview-02.png:widget_preview_entry3:160:50 \
	entry:preview-03.png:widget_preview_entry4:160:50
	diskselector:preview-00.png:widget_preview_diskselector:120:50 \
	fileselector_entry:preview-00.png:widget_preview_fileselector_entry:70:50 \
	flipselector:preview-00.png:widget_preview_flipselector:100:60 \
	hoversel:preview-00.png:widget_preview_hoversel:90:170 \
	hover:preview-00.png:widget_preview_hover:90:170 \
	anchorview:preview-00.png:widget_preview_anchorview:100:30 \
	anchorblock:preview-00.png:widget_preview_anchorblock:100:30

widget-preview:
	@for ss in $(WGT_PREVIEW); do \
		ENTRY=($${ss//:/ }) ; \
		DIR=$${ENTRY[0]} ; \
		SS=$${ENTRY[1]} ; \
		BIN=$${ENTRY[2]} ; \
		X=$${ENTRY[3]} ; \
		Y=$${ENTRY[4]} ; \
		$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/$${DIR} $${SS} $(top_builddir)/doc/$${BIN} $${X} $${Y} ; \
		mkdir -p $(top_builddir)/doc/latex/img/widget/$${DIR} ; \
		convert $(top_builddir)/doc/html/img/widget/$${DIR}/$${SS} $(top_builddir)/doc/latex/img/widget/$${DIR}/$${SS/.png/.eps} ; \
	done

doc-clean:
	rm -rf img/widget html/ latex/ man/ xml/ $(top_builddir)/$(PACKAGE_DOCNAME).tar*

gen-doc:
	$(efl_doxygen)
	cp -r $(srcdir)/img/* html/
	cp -r $(srcdir)/img/* latex/

doc: all doc-clean widget-preview gen-doc
	@echo "entering src/examples/"
	$(MAKE) -C $(top_builddir)/src/examples screenshots
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
