MAINTAINERCLEANFILES = Makefile.in
CLEANFILES =
BUILT_SOURCES =

include Makefile_File.am

lib_LTLIBRARIES = libeina.la

installed_einaheadersdir = $(includedir)/eina-@VMAJ@/eina
dist_installed_einaheaders_DATA = $(notdir $(EINA_H))

libeina_la_SOURCES = $(notdir $(EINA_S))
libeina_la_CPPFLAGS = $(EINA_FLAGS)
libeina_la_LIBADD = @EINA_LIBS@ @UNWIND_LIBS@
libeina_la_DEPENDENCIES = @EINA_INTERNAL_LIBS@
libeina_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

libeina_la-%.Plo: lib_eina_libeina_la-%.Plo
	ln -fs $(subst $(top_srcdir)/src/lib/eina/,,$<) $(subst $(top_srcdir)/src/lib/eina/,,$@)
