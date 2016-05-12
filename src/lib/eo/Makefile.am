EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

### Library
eo_eolian_files = \
	eo_base.eo \
	eo_class.eo \
	eo_interface.eo

eo_eolian_c = $(eo_eolian_files:%.eo=%.eo.c)
eo_eolian_h = $(eo_eolian_files:%.eo=%.eo.h)

BUILT_SOURCES = $(eo_eolian_c) \
                $(eo_eolian_h)

CLEANFILES = $(eo_eolian_c) \
             $(eo_eolian_h)

lib_LTLIBRARIES = libeo.la

installed_eomainheadersdir = $(includedir)/eo-@VMAJ@
dist_installed_eomainheaders_DATA = Eo.h

nodist_installed_eomainheaders_DATA = $(eo_eolian_h)

libeo_la_SOURCES = \
eo.c \
eo_ptr_indirection.c \
eo_ptr_indirection.h \
eo_base_class.c \
eo_class_class.c \
eo_add_fallback.c \
eo_add_fallback.h \
eo_private.h

libeo_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl @EO_CFLAGS@
libeo_la_LIBADD = @EO_SUBBUILD_LIBS@
libeo_la_DEPENDENCIES = @EO_SUBBUILD_INTERNAL_LIBS@
libeo_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

eoeolianfilesdir = $(datadir)/eolian/include/eo-@VMAJ@
eoeolianfiles_DATA = $(eo_eolian_files) \
		     eina_types.eot # We don't generate .c/.h on purpose
