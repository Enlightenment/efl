MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

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
	eina_iterator_01.c \
	eina_list_01.c \
	eina_list_02.c \
	eina_list_03.c \
	eina_list_04.c \
	eina_inlist_01.c \
	eina_inlist_02.c \
	eina_inlist_03.c

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	eina_accessor_01 \
    eina_array_01 \
	eina_array_02 \
	eina_error_01 \
	eina_file_01 \
	eina_hash_01 \
	eina_hash_02 \
	eina_iterator_01 \
	eina_list_01 \
	eina_list_02 \
	eina_list_03 \
	eina_list_04 \
	eina_inlist_01 \
	eina_inlist_02 \
	eina_inlist_03
endif

