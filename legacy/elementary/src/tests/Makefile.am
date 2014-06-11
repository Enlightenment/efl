AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = elm_suite.h

TESTS = elm_suite
check_PROGRAMS = elm_suite
elm_suite_SOURCES = \
	elm_suite.c \
	elm_test_atspi.c \
	elm_test_check.c \
	elm_test_colorselector.c \
	elm_test_entry.c \
	elm_test_init.c \
	elm_test_list.c \
	elm_test_button.c \
	elm_test_image.c \
	elm_test_photo.c \
	elm_test_actionslider.c \
	elm_test_box.c \
	elm_test_table.c \
	elm_test_thumb.c \
	elm_test_menu.c \
	elm_test_photocam.c \
	elm_test_win.c \
	elm_test_icon.c \
	elm_test_prefs.c \
	elm_test_map.c \
	elm_test_glview.c \
	elm_test_web.c \
	elm_test_toolbar.c \
	elm_test_grid.c \
	elm_test_diskselector.c \
	elm_test_notify.c \
	elm_test_mapbuf.c \
	elm_test_flip.c \
	elm_test_layout.c \
	elm_test_slider.c \
	elm_test_ctxpopup.c \
	elm_test_separator.c \
	elm_test_calendar.c \
	elm_test_inwin.c \
	elm_test_gengrid.c \
	elm_test_radio.c \
	elm_test_scroller.c \
	elm_test_frame.c \
	elm_test_datetime.c \
	elm_test_player.c \
	elm_test_bg.c \
	elm_test_video.c \
	elm_test_segmentcontrol.c \
	elm_test_progressbar.c \
	elm_test_fileselector.c \
	elm_test_fileselector_button.c \
	elm_test_fileselector_entry.c \
	elm_test_hoversel.c \
	elm_test_multibuttonentry.c \
	elm_test_naviframe.c \
	elm_test_popup.c \
	elm_test_bubble.c \
	elm_test_clock.c \
	elm_test_conformant.c \
	elm_test_dayselector.c \
	elm_test_flipselector.c \
	elm_test_genlist.c \
	elm_test_hover.c \
	elm_test_index.c \
	elm_test_label.c \
	elm_test_panel.c \
	elm_test_panes.c \
	elm_test_slideshow.c \
	elm_test_spinner.c \
	elm_test_plug.c

elm_suite_CPPFLAGS = \
	-DTESTS_BUILD_DIR=\"${top_builddir}/src/tests\" \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	@CHECK_CFLAGS@ \
	@ELEMENTARY_CFLAGS@ \
	@ELEMENTARY_X_CFLAGS@ \
	@ELEMENTARY_FB_CFLAGS@ \
	@ELEMENTARY_SDL_CFLAGS@ \
	@ELEMENTARY_WIN32_CFLAGS@ \
	@ELEMENTARY_ELOCATION_CFLAGS@ \
	@ELEMENTARY_EWEATHER_CFLAGS@ \
	@ELEMENTARY_WEB_CFLAGS@ \
	@ELEMENTARY_EMAP_CFLAGS@ \
	@ELEMENTARY_WAYLAND_CFLAGS@ \
	@EVIL_CFLAGS@

elm_suite_LDADD = \
	$(top_builddir)/src/lib/libelementary.la \
	@CHECK_LIBS@ \
	@ELEMENTARY_EWEATHER_LIBS@ \
	@ELEMENTARY_ELOCATION_LIBS@ \
	@ELEMENTARY_EMAP_LIBS@ \
	@ELEMENTARY_LIBS@ \
	@ELEMENTARY_WEB_LIBS@ \
	@my_libs@
