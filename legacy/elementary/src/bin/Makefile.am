AUTOMAKE_OPTIONS = 1.4 foreign

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-I$(top_srcdir)/src/bin \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_X_CFLAGS@ \
@ELEMENTARY_FB_CFLAGS@ \
@ELEMENTARY_SDL_CFLAGS@ \
@ELEMENTARY_WIN32_CFLAGS@ \
@ELEMENTARY_WINCE_CFLAGS@ \
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_EWEATHER_CFLAGS@ \
@ELEMENTARY_WEB_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_WAYLAND_CFLAGS@ \
@ELEMENTARY_DRM_CFLAGS@ \
@EVIL_CFLAGS@

bin_PROGRAMS = \
@ELEMENTARY_TEST_PRG@ \
@ELEMENTARY_CONFIG_PRG@  \
@ELEMENTARY_CODEGEN_PRG@ \
@ELM_PREFS_CC_PRG@

if BUILD_QUICKLAUNCH
bin_PROGRAMS += elementary_quicklaunch elementary_run
endif

EXTRA_PROGRAMS = elementary_test elementary_config elementary_codegen elm_prefs_cc

elementary_test_SOURCES = \
test.c \
test_3d.c \
test_access.c \
test_actionslider.c \
test_anim.c \
test_application_server.c \
test_bg.c \
test_box.c \
test_bubble.c \
test_button.c \
test_calendar.c \
test_check.c \
test_clock.c \
test_cnp.c \
test_colorselector.c \
test_config.c \
test_conform.c \
test_conform_indicator.c \
test_ctxpopup.c \
test_cursor.c \
test_datetime.c \
test_dayselector.c \
test_diskselector.c \
test_dnd.c \
test_eio.c \
test_entry.c \
test_entry_anchor.c \
test_entry_anchor2.c \
test_external.c \
test_fileselector_button.c \
test_fileselector.c \
test_fileselector_entry.c \
test_flip.c \
test_flip_page.c \
test_flipselector.c \
test_floating.c \
test_focus.c \
test_focus_custom_chain.c \
test_focus_style.c \
test_gengrid.c \
test_genlist.c \
test_gesture_layer.c \
test_gesture_layer2.c \
test_gesture_layer3.c \
test_glview_simple.c \
test_glview.c \
test_glview_manygears.c \
test_grid.c \
test_hover.c \
test_hoversel.c \
test_icon.c \
test_icon_desktops.c \
test_icon_animated.c \
test_image.c \
test_index.c \
test_inwin.c \
test_label.c \
test_launcher.c \
test_layout.c \
test_list.c \
test_map.c \
test_main_menu.c \
test_menu.c \
test_multi.c \
test_multibuttonentry.c \
test_naviframe.c \
test_naviframe_complex.c \
test_notify.c \
test_panel.c \
test_panes.c \
test_photo.c \
test_photocam.c \
test_popup.c \
test_prefs.c \
test_progressbar.c \
test_radio.c \
test_scaling.c \
test_scroller.c \
test_segment_control.c \
test_separator.c \
test_slider.c \
test_slideshow.c \
test_spinner.c \
test_store.c \
test_sys_notify.c \
test_systray.c \
test_task_switcher.c \
test_table.c \
test_thumb.c \
test_toolbar.c \
test_tooltip.c \
test_transit.c \
test_video.c \
test_weather.c \
test_web.c \
test_win_inline.c \
test_win_socket.c \
test_win_plug.c \
test_win_state.c \
test_win_wm_rotation.c \
test.h

elementary_test_LDADD = $(top_builddir)/src/lib/libelementary.la \
        @ELEMENTARY_WAYLAND_LIBS@ \
	@ELEMENTARY_EWEATHER_LIBS@ \
	@ELEMENTARY_ELOCATION_LIBS@ \
	@ELEMENTARY_EMAP_LIBS@ \
	@ELEMENTARY_LIBS@ \
	@ELEMENTARY_WEB_LIBS@ \
	@ELEMENTARY_DRM_LIBS@ \
	@my_libs@
elementary_test_CFLAGS =
elementary_test_LDFLAGS =

elementary_config_SOURCES = \
config.c

elementary_config_LDADD = $(top_builddir)/src/lib/libelementary.la \
        @ELEMENTARY_WAYLAND_LIBS@ \
	@ELEMENTARY_EWEATHER_LIBS@ \
	@ELEMENTARY_ELOCATION_LIBS@ \
	@ELEMENTARY_EMAP_LIBS@ \
	@ELEMENTARY_LIBS@ \
	@ELEMENTARY_WEB_LIBS@ \
	@ELEMENTARY_DRM_LIBS@ \
	@my_libs@
elementary_config_LDFLAGS =

elementary_codegen_SOURCES = \
elementary_codegen.c

elementary_codegen_LDADD = \
	$(top_builddir)/src/lib/libelementary.la \
        @ELEMENTARY_WAYLAND_LIBS@ \
	@ELEMENTARY_EWEATHER_LIBS@ \
	@ELEMENTARY_ELOCATION_LIBS@ \
	@ELEMENTARY_EMAP_LIBS@ \
	@ELEMENTARY_LIBS@ \
	@ELEMENTARY_WEB_LIBS@ \
	@ELEMENTARY_DRM_LIBS@ \
	@my_libs@
elementary_codegen_LDFLAGS =

noinst_HEADERS = \
elm_prefs_cc.h

elm_prefs_cc_SOURCES = \
elm_prefs_cc.c \
elm_prefs_cc_mem.c \
elm_prefs_cc_out.c \
elm_prefs_cc_parse.c \
elm_prefs_cc_handlers.c

elm_prefs_cc_LDADD = \
$(top_builddir)/src/lib/libelementary.la \
@ELEMENTARY_LIBS@ \
@my_libs@

if BUILD_QUICKLAUNCH
elementary_quicklaunch_SOURCES = quicklaunch.c
elementary_quicklaunch_LDADD = $(top_builddir)/src/lib/libelementary.la \
        @ELEMENTARY_WAYLAND_LIBS@ \
	@ELEMENTARY_EWEATHER_LIBS@ \
	@ELEMENTARY_ELOCATION_LIBS@ \
	@ELEMENTARY_EMAP_LIBS@ \
	@ELEMENTARY_LIBS@ \
	@ELEMENTARY_WEB_LIBS@ \
	@ELEMENTARY_DRM_LIBS@ \
	@my_libs@
elementary_quicklaunch_LDFLAGS =

if BUILD_RUN
elementary_run_SOURCES = run.c
elementary_run_LDADD =
elementary_run_LDFLAGS =
endif
endif

EXTRA_DIST = \
test_factory.c
