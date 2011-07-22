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
widget_preview_entry4 \
widget_preview_diskselector \
widget_preview_fileselector_entry \
widget_preview_flipselector \
widget_preview_hoversel \
widget_preview_hover \
widget_preview_anchorview \
widget_preview_anchorblock \
widget_preview_flip

LDADD = $(top_builddir)/src/lib/libelementary.la @ELEMENTARY_EWEATHER_LIBS@ @ELEMENTARY_EDBUS_LIBS@ @ELEMENTARY_EFREET_LIBS@ @ELEMENTARY_LIBS@ @EIO_LIBS@ @my_libs@

endif
