## Process this file with automake to produce Makefile.in

INCLUDES = \
-I. -I$(top_srcdir)/src/lib -I$(top_srcdir) \
@DIRECTFB_CFLAGS@ @qt_cflags@ @gl_cflags@

bin_PROGRAMS = \
@ENGINE_SOFTWARE_X11_PRG@ \
@ENGINE_DIRECTFB_PRG@ \
@ENGINE_FB_PRG@ \
@ENGINE_BUFFER_PRG@ \
@ENGINE_SOFTWARE_QTOPIA_PRG@ \
@ENGINE_GL_X11_PRG@

EXTRA_PROGRAMS = \
evas_software_x11_test \
evas_software_x11_perf_test \
evas_software_x11_perf_load \
evas_directfb_test \
evas_directfb_window \
evas_fb_test \
evas_buffer_test \
evas_software_qtopia_test \
evas_gl_x11_test

SUBDIRS = evas_software_win32

evas_software_x11_test_SOURCES = evas_test_main.h evas_test_main.c evas_software_x11_main.c
evas_software_x11_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm @x_libs@
evas_software_x11_test_LDFLAGS = $(LDFLAGS)
evas_software_x11_test_CFLAGS = $(CFLAGS) @x_cflags@
evas_software_x11_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_software_x11_perf_test_SOURCES = evas_software_x11_perf_test.c
evas_software_x11_perf_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm @x_libs@
evas_software_x11_perf_test_LDFLAGS = $(LDFLAGS)
evas_software_x11_perf_test_CFLAGS = $(CFLAGS) @x_cflags@
evas_software_x11_perf_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_software_x11_perf_load_SOURCES = evas_software_x11_perf_load.c
evas_software_x11_perf_load_LDADD = $(top_builddir)/src/lib/libevas.la -lm @x_libs@
evas_software_x11_perf_load_LDFLAGS = $(LDFLAGS)
evas_software_x11_perf_load_CFLAGS = $(CFLAGS) @x_cflags@
evas_software_x11_perf_load_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_directfb_test_SOURCES = evas_test_main.h evas_test_main.c evas_directfb_main.c
evas_directfb_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm @DIRECTFB_LIBS@ 
evas_directfb_test_LDFLAGS = $(LDFLAGS)
evas_directfb_test_CFLAGS = $(CFLAGS) @DIRECTFB_CFLAGS@
evas_directfb_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_directfb_window_SOURCES = evas_test_main.h evas_test_main.c evas_directfb_window.c
evas_directfb_window_LDADD = $(top_builddir)/src/lib/libevas.la -lm @DIRECTFB_LIBS@
evas_directfb_window_LDFLAGS = $(LDFLAGS) 
evas_directfb_window_CFLAGS = $(CFLAGS) @DIRECTFB_CFLAGS@
evas_directfb_window_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_fb_test_SOURCES = evas_test_main.h evas_test_main.c evas_fb_main.c
evas_fb_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm
evas_fb_test_LDFLAGS = $(LDFLAGS)
evas_fb_test_CFLAGS = $(CFLAGS)
evas_fb_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

evas_buffer_test_SOURCES = evas_test_main.h evas_test_main.c evas_buffer_main.c
evas_buffer_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm
evas_buffer_test_LDFLAGS = $(LDFLAGS)
evas_buffer_test_CFLAGS = $(CFLAGS)
evas_buffer_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

if BUILD_ENGINE_SOFTWARE_QTOPIA
moc_evas_software_qtopia_main.cpp: evas_software_qtopia_main.h
	@qt_moc@ evas_software_qtopia_main.h -o moc_evas_software_qtopia_main.cpp

evas_software_qtopia_test_SOURCES = evas_test_main.h evas_test_main.c evas_software_qtopia_main.h evas_software_qtopia_main.cpp moc_evas_software_qtopia_main.cpp
evas_software_qtopia_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm @qt_libs@
evas_software_qtopia_test_LDFLAGS = $(LDFLAGS)
evas_software_qtopia_test_CFLAGS = $(CFLAGS) @qt_cflags@
evas_software_qtopia_test_CXXFLAGS = $(CFLAGS) $(CXXFLAGS) @qt_cflags@
evas_software_qtopia_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la
endif

evas_gl_x11_test_SOURCES = evas_test_main.h evas_test_main.c evas_gl_x11_main.c
evas_gl_x11_test_LDADD = $(top_builddir)/src/lib/libevas.la -lm @x_libs@ @gl_libs@
evas_gl_x11_test_LDFLAGS = $(LDFLAGS)
evas_gl_x11_test_CFLAGS = $(CFLAGS) @x_cflags@ @gl_cflags@
evas_gl_x11_test_DEPENDENCIES = $(top_builddir)/src/lib/libevas.la

EXTRA_DIST = evas_software_qtopia_main.h evas_software_qtopia_main.cpp
