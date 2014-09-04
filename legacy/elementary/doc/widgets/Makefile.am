MAINTAINERCLEANFILES = Makefile.in

if EFL_BUILD_DOC

include ../../Makefile_Elm_Helpers.am

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/bin \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DDOCW_DIR=\"$(abs_top_builddir)/doc/widgets\" \
@ELEMENTARY_CFLAGS@

noinst_DATA = widget_preview_prefs.epb

widget_preview_prefs.epb: Makefile widget_preview_prefs.epc
	$(AM_V_EPB)$(ELM_PREFS_CC) $(ELM_PREFS_FLAGS) \
	$(top_srcdir)/doc/widgets/widget_preview_prefs.epc \
	$(top_builddir)/doc/widgets/widget_preview_prefs.epb

noinst_PROGRAMS = \
widget_preview_actionslider \
widget_preview_bg \
widget_preview_box \
widget_preview_bubble1 \
widget_preview_bubble2 \
widget_preview_bubble3 \
widget_preview_button1 \
widget_preview_button2 \
widget_preview_button3 \
widget_preview_calendar \
widget_preview_check1 \
widget_preview_check2 \
widget_preview_check3 \
widget_preview_clock \
widget_preview_colorselector \
widget_preview_conformant \
widget_preview_ctxpopup \
widget_preview_datetime1 \
widget_preview_datetime2 \
widget_preview_datetime3 \
widget_preview_dayselector \
widget_preview_diskselector \
widget_preview_entry1 \
widget_preview_entry2 \
widget_preview_entry3 \
widget_preview_entry4 \
widget_preview_fileselector \
widget_preview_fileselector_button1 \
widget_preview_fileselector_button2 \
widget_preview_fileselector_button3 \
widget_preview_fileselector_entry \
widget_preview_flip \
widget_preview_flipselector \
widget_preview_frame \
widget_preview_gengrid \
widget_preview_genlist1 \
widget_preview_genlist2 \
widget_preview_genlist3 \
widget_preview_genlist4 \
widget_preview_genlist5 \
widget_preview_hover \
widget_preview_hoversel \
widget_preview_icon \
widget_preview_image \
widget_preview_index \
widget_preview_inwin1 \
widget_preview_inwin2 \
widget_preview_inwin3 \
widget_preview_label \
widget_preview_layout \
widget_preview_list \
widget_preview_map \
widget_preview_mapbuf \
widget_preview_menu \
widget_preview_notify \
widget_preview_panel \
widget_preview_panes \
widget_preview_photocam \
widget_preview_popup \
widget_preview_prefs \
widget_preview_progressbar \
widget_preview_radio \
widget_preview_scroller \
widget_preview_segment_control \
widget_preview_separator \
widget_preview_slider \
widget_preview_slideshow \
widget_preview_spinner \
widget_preview_table \
widget_preview_thumb \
widget_preview_toolbar \
widget_preview_web \
widget_preview_win

LDADD = $(top_builddir)/src/lib/libelementary.la \
@ELEMENTARY_LIBS@

endif

EXTRA_DIST = \
	widget_preview_actionslider.c \
	widget_preview_bg.c \
	widget_preview_bubble1.c \
	widget_preview_bubble2.c \
	widget_preview_bubble3.c \
	widget_preview_button1.c \
	widget_preview_button2.c \
	widget_preview_button3.c \
	widget_preview_calendar.c \
	widget_preview_check1.c \
	widget_preview_check2.c \
	widget_preview_check3.c \
	widget_preview_spinner.c \
	widget_preview_clock.c \
	widget_preview_colorselector.c \
	widget_preview_conformant.c \
	widget_preview_slider.c \
	widget_preview_panes.c \
	widget_preview_toolbar.c \
	widget_preview_mapbuf.c \
	widget_preview_map.c \
	widget_preview_ctxpopup.c \
	widget_preview_datetime1.c \
	widget_preview_datetime2.c \
	widget_preview_datetime3.c \
	widget_preview_dayselector.c \
	widget_preview_diskselector.c \
	widget_preview_entry1.c \
	widget_preview_entry2.c \
	widget_preview_entry3.c \
	widget_preview_entry4.c \
	widget_preview_fileselector_button1.c \
	widget_preview_fileselector_button2.c \
	widget_preview_fileselector_button3.c \
	widget_preview_fileselector.c \
	widget_preview_fileselector_entry.c \
	widget_preview_flip.c \
	widget_preview_flipselector.c \
	widget_preview_frame.c \
	widget_preview_hover.c \
	widget_preview_hoversel.c \
	widget_preview_icon.c \
	widget_preview_image.c \
	widget_preview_index.c \
	widget_preview_label.c \
	widget_preview_layout.c \
	widget_preview_list.c \
	widget_preview_segment_control.c \
	widget_preview_popup.c \
	widget_preview_separator.c \
	widget_preview_radio.c \
	widget_preview_panel.c \
	widget_preview_gengrid.c \
	widget_preview_genlist1.c \
	widget_preview_genlist2.c \
	widget_preview_genlist3.c \
	widget_preview_genlist4.c \
	widget_preview_genlist5.c \
	widget_preview_prefs.c \
	widget_preview_prefs.epc \
	widget_preview_progressbar.c \
	widget_preview_box.c \
	widget_preview_notify.c \
	widget_preview_slideshow.c \
	widget_preview_photocam.c \
	widget_preview_inwin1.c \
	widget_preview_inwin2.c \
	widget_preview_inwin3.c \
	widget_preview_scroller.c \
	widget_preview_table.c \
	widget_preview_win.c \
	widget_preview_menu.c \
	widget_preview_thumb.c \
	widget_preview_web.c \
	widget_preview_tmpl_foot.c \
	widget_preview_tmpl_head.c

clean-local:
	rm -f widget_preview_prefs.epb
