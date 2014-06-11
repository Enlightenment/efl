MAINTAINERCLEANFILES = Makefile.in

include ../../../Makefile_Elm_Helpers.am

EXTRA_DIST = \
elm_prefs_swallow.edc

filesdir = $(libdir)/elementary/modules/prefs/$(MODULE_ARCH)
files_DATA = elm_prefs_swallow.edj

elm_prefs_swallow.edj: Makefile elm_prefs_swallow.edc
	$(AM_V_EDJ)$(EDJE_CC) $(EDJE_CC_FLAGS) \
        $(top_srcdir)/src/modules/prefs/elm_prefs_swallow.edc \
	$(top_builddir)/src/modules/prefs/elm_prefs_swallow.edj

AM_CPPFLAGS = \
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
@ELEMENTARY_ELOCATION_CFLAGS@ \
@ELEMENTARY_WEB_CFLAGS@ \
@ELEMENTARY_EMAP_CFLAGS@ \
@ELEMENTARY_WAYLAND_CFLAGS@ \
@EVIL_CFLAGS@

if ELEMENTARY_WINDOWS_BUILD
AM_CPPFLAGS += -DELEMENTARY_BUILD
endif

pkgdir = $(libdir)/elementary/modules/prefs/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la

module_la_SOURCES = \
private.h \
item_widgets.inc \
page_widgets.inc \
prefs_iface.c \
elm_button.c \
elm_check.c \
elm_datetime.c \
elm_entry.c \
elm_label.c \
elm_separator.c \
elm_slider.c \
elm_spinner.c \
elm_swallow.c \
elm_vertical_box.c \
elm_horizontal_box.c \
elm_vertical_frame.c \
elm_horizontal_frame.c

module_la_LIBADD = @ELEMENTARY_LIBS@ $(top_builddir)/src/lib/libelementary.la @EVIL_LIBS@
module_la_LDFLAGS = -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

clean-local:
	rm -f *.edj
