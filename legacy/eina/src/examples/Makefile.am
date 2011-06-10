MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include

LDADD = \
	$(top_builddir)/src/lib/libeina.la

SRCS = \
	eina_array_01.c \
	eina_array_02.c \
	eina_hash_01.c \
	eina_hash_02.c \
	eina_list_01.c \
	eina_list_02.c \
	eina_list_03.c \
	eina_list_04.c

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
    eina_array_01 \
	eina_array_02 \
	eina_hash_01 \
	eina_hash_02 \
	eina_list_01 \
	eina_list_02 \
	eina_list_03 \
	eina_list_04
endif

