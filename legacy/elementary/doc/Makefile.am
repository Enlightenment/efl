SUBDIRS = widgets

MAINTAINERCLEANFILES = Makefile.in elementary.dox

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if EFL_BUILD_DOC

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
	index:preview-00.png:widget_preview_index:200:160 \
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
	entry:preview-03.png:widget_preview_entry4:160:50 \
	diskselector:preview-00.png:widget_preview_diskselector:120:50 \
	fileselector_entry:preview-00.png:widget_preview_fileselector_entry:70:50 \
	flipselector:preview-00.png:widget_preview_flipselector:100:60 \
	hoversel:preview-00.png:widget_preview_hoversel:90:170 \
	hover:preview-00.png:widget_preview_hover:90:170 \
	anchorview:preview-00.png:widget_preview_anchorview:100:30 \
	anchorblock:preview-00.png:widget_preview_anchorblock:100:30 \
	pager:preview-00.png:widget_preview_pager:100:100 \
	separator:preview-00.png:widget_preview_separator:10:80 \
	radio:preview-00.png:widget_preview_radio:60:20 \
	toggle:preview-00.png:widget_preview_toggle:150:30 \
	panel:preview-00.png:widget_preview_panel:150:50 \
	gengrid:preview-00.png:widget_preview_gengrid:200:160

widget-build:
	@$(MAKE) -C widgets

widget-preview: widget-build
	@for ss in $(WGT_PREVIEW); do \
		ENTRY=($${ss//:/ }) ; \
		DIR=$${ENTRY[0]} ; \
		SS=$${ENTRY[1]} ; \
		BIN=$${ENTRY[2]} ; \
		X=$${ENTRY[3]} ; \
		Y=$${ENTRY[4]} ; \
		$(top_srcdir)/doc/shot.sh $(top_builddir)/doc/html/img/widget/$${DIR} $${SS} $(top_builddir)/doc/widgets/$${BIN} $${X} $${Y} ; \
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

EXTRA_DIST = Doxyfile.in $(wildcard img/*.*) e.css head.html foot.html elementary.dox.in
