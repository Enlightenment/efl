## Process this file with automake to produce Makefile.in

INCLUDES = @x_cflags@ 

lib_LTLIBRARIES = libecore.la
include_HEADERS = \
Ecore.h

libecore_la_SOURCES = \
	e_ev_filter.c \
	e_ev_signal.c \
	e_ev_x.c \
	e_events.c \
	e_util.c \
	e_x.c

libecore_la_LIBADD = @x_ldflags@ @x_libs@ -lm

