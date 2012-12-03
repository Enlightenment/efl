MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_evas \
-DPACKAGE_EXAMPLES_DIR=\"$(datadir)/ecore/examples\" \
@ECORE_CFLAGS@

EXTRA_PROGRAMS = \
ecore_animator_example \
ecore_client_bench \
ecore_con_client_simple_example \
ecore_con_lookup_example \
ecore_con_server_http_example \
ecore_con_server_simple_example \
ecore_con_url_cookies_example \
ecore_con_url_download_example \
ecore_con_url_headers_example \
ecore_evas_basics_example \
ecore_evas_buffer_example_01 \
ecore_evas_buffer_example_02 \
ecore_evas_callbacks \
ecore_evas_ews_example \
ecore_evas_object_example \
ecore_evas_window_sizes_example \
ecore_event_example_01 \
ecore_event_example_02 \
ecore_exe_example \
ecore_exe_example_child \
ecore_fd_handler_example \
ecore_idler_example \
ecore_job_example \
ecore_pipe_simple_example \
ecore_poller_example \
ecore_server_bench \
ecore_thread_example \
ecore_time_functions_example \
ecore_timer_example

ecore_animator_example_SOURCES = ecore_animator_example.c
ecore_animator_example_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_client_bench_SOURCES = ecore_client_bench.c
ecore_client_bench_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_client_simple_example_SOURCES = ecore_con_client_simple_example.c
ecore_con_client_simple_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_lookup_example_SOURCES = ecore_con_lookup_example.c
ecore_con_lookup_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_server_http_example_SOURCES = ecore_con_server_http_example.c
ecore_con_server_http_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_server_simple_example_SOURCES = ecore_con_server_simple_example.c
ecore_con_server_simple_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_url_cookies_example_SOURCES = ecore_con_url_cookies_example.c
ecore_con_url_cookies_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_url_download_example_SOURCES = ecore_con_url_download_example.c
ecore_con_url_download_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_con_url_headers_example_SOURCES = ecore_con_url_headers_example.c
ecore_con_url_headers_example_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_basics_example_SOURCES = ecore_evas_basics_example.c
ecore_evas_basics_example_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_buffer_example_01_SOURCES = ecore_evas_buffer_example_01.c
ecore_evas_buffer_example_01_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_buffer_example_02_SOURCES = ecore_evas_buffer_example_02.c
ecore_evas_buffer_example_02_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_callbacks_SOURCES = ecore_evas_callbacks.c
ecore_evas_callbacks_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_ews_example_SOURCES = ecore_evas_ews_example.c
ecore_evas_ews_example_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_object_example_SOURCES = ecore_evas_object_example.c
ecore_evas_object_example_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_evas_window_sizes_example_SOURCES = ecore_evas_window_sizes_example.c
ecore_evas_window_sizes_example_LDADD = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_event_example_01_SOURCES = ecore_event_example_01.c
ecore_event_example_01_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_event_example_02_SOURCES = ecore_event_example_02.c
ecore_event_example_02_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_exe_example_SOURCES = ecore_exe_example.c
ecore_exe_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_exe_example_child_SOURCES = ecore_exe_example_child.c
ecore_exe_example_child_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_fd_handler_example_SOURCES = ecore_fd_handler_example.c
ecore_fd_handler_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_idler_example_SOURCES = ecore_idler_example.c
ecore_idler_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_job_example_SOURCES = ecore_job_example.c
ecore_job_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_pipe_simple_example_SOURCES = ecore_pipe_simple_example.c
ecore_pipe_simple_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_poller_example_SOURCES = ecore_poller_example.c
ecore_poller_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_server_bench_SOURCES = ecore_server_bench.c
ecore_server_bench_LDADD = $(top_builddir)/src/lib/ecore_con/libecore_con.la $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_thread_example_SOURCES = ecore_thread_example.c
ecore_thread_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_time_functions_example_SOURCES = ecore_time_functions_example.c
ecore_time_functions_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

ecore_timer_example_SOURCES = ecore_timer_example.c
ecore_timer_example_LDADD = $(top_builddir)/src/lib/ecore/libecore.la @ECORE_LIBS@

SRCS = \
ecore_animator_example \
ecore_client_bench \
ecore_con_client_simple_example \
ecore_con_lookup_example \
ecore_con_server_http_example \
ecore_con_server_simple_example \
ecore_con_url_cookies_example \
ecore_con_url_download_example \
ecore_con_url_headers_example \
ecore_evas_basics_example \
ecore_evas_buffer_example_01 \
ecore_evas_buffer_example_02 \
ecore_evas_callbacks \
ecore_evas_ews_example \
ecore_evas_object_example \
ecore_evas_window_sizes_example \
ecore_event_example_01 \
ecore_event_example_02 \
ecore_exe_example \
ecore_exe_example_child \
ecore_fd_handler_example \
ecore_idler_example \
ecore_job_example \
ecore_pipe_simple_example \
ecore_poller_example \
ecore_server_bench \
ecore_thread_example \
ecore_time_functions_example \
ecore_timer_example

EXTRA_DIST = red.png

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	mkdir -p $(datadir)/ecore/examples
	$(install_sh_DATA) -c $(SRCS) $(datadir)/ecore/examples
	$(install_sh_DATA) -c  red.png $(datadir)/ecore/examples

uninstall-local:
	rm -f $(datadir)/ecore/examples/red.png
	for f in $(SRCS) ; do \
	  rm -f $(datadir)/ecore/examples/$$f ; \
	done
