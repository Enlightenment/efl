CFLAGS_X      = @x_cflags@
CFLAGS_EXTRA  = -I$(includedir) -I$(top_srcdir) -I$(top_srcdir)/src -I/usr/local/include

CFLGAS_ALL    = $(CFLAGS_X) $(CFLAGS_EXTRA)

SUBDIRS       = fnt img

INCLUDES = $(CFLGAS_ALL) \
           -DLIBDIR=\"$(libdir)\" \
	   -DBINDIR=\"$(bindir)\" \
	   -DDATADIR=\"$(datadir)\"
								     
bin_PROGRAMS = evas_test evas_test_old

evas_test_DEPENDENCIES = $(top_srcdir)/src/libevas.la

evas_test_SOURCES = evas_test.c

evas_test_LDFLAGS = -static

evas_test_LDADD = $(top_builddir)/src/libevas.la

evas_test_old_DEPENDENCIES = $(top_srcdir)/src/libevas.la

evas_test_old_SOURCES = evas_test_old.c

evas_test_old_LDFLAGS = -static

evas_test_old_LDADD = $(top_builddir)/src/libevas.la
