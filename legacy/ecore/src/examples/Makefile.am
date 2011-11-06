MAINTAINERCLEANFILES = Makefile.in

pkglibdir = $(datadir)/$(PACKAGE)/examples

filesdir = $(datadir)/$(PACKAGE)/examples
files_DATA =

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_con \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/$(PACKAGE)/examples\" \
@GLIB_CFLAGS@ @EVIL_CFLAGS@ @EVAS_CFLAGS@ @EINA_CFLAGS@ @WIN32_CPPFLAGS@ @EFL_ECORE_BUILD@

ECOREBASELDADD = \
	$(top_builddir)/src/lib/ecore/libecore.la \
	$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
	@dlopen_libs@ @EINA_LIBS@ @EVIL_LIBS@ @EVAS_LIBS@ @GLIB_LIBS@ @WIN32_LIBS@ @LTLIBINTL@ @EFL_PTHREAD_LIBS@ @rt_libs@ -lm


LDADD = \
	$(ECOREBASELDADD)

SRCS = \
	ecore_animator_example.c \
	ecore_fd_handler_example.c \
	ecore_poller_example.c \
	ecore_event_example.c \
	ecore_idler_example.c \
	ecore_timer_example.c \
	ecore_time_functions_example.c \
	ecore_job_example.c \
	ecore_con_lookup_example.c \
	ecore_con_url_headers_example.c \
	ecore_con_url_download_example.c \
	ecore_con_url_cookies_example.c \
	ecore_con_server_simple_example.c \
	ecore_con_server_http_example.c \
	ecore_con_client_simple_example.c \
	ecore_client_bench.c \
	ecore_server_bench.c \
	ecore_con_client_example.c \
	ecore_con_server_example.c \
	ecore_fd_handler_gnutls_example.c \
	ecore_file_download_example.c \
	ecore_pipe_simple_example.c \
	ecore_pipe_gstreamer_example.c \
	ecore_thread_example.c \
	ecore_evas_callbacks.c \
	ecore_evas_window_sizes_example.c \
	ecore_evas_object_example.c \
	ecore_evas_basics_example.c \
	ecore_evas_buffer_example_01.c \
	ecore_evas_buffer_example_02.c \
	ecore_evas_ews_example.c

EXTRA_DIST = $(SRCS) \
	$(srcdir)/red.png

pkglib_PROGRAMS =

if EFL_INSTALL_EXAMPLES
files_DATA += $(SRCS) \
	$(srcdir)/red.png
endif

if EFL_BUILD_EXAMPLES
pkglib_PROGRAMS += \
	ecore_animator_example \
	ecore_fd_handler_example \
	ecore_poller_example \
	ecore_event_example \
	ecore_idler_example \
	ecore_job_example \
	ecore_timer_example \
	ecore_time_functions_example \
	ecore_pipe_simple_example \
	ecore_con_lookup_example \
	ecore_con_url_headers_example \
	ecore_con_url_download_example \
	ecore_con_url_cookies_example \
	ecore_con_server_simple_example \
	ecore_con_server_http_example \
	ecore_con_client_simple_example \
	ecore_thread_example \
	ecore_evas_callbacks \
	ecore_evas_window_sizes_example \
	ecore_evas_object_example \
	ecore_evas_basics_example \
	ecore_evas_buffer_example_01 \
	ecore_evas_buffer_example_02 \
	ecore_evas_ews_example \
	ecore_client_bench \
	ecore_server_bench

ecore_con_lookup_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_url_headers_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_url_download_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_url_cookies_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_server_simple_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_server_http_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_con_client_simple_example_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_evas_window_sizes_example_LDADD = $(ECOREBASELDADD) @EVAS_LIBS@ $(top_builddir)/src/lib/ecore_evas/libecore_evas.la
ecore_evas_buffer_example_01_LDADD = $(ECOREBASELDADD) @EVAS_LIBS@ $(top_builddir)/src/lib/ecore_evas/libecore_evas.la
ecore_evas_buffer_example_02_LDADD = $(ECOREBASELDADD) @EVAS_LIBS@ $(top_builddir)/src/lib/ecore_evas/libecore_evas.la
ecore_client_bench_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la
ecore_server_bench_LDADD = $(ECOREBASELDADD) $(top_builddir)/src/lib/ecore_con/libecore_con.la

endif
