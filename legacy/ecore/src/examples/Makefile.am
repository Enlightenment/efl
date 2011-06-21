MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/ecore \
@GLIB_CFLAGS@ @EVIL_CFLAGS@ @EINA_CFLAGS@ @WIN32_CPPFLAGS@ @EFL_ECORE_BUILD@

LDADD = \
	$(top_builddir)/src/lib/ecore/libecore.la \
	@dlopen_libs@ @EINA_LIBS@ @EVIL_LIBS@ @GLIB_LIBS@ @WIN32_LIBS@ @LTLIBINTL@ @EFL_PTHREAD_LIBS@ @rt_libs@ -lm

SRCS = \
	client_bench.c \
	server_bench.c \
	ecore_con_client_example.c \
	ecore_con_server_example.c \
	ecore_fd_handler_example.c \
	ecore_file_download_example.c

EXTRA_DIST = $(SRCS)

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS +=
endif

