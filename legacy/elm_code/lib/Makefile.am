MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/elm_code/lib \
-I$(top_builddir)/elm_code/lib \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DEFL_BETA_API_SUPPORT \
@EFL_CFLAGS@ \
-DEFL_EFL_BUILD

lib_LTLIBRARIES = libelm_code.la

includes_HEADERS = \
elm_code_file.h \
elm_code_parse.h \
elm_code_widget.h \
elm_code_diff_widget.h \
Elm_Code.h
includesdir = $(includedir)/edi-@VMAJ@

libelm_code_la_SOURCES = \
elm_code_file.c \
elm_code_parse.c \
elm_code_widget.c \
elm_code_diff_widget.c \
elm_code.c
libelm_code_la_LIBADD = @EFL_LIBS@ -lm
libelm_code_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
