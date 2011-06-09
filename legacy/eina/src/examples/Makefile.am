MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/include \
-I$(top_builddir)/src/include

LDADD = \
	$(top_builddir)/src/lib/libeina.la

SRCS = \
	eina_hash_01.c \
	eina_hash_02.c

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	eina_hash_01 \
	eina_hash_02
endif

