MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@

installed_headerdir = $(includedir)/eo-@VMAJ@
dist_installed_header_DATA = Eo.h

lib_LTLIBRARIES = libeo.la

libeo_la_SOURCES = eo.c eo_base_class.c eo_private.h
libeo_la_LIBADD = @EO_LIBS@
libeo_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@ @release_info@

clean-local:
	rm -rf *.gcno
