MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/elm_code/lib \
-DEFL_BETA_API_SUPPORT \
-DEFL_EO_API_SUPPORT \
@EFL_CFLAGS@ \
-DEFL_ELM_CODE_BUILD

lib_LTLIBRARIES = libelm_code.la

includes_HEADERS = \
elm_code_file.h \
elm_code_parse.h \
elm_code_widget2.eo.h \
elm_code_diff_widget.h \
Elm_Code.h
includesdir = $(includedir)/edi-@VMAJ@

libelm_code_la_SOURCES = \
elm_code_file.c \
elm_code_parse.c \
elm_code_widget2.c \
elm_code_diff_widget.c \
elm_code.c \
elm_code_private.h

libelm_code_la_LIBADD = @EFL_LIBS@ -lm
libelm_code_la_LDFLAGS = -no-undefined @EFL_LTLIBRARY_FLAGS@
