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
	elm_test_list.c

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
	@ELEMENTARY_WINCE_CFLAGS@ \
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
