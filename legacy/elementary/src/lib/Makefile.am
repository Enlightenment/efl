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
@ELEMENTARY_WEB_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_ECORE_CON_CFLAGS@ \
@ELEMENTARY_ECORE_IMF_CFLAGS@ \
@ELEMENTARY_WAYLAND_CFLAGS@ \
@EVIL_CFLAGS@ \
@EIO_CFLAGS@ \
@EMOTION_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

lib_LTLIBRARIES = libelementary.la

includes_HEADERS = \
Elementary.h \
Elementary_Cursor.h
includesdir = $(includedir)/elementary-@VMAJ@

includesunstable_HEADERS = \
elm_interface_scrollable.h \
elm_widget.h \
elm_widget_actionslider.h \
elm_widget_bg.h \
elm_widget_box.h \
elm_widget_bubble.h \
elm_widget_button.h \
elm_widget_calendar.h \
elm_widget_check.h \
elm_widget_clock.h \
elm_widget_colorselector.h \
elm_widget_conform.h \
elm_widget_container.h \
elm_widget_ctxpopup.h \
elm_widget_datetime.h \
elm_widget_dayselector.h \
elm_widget_diskselector.h \
elm_widget_entry.h \
elm_widget_fileselector.h \
elm_widget_fileselector_button.h \
elm_widget_flip.h \
elm_widget_flipselector.h \
elm_widget_frame.h \
elm_widget_gengrid.h \
elm_widget_genlist.h \
elm_widget_glview.h \
elm_widget_grid.h \
elm_widget_hover.h \
elm_widget_hoversel.h \
elm_widget_icon.h \
elm_widget_image.h \
elm_widget_index.h \
elm_widget_inwin.h \
elm_widget_label.h \
elm_widget_layout.h \
elm_widget_list.h \
elm_widget_map.h \
elm_widget_mapbuf.h \
elm_widget_menu.h \
elm_widget_multibuttonentry.h \
elm_widget_naviframe.h \
elm_widget_notify.h \
elm_widget_panel.h \
elm_widget_panes.h \
elm_widget_photo.h \
elm_widget_photocam.h \
elm_widget_player.h \
elm_widget_plug.h \
elm_widget_popup.h \
elm_widget_progressbar.h \
elm_widget_radio.h \
elm_widget_route.h \
elm_widget_scroller.h \
elm_widget_segment_control.h \
elm_widget_separator.h \
elm_widget_slider.h \
elm_widget_slideshow.h \
elm_widget_spinner.h \
elm_widget_table.h \
elm_widget_thumb.h \
elm_widget_video.h \
elm_widget_web.h
includesunstabledir = $(includedir)/elementary-@VMAJ@

includesub_HEADERS = \
elc_ctxpopup.h \
elm_dayselector.h \
elc_fileselector_button.h \
elc_fileselector_entry.h \
elc_fileselector.h \
elc_hoversel.h \
elc_multibuttonentry.h \
elc_naviframe.h \
elc_popup.h \
elm_actionslider.h \
elm_app.h \
elm_authors.h \
elm_bg.h \
elm_box.h \
elm_bubble.h \
elm_button.h \
elm_cache.h \
elm_calendar.h \
elm_check.h \
elm_clock.h \
elm_cnp.h \
elm_colorselector.h \
elm_config.h \
elm_conform.h \
elm_cursor.h \
elm_datetime.h \
elm_debug.h \
elm_deprecated.h \
elm_diskselector.h \
elm_entry.h \
elm_finger.h \
elm_flip.h \
elm_flipselector.h \
elm_focus.h \
elm_font.h \
elm_frame.h \
elm_gen.h \
elm_general.h \
elm_gengrid.h \
elm_genlist.h \
elm_gesture_layer.h \
elm_getting_started.h \
elm_glview.h \
elm_grid.h \
elm_hover.h \
elm_icon.h \
elm_image.h \
elm_inwin.h \
elm_plug.h \
elm_index.h \
elm_intro.h \
elm_label.h \
elm_layout.h \
elm_list.h \
elm_macros.h \
elm_mapbuf.h \
elm_map.h \
elm_menu.h \
elm_mirroring.h \
elm_need.h \
elm_notify.h \
elm_object.h \
elm_object_item.h \
elm_panel.h \
elm_panes.h \
elm_photocam.h \
elm_photo.h \
elm_progressbar.h \
elm_radio.h \
elm_route.h \
elm_scale.h \
elm_scroller.h \
elm_scroll.h \
elm_segment_control.h \
elm_separator.h \
elm_slider.h \
elm_slideshow.h \
elm_spinner.h \
elm_store.h \
elm_table.h \
elm_theme.h \
elm_thumb.h \
elm_toolbar.h \
elm_tooltip.h \
elm_transit.h \
elm_video.h \
elm_web.h \
elm_win.h
includesubdir = $(includedir)/elementary-@VMAJ@/

libelementary_la_SOURCES = \
elm_gen_common.h \
elm_priv.h \
els_box.h \
els_pan.h \
els_scroller.h \
elc_ctxpopup.c \
elc_fileselector.c \
elc_fileselector_button.c \
elc_fileselector_entry.c \
elc_hoversel.c \
elc_multibuttonentry.c \
elc_naviframe.c \
elc_player.c \
elc_popup.c \
elc_scrolled_entry.c \
elm_access.c \
elm_actionslider.c \
elm_bg.c \
elm_box.c \
elm_bubble.c \
elm_button.c \
elm_calendar.c \
elm_check.c \
elm_clock.c \
elm_cnp.c \
elm_colorselector.c \
elm_config.c \
elm_conform.c \
elm_container.c \
elm_datetime.c \
elm_dayselector.c \
elm_diskselector.c \
elm_entry.c \
elm_flip.c \
elm_flipselector.c \
elm_font.c \
elm_frame.c \
elm_gengrid.c \
elm_genlist.c \
elm_gesture_layer.c \
elm_glview.c \
elm_grid.c \
elm_hover.c \
elm_icon.c \
elm_image.c \
elm_index.c \
elm_interface_scrollable.c \
elm_inwin.c \
elm_label.c \
elm_layout.c \
elm_list.c \
elm_main.c \
elm_map.c \
elm_mapbuf.c \
elm_menu.c \
elm_module.c \
elm_notify.c \
elm_panel.c \
elm_panes.c \
elm_photo.c \
elm_photocam.c \
elm_plug.c \
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
elm_toolbar.c \
elm_transit.c \
elm_util.c \
elm_video.c \
elm_web.c \
elm_widget.c \
elm_win.c \
els_box.c \
els_cursor.c \
els_pan.c \
els_scroller.c \
els_tooltip.c \
elu_ews_wm.c

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
@ELEMENTARY_WEB_LIBS@ \
@ELEMENTARY_EMAP_LIBS@ \
@ELEMENTARY_ECORE_IMF_LIBS@ \
@ELEMENTARY_ECORE_CON_LIBS@ \
@ELEMENTARY_WAYLAND_LIBS@ \
@EVIL_LIBS@ \
@EIO_LIBS@ \
@EMOTION_LIBS@ \
@LTLIBINTL@

libelementary_la_LDFLAGS = \
-no-undefined @lt_enable_auto_import@ \
-version-info @version_info@ @release_info@

EXTRA_DIST = \
elm_factory.h \
elm_factory.c
