INCLUDES = \
        -I$(top_srcdir)/src -I/usr/local/include

noinst_PROGRAMS = evas_test

evas_test_DEPENDENCIES = $(top_srcdir)/src/libevas.la

evas_test_SOURCES = evas_test.c

evas_test_LDFLAGS = -static 

evas_test_LDADD = $(top_builddir)/src/libevas.la -lImlib2 -lttf -lX11 -lXext -ldl
