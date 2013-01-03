MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_audio \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_audio \
-DPACKAGE_EXAMPLES_DIR=\".\" \
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

ECORE_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
@ECORE_LDFLAGS@

if HAVE_ECORE_AUDIO
EXTRA_PROGRAMS += \
ecore_audio_custom \
ecore_audio_playback \
ecore_audio_to_ogg

ECORE_AUDIO_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_audio/libecore_audio.la \
$(ECORE_COMMON_LDADD)

ecore_audio_custom_SOURCES = ecore_audio_custom.c
ecore_audio_custom_LDADD = $(ECORE_AUDIO_COMMON_LDADD)

ecore_audio_playback_SOURCES = ecore_audio_playback.c
ecore_audio_playback_LDADD = $(ECORE_AUDIO_COMMON_LDADD)

ecore_audio_to_ogg_SOURCES = ecore_audio_to_ogg.c
ecore_audio_to_ogg_LDADD = $(ECORE_AUDIO_COMMON_LDADD)
endif

ECORE_EVAS_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(ECORE_COMMON_LDADD)

ECORE_CON_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(ECORE_COMMON_LDADD)

ecore_animator_example_SOURCES = ecore_animator_example.c
ecore_animator_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_client_bench_SOURCES = ecore_client_bench.c
ecore_client_bench_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_client_simple_example_SOURCES = ecore_con_client_simple_example.c
ecore_con_client_simple_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_lookup_example_SOURCES = ecore_con_lookup_example.c
ecore_con_lookup_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_server_http_example_SOURCES = ecore_con_server_http_example.c
ecore_con_server_http_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_server_simple_example_SOURCES = ecore_con_server_simple_example.c
ecore_con_server_simple_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_url_cookies_example_SOURCES = ecore_con_url_cookies_example.c
ecore_con_url_cookies_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_url_download_example_SOURCES = ecore_con_url_download_example.c
ecore_con_url_download_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_url_headers_example_SOURCES = ecore_con_url_headers_example.c
ecore_con_url_headers_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_evas_basics_example_SOURCES = ecore_evas_basics_example.c
ecore_evas_basics_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_buffer_example_01_SOURCES = ecore_evas_buffer_example_01.c
ecore_evas_buffer_example_01_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_buffer_example_02_SOURCES = ecore_evas_buffer_example_02.c
ecore_evas_buffer_example_02_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_callbacks_SOURCES = ecore_evas_callbacks.c
ecore_evas_callbacks_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_ews_example_SOURCES = ecore_evas_ews_example.c
ecore_evas_ews_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_object_example_SOURCES = ecore_evas_object_example.c
ecore_evas_object_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_window_sizes_example_SOURCES = ecore_evas_window_sizes_example.c
ecore_evas_window_sizes_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_event_example_01_SOURCES = ecore_event_example_01.c
ecore_event_example_01_LDADD = $(ECORE_COMMON_LDADD)

ecore_event_example_02_SOURCES = ecore_event_example_02.c
ecore_event_example_02_LDADD = $(ECORE_COMMON_LDADD)

ecore_exe_example_SOURCES = ecore_exe_example.c
ecore_exe_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_exe_example_child_SOURCES = ecore_exe_example_child.c
ecore_exe_example_child_LDADD = $(ECORE_COMMON_LDADD)

ecore_fd_handler_example_SOURCES = ecore_fd_handler_example.c
ecore_fd_handler_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_idler_example_SOURCES = ecore_idler_example.c
ecore_idler_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_job_example_SOURCES = ecore_job_example.c
ecore_job_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_pipe_simple_example_SOURCES = ecore_pipe_simple_example.c
ecore_pipe_simple_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_poller_example_SOURCES = ecore_poller_example.c
ecore_poller_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_server_bench_SOURCES = ecore_server_bench.c
ecore_server_bench_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_thread_example_SOURCES = ecore_thread_example.c
ecore_thread_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_time_functions_example_SOURCES = ecore_time_functions_example.c
ecore_time_functions_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_timer_example_SOURCES = ecore_timer_example.c
ecore_timer_example_LDADD = $(ECORE_COMMON_LDADD)

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

if HAVE_ECORE_AUDIO
SRCS += \
ecore_audio_custom \
ecore_audio_playback \
ecore_audio_to_ogg
endif

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
