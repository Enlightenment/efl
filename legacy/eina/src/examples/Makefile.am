MAINTAINERCLEANFILES = Makefile.in

examplesdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include

LDADD = \
	$(top_builddir)/src/lib/libeina.la

SRCS = \
	eina_accessor_01.c \
	eina_array_01.c \
	eina_array_02.c \
	eina_error_01.c \
	eina_file_01.c \
	eina_hash_01.c \
	eina_hash_02.c \
	eina_hash_03.c \
	eina_hash_04.c \
	eina_hash_05.c \
	eina_hash_06.c \
	eina_hash_07.c \
	eina_hash_08.c \
	eina_iterator_01.c \
	eina_list_01.c \
	eina_list_02.c \
	eina_list_03.c \
	eina_list_04.c \
	eina_log_01.c \
	eina_log_02.c \
	eina_log_03.c \
	eina_inlist_01.c \
	eina_inlist_02.c \
	eina_inlist_03.c \
	eina_str_01.c \
	eina_strbuf_01.c \
	eina_tiler_01.c \
	eina_model_01.c \
	eina_model_02.c \
	eina_model_03.c \
	eina_value_01.c

examples_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
examples_PROGRAMS += \
	eina_accessor_01 \
	eina_array_01 \
	eina_array_02 \
	eina_error_01 \
	eina_file_01 \
	eina_hash_01 \
	eina_hash_02 \
	eina_hash_03 \
	eina_hash_04 \
	eina_hash_05 \
	eina_hash_06 \
	eina_hash_07 \
	eina_hash_08 \
	eina_iterator_01 \
	eina_list_01 \
	eina_list_02 \
	eina_list_03 \
	eina_list_04 \
	eina_log_01 \
	eina_log_02 \
	eina_log_03 \
	eina_inlist_01 \
	eina_inlist_02 \
	eina_inlist_03 \
	eina_str_01 \
	eina_strbuf_01 \
	eina_model_01 \
	eina_model_02 \
	eina_model_03 \
	eina_model_04 \
	eina_value_01

eina_model_04_SOURCES = \
	eina_model_04_animal.c \
	eina_model_04_child.c \
	eina_model_04_human.c \
	eina_model_04_main.c \
	eina_model_04_parrot.c \
	eina_model_04_whistler.c


if BUILD_TILER_EXAMPLE
AM_CPPFLAGS += @ECORE_EVAS_CFLAGS@
examples_PROGRAMS += eina_tiler_01
eina_tiler_01_LDADD = $(top_builddir)/src/lib/libeina.la @ECORE_EVAS_LIBS@
endif

endif
