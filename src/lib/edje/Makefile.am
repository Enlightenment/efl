EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

### Library

edje_eolian_files = \
	edje_object.eo \
	edje_edit.eo

edje_eolian_type_files = \
	edje_types.eot

edje_eolian_c = $(edje_eolian_files:%.eo=%.eo.c)
edje_eolian_h = $(edje_eolian_files:%.eo=%.eo.h) \
                $(edje_eolian_files:%.eo=%.eo.legacy.h) \
                $(edje_eolian_type_files:%.eot=%.eot.h)

BUILT_SOURCES = $(edje_eolian_c) \
                $(edje_eolian_h)

edjeeolianfilesdir = $(datadir)/eolian/include/edje-@VMAJ@
edjeeolianfiles_DATA = $(edje_eolian_files) \
                       $(edje_eolian_type_files)

lib_LTLIBRARIES = libedje.la

EDJE_COMMON_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
-I$(top_builddir) \
-I$(top_srcdir)/src/lib/edje \
-I$(top_builddir)/src/lib/edje \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/edje\" \
-DPACKAGE_BUILD_DIR=\"$(abs_top_builddir)\" \
-DPACKAGE_SRC_DIR=\"$(abs_top_srcdir)\" \
@EDJE_CFLAGS@

installed_edjemainheadersdir = $(includedir)/edje-@VMAJ@
dist_installed_edjemainheaders_DATA = \
Edje.h \
Edje_Common.h \
Edje_Eo.h \
Edje_Legacy.h \
Edje_Edit.h

nodist_installed_edjemainheaders_DATA = $(edje_eolian_h)

libedje_la_SOURCES = \
edje_private.h \
edje_convert.h \
edje_box_layout.c \
edje_cache.c \
edje_calc.c \
edje_callbacks.c \
edje_data.c \
edje_edit.c \
edje_embryo.c \
edje_entry.c \
edje_external.c \
edje_load.c \
edje_lua.c \
edje_lua2.c \
edje_lua_script_only.c \
edje_main.c \
edje_match.c \
edje_message_queue.c \
edje_misc.c \
edje_module.c \
edje_multisense.c \
edje_program.c \
edje_smart.c \
edje_text.c \
edje_textblock_styles.c \
edje_util.c \
edje_var.c \
edje_signal.c

libedje_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl $(EDJE_COMMON_CPPFLAGS)
libedje_la_LIBADD = @EDJE_SUBBUILD_LIBS@
libedje_la_DEPENDENCIES = @EDJE_SUBBUILD_INTERNAL_LIBS@
libedje_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
