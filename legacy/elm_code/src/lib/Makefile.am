MAINTAINERCLEANFILES = Makefile.in

CLEANFILES=

EOLIAN_FLAGS = @DEPS_EOLIAN_FLAGS@ \
               -I$(top_srcdir)/elm_code/src/lib

include $(top_srcdir)/Makefile_Eolian_Helper.am

AM_CPPFLAGS = \
-I$(top_srcdir)/elm_code/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DEFL_BETA_API_SUPPORT \
-DEFL_EO_API_SUPPORT \
@EFL_CFLAGS@ \
-DEFL_ELM_CODE_BUILD

lib_LTLIBRARIES = libelm_code.la

includes_HEADERS = \
elm_code_common.h \
elm_code_line.h \
elm_code_text.h \
elm_code_file.h \
elm_code_parse.h \
widget/elm_code_widget.eo.h \
widget/elm_code_widget_selection.h \
elm_code_diff_widget.h \
Elm_Code.h
includesdir = $(includedir)/edi-@VMAJ@

libelm_code_la_SOURCES = \
elm_code_line.c \
elm_code_text.c \
elm_code_file.c \
elm_code_parse.c \
widget/elm_code_widget_tooltip.c \
widget/elm_code_widget_selection.c \
widget/elm_code_widget.c \
elm_code_diff_widget.c \
elm_code.c \
elm_code_private.h \
widget/elm_code_widget_private.h

libelm_code_la_LIBADD = @EFL_LIBS@ -lm
libelm_code_la_LDFLAGS = -no-undefined @EFL_LTLIBRARY_FLAGS@

elm_code_eolian_files = \
widget/elm_code_widget.eo

elm_code_eolian_c = $(elm_code_eolian_files:%.eo=%.eo.c)
elm_code_eolian_h = $(elm_code_eolian_files:%.eo=%.eo.h)

BUILT_SOURCES = \
        $(elm_code_eolian_c) \
        $(elm_code_eolian_h)

elmcodeeolianfilesdir = $(datadir)/eolian/include/elm_code-@VMAJ@
elmcodeeolianfiles_DATA = $(elm_code_eolian_files)
EXTRA_DIST = ${elmcodeeolianfiles_DATA}
