AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_WINCE_CFLAGS@ \
@ELEMENTARY_EDBUS_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

lib_LTLIBRARIES = libelementary.la

include_HEADERS = Elementary.h

libelementary_la_SOURCES = \
elm_priv.h \
elm_main.c \
elm_theme.c \
\
elm_win.c \
elm_widget.c \
elm_bg.c \
elm_icon.c \
elm_box.c \
elm_button.c \
elm_scroller.c \
elm_label.c \
elm_toggle.c \
elm_frame.c \
elm_table.c \
elm_clock.c \
elm_layout.c \
elm_hover.c \
elm_entry.c \
elm_bubble.c \
elm_photo.c \
elm_toolbar.c \
elm_list.c \
elm_carousel.c \
\
elc_notepad.c \
elc_anchorview.c \
elc_anchorblock.c \
elc_hoversel.c \
\
els_pan.c \
els_pan.h \
els_scroller.c \
els_scroller.h \
els_box.c \
els_box.h \
els_table.c \
els_table.h \
els_icon.c \
els_icon.h


libelementary_la_CFLAGS =
libelementary_la_LIBADD = @my_libs@ @dlopen_libs@ @ELEMENTARY_LIBS@ @ELEMENTARY_X_LIBS@ @ELEMENTARY_FB_LIBS@ @ELEMENTARY_WIN32_LIBS@ @ELEMENTARY_WINCE_LIBS@ @ELEMENTARY_EDBUS_LIBS@
libelementary_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@
