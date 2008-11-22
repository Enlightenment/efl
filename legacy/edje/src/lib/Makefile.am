
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir) \
-I$(top_srcdir)/src/bin \
-I$(top_srcdir)/src/lib \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@EVIL_CFLAGS@ \
@EDJE_CFLAGS@ \
@EFL_EDJE_BUILD@

lib_LTLIBRARIES = libedje.la

include_HEADERS = Edje.h Edje_Edit.h

libedje_la_SOURCES  = \
edje_calc.c \
edje_callbacks.c \
edje_data.c \
edje_embryo.c \
edje_load.c \
edje_main.c \
edje_misc.c \
edje_program.c \
edje_smart.c \
edje_text.c \
edje_util.c \
edje_var.c \
edje_container.c \
edje_container.h \
edje_message_queue.c \
edje_private.h \
edje_cache.c \
edje_match.c \
edje_textblock_styles.c \
edje_edit.c \
edje_script_only.c \
edje_entry.c

libedje_la_LIBADD = @EDJE_LIBS@ -lm
libedje_la_LDFLAGS = -no-undefined @lt_enable_auto_import@ -version-info @version_info@
