MAINTAINERCLEANFILES = Makefile.in

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
@ELEMENTARY_EMAP_CFLAGS@ \
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
widget_preview_spinner \
widget_preview_bubble1 \
widget_preview_bubble2 \
widget_preview_bubble3 \
widget_preview_fileselector_button1 \
widget_preview_fileselector_button2 \
widget_preview_fileselector_button3 \
widget_preview_colorselector \
widget_preview_layout \
widget_preview_list \
widget_preview_conformant \
widget_preview_index \
widget_preview_clock \
widget_preview_label \
widget_preview_frame \
widget_preview_slider \
widget_preview_panes \
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
widget_preview_entry4 \
widget_preview_diskselector \
widget_preview_fileselector_entry \
widget_preview_flipselector \
widget_preview_hoversel \
widget_preview_hover \
widget_preview_anchorview \
widget_preview_anchorblock \
widget_preview_flip \
widget_preview_pager \
widget_preview_separator \
widget_preview_radio \
widget_preview_toggle \
widget_preview_panel \
widget_preview_gengrid \
widget_preview_progressbar \
widget_preview_box \
widget_preview_notify

LDADD = $(top_builddir)/src/lib/libelementary.la @ELEMENTARY_EWEATHER_LIBS@ @ELEMENTARY_EDBUS_LIBS@ @ELEMENTARY_EFREET_LIBS@ @ELEMENTARY_EMAP_LIBS@ @ELEMENTARY_LIBS@ @EIO_LIBS@ @my_libs@

endif

EXTRA_DIST = \
	widget_preview_actionslider.c \
	widget_preview_anchorblock.c \
	widget_preview_anchorview.c \
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
	widget_preview_ctxpopup.c \
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
	widget_preview_pager.c \
	widget_preview_separator.c \
	widget_preview_radio.c \
	widget_preview_toggle.c \
	widget_preview_panel.c \
	widget_preview_gengrid.c \
	widget_preview_progressbar.c \
	widget_preview_box.c \
	widget_preview_notify.c \
	widget_preview_tmpl_foot.c \
	widget_preview_tmpl_head.c
