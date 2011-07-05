AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-DELM_INTERNAL_API_ARGESFSDFEFC=1 \
-DMODULES_PATH=\"$(libdir)/elementary/modules\" \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DLOCALE_DIR=\"@LOCALE_DIR@\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_SDL_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_WINCE_CFLAGS@ \
@ELEMENTARY_EDBUS_CFLAGS@ \
@ELEMENTARY_EFREET_CFLAGS@ \
@ELEMENTARY_ETHUMB_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@EVIL_CFLAGS@ \
@EIO_CFLAGS@ \
@EMOTION_CFLAGS@ \
@EFL_PTHREAD_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

lib_LTLIBRARIES = libelementary.la

includes_HEADERS = \
Elementary.h \
Elementary_Cursor.h \
elm_widget.h
includesdir = $(includedir)/elementary-@VMAJ@

libelementary_la_SOURCES = \
elm_priv.h \
elc_anchorblock.c \
elc_anchorview.c \
elc_ctxpopup.c \
elc_fileselector_button.c \
elc_fileselector.c \
elc_fileselector_entry.c \
elc_hoversel.c \
elc_player.c \
elc_scrolled_entry.c \
elm_actionslider.c \
elm_animator.c \
elm_bg.c \
elm_box.c \
elm_bubble.c \
elm_button.c \
elm_calendar.c \
elm_check.c \
elm_clock.c \
elm_cnp_helper.c \
elm_colorselector.c \
elm_config.c \
elm_conform.c \
elm_diskselector.c \
elm_entry.c \
elm_flip.c \
elm_flipselector.c \
elm_font.c \
elm_frame.c \
elm_gengrid.c \
elm_genlist.c \
elm_genscroller.c \
elm_glview.c \
elm_grid.c \
elm_hover.c \
elm_icon.c \
elm_image.c \
elm_index.c \
elm_label.c \
elm_layout.c \
elm_list.c \
elm_main.c \
elm_mapbuf.c \
elm_map.c \
elm_menu.c \
elm_module.c \
elm_notify.c \
elm_pager.c \
elm_panel.c \
elm_panes.c \
elm_photo.c \
elm_photocam.c \
elm_progressbar.c \
elm_radio.c \
elm_route.c \
elm_scroller.c \
elm_segment_control.c \
elm_separator.c \
elm_slider.c \
elm_slideshow.c \
elm_spinner.c \
elm_store.c \
elm_table.c \
elm_theme.c \
elm_thumb.c \
elm_toggle.c \
elm_toolbar.c \
elm_transit.c \
elm_util.c \
elm_video.c \
elm_widget.c \
elm_win.c \
els_box.c \
els_box.h \
els_cursor.c \
els_icon.c \
els_icon.h \
els_pan.c \
els_pan.h \
els_scroller.c \
els_scroller.h \
els_tooltip.c

libelementary_la_CFLAGS =
libelementary_la_LIBADD = \
@my_libs@ \
@dlopen_libs@ \
@ELEMENTARY_LIBS@ \
@ELEMENTARY_X_LIBS@ \
@ELEMENTARY_FB_LIBS@ \
@ELEMENTARY_SDL_LIBS@ \
@ELEMENTARY_WIN32_LIBS@ \
@ELEMENTARY_WINCE_LIBS@ \
@ELEMENTARY_EDBUS_LIBS@ \
@ELEMENTARY_EFREET_LIBS@ \
@ELEMENTARY_ETHUMB_LIBS@ \
@ELEMENTARY_EMAP_LIBS@ \
@EVIL_LIBS@ \
@EIO_LIBS@ \
@EMOTION_LIBS@ \
@EFL_PTHREAD_LIBS@
libelementary_la_LDFLAGS = \
-no-undefined @lt_enable_auto_import@ \
-version-info @version_info@ @release_info@
