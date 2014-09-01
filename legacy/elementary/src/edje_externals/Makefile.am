MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-DELM_INTERNAL_API_ARGESFSDFEFC=1 \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DELEMENTARY_BUILD \
@ELEMENTARY_CFLAGS@ \
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_WEB_CFLAGS@

pkgdir = $(libdir)/edje/modules/elm/$(MODULE_EDJE)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = private.h \
modules.inc \
elm.c \
elm_actionslider.c \
elm_bg.c \
elm_bubble.c \
elm_button.c \
elm_calendar.c \
elm_check.c \
elm_clock.c \
elm_entry.c \
elm_fileselector.c \
elm_fileselector_button.c \
elm_fileselector_entry.c \
elm_frame.c \
elm_gengrid.c \
elm_genlist.c \
elm_hoversel.c \
elm_icon.c \
elm_index.c \
elm_label.c \
elm_list.c \
elm_map.c \
elm_multibuttonentry.c \
elm_naviframe.c \
elm_notify.c \
elm_panes.c \
elm_photocam.c \
elm_progressbar.c \
elm_radio.c \
elm_scroller.c \
elm_segment_control.c \
elm_slider.c \
elm_slideshow.c \
elm_spinner.c \
elm_thumb.c \
elm_toolbar.c \
elm_video.c \
elm_web.c

module_la_LIBADD = @ELEMENTARY_LIBS@ $(top_builddir)/src/lib/libelementary.la
module_la_LDFLAGS = -no-undefined -module -avoid-version -shared -fPIC
