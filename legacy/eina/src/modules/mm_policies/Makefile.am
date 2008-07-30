MAINTAINERCLEANFILES = \
Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include

controllerdir = $(libdir)/eina/mm_policies
controller_LTLIBRARIES = fixed.la ememoa.la

fixed_la_SOURCES = \
fixed.c

fixed_la_LIBADD       = $(top_builddir)/src/lib/libeina.la
fixed_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
fixed_la_DEPENDENCIES = $(top_builddir)/src/lib/libeina.la

ememoa_la_SOURCES = \
ememoa.c

ememoa_la_LIBADD       = $(top_builddir)/src/lib/libeina.la
ememoa_la_LDFLAGS      = -no-undefined @lt_enable_auto_import@ -module -avoid-version
ememoa_la_DEPENDENCIES = $(top_builddir)/src/lib/libeina.la
