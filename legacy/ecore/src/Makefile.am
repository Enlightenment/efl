## Process this file with automake to produce Makefile.in

INCLUDES = \
        @evas_cflags@ @edb_cflags@

lib_LTLIBRARIES = libecore.la

libecore_la_SOURCES = \
	e_ev_filter.c e_ev_filter.h \
	e_ev_signal.c e_ev_signal.h \
	e_ev_x.c e_ev_x.h \
	e_events.c e_events.h \
	e_mem.c e_mem.h \
	e_str.c e_str.h \
	e_util.c e_util.h \
	e_x.c e_x.h

libecore_la_LIBADD = @evas_libs@ @edb_libs@ -lm

