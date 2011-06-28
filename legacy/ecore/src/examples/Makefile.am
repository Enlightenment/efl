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
	ecore_fd_handler_example.c \
	ecore_event_example.c \
	ecore_idler_example.c \
	ecore_time_example.c \
	ecore_job_example.c \
	client_bench.c \
	server_bench.c \
	ecore_con_client_example.c \
	ecore_con_server_example.c \
	ecore_fd_handler_gnutls_example.c \
	ecore_file_download_example.c

EXTRA_DIST = $(SRCS)

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA = $(SRCS)
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	ecore_fd_handler_example \
	ecore_event_example \
	ecore_idler_example \
	ecore_job_example \
	ecore_time_example
endif

