MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/eina \
-I$(top_srcdir)/src/lib/eet \
-I$(top_srcdir)/src/lib/emile \
-I$(top_srcdir)/src/lib/eo \
-I$(top_srcdir)/src/lib/efl \
-I$(top_srcdir)/src/lib/evas \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_srcdir)/src/lib/ecore_input_evas \
-I$(top_srcdir)/src/lib/ecore_file \
-I$(top_srcdir)/src/lib/ecore_imf \
-I$(top_srcdir)/src/lib/ecore_imf_evas \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_audio \
-I$(top_builddir)/src/lib/eina \
-I$(top_builddir)/src/lib/eet \
-I$(top_builddir)/src/lib/emile \
-I$(top_builddir)/src/lib/eo \
-I$(top_builddir)/src/lib/efl \
-I$(top_builddir)/src/lib/evas \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_file \
-I$(top_builddir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore_input_evas \
-I$(top_builddir)/src/lib/ecore_imf \
-I$(top_builddir)/src/lib/ecore_imf_evas \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_audio \
-DPACKAGE_EXAMPLES_DIR=\".\" \
@ECORE_CFLAGS@

if HAVE_ECORE_BUFFER
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/ecore_buffer
endif

EXTRA_PROGRAMS = \
ecore_animator_example \
ecore_client_bench \
ecore_compose_get_example \
ecore_con_client_example \
ecore_con_client_simple_example \
ecore_con_lookup_example \
ecore_con_server_example \
ecore_con_server_http_example \
ecore_con_server_simple_example \
ecore_con_url_cookies_example \
ecore_con_url_download_example \
ecore_con_url_headers_example \
ecore_con_url_ftp_example \
ecore_evas_basics_example \
ecore_evas_buffer_example_01 \
ecore_evas_buffer_example_02 \
ecore_evas_callbacks \
ecore_evas_cursor_example \
ecore_evas_extn_socket_example \
ecore_evas_extn_plug_example \
ecore_evas_ews_example \
ecore_evas_object_example \
ecore_evas_wayland_multiseat_example \
ecore_evas_window_sizes_example \
ecore_evas_vnc_example \
ecore_event_example_01 \
ecore_event_example_02 \
ecore_exe_example \
ecore_exe_example_child \
ecore_fd_handler_example \
ecore_file_download_example \
ecore_idler_example \
ecore_imf_example \
ecore_job_example \
ecore_poller_example \
ecore_promise2_example \
ecore_server_bench \
ecore_thread_example \
ecore_time_functions_example \
ecore_timer_example \
ecore_getopt_example \
ecore_con_eet_client_example \
ecore_con_eet_server_example \
efl_io_copier_example \
efl_io_copier_simple_example \
efl_io_queue_example \
efl_io_buffered_stream_example \
efl_net_server_example \
efl_net_server_simple_example \
efl_net_dialer_http_example \
efl_net_dialer_websocket_example \
efl_net_dialer_websocket_autobahntestee \
efl_net_dialer_udp_example \
efl_net_dialer_simple_example \
efl_net_socket_ssl_dialer_example \
efl_net_socket_ssl_server_example \
efl_net_session_example \
efl_net_control_example \
efl_net_ip_address_example \
ecore_ipc_server_example \
ecore_ipc_client_example


ECORE_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(top_builddir)/src/lib/eo/libeo.la \
$(top_builddir)/src/lib/eina/libeina.la \
$(top_builddir)/src/lib/emile/libemile.la \
@ECORE_LDFLAGS@

if HAVE_ECORE_AUDIO
#EXTRA_PROGRAMS += \
#ecore_audio_playback \
#ecore_audio_to_ogg \
#ecore_audio_custom
#
#ECORE_AUDIO_COMMON_LDADD = \
#$(top_builddir)/src/lib/ecore_audio/libecore_audio.la \
#$(ECORE_COMMON_LDADD)
#
#ecore_audio_custom_SOURCES = ecore_audio_custom.c
#ecore_audio_custom_LDADD = $(ECORE_AUDIO_COMMON_LDADD)
#
#ecore_audio_playback_SOURCES = ecore_audio_playback.c
#ecore_audio_playback_LDADD = $(ECORE_AUDIO_COMMON_LDADD)
#
#ecore_audio_to_ogg_SOURCES = ecore_audio_to_ogg.c
#ecore_audio_to_ogg_LDADD = $(ECORE_AUDIO_COMMON_LDADD)
endif

ECORE_EVAS_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_evas/libecore_evas.la \
$(top_builddir)/src/lib/evas/libevas.la \
$(ECORE_COMMON_LDADD)

ECORE_CON_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_con/libecore_con.la \
$(ECORE_COMMON_LDADD)

ECORE_IPC_COMMON_LDADD = \
$(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la \
$(ECORE_CON_COMMON_LDADD)

ecore_animator_example_SOURCES = ecore_animator_example.c
ecore_animator_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

if HAVE_ECORE_BUFFER
EXTRA_PROGRAMS += ecore_buffer_example \
ecore_buffer_consumer_example \
ecore_buffer_provider_example

ecore_buffer_example_SOURCES = ecore_buffer_example.c
ecore_buffer_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
                             $(top_builddir)/src/lib/ecore_buffer/libecore_buffer.la

ecore_buffer_consumer_example_SOURCES = ecore_buffer_consumer_example.c
ecore_buffer_consumer_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
                             $(top_builddir)/src/lib/ecore_buffer/libecore_buffer.la

ecore_buffer_provider_example_SOURCES = ecore_buffer_provider_example.c
ecore_buffer_provider_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
                             $(top_builddir)/src/lib/ecore_buffer/libecore_buffer.la
endif

ecore_client_bench_SOURCES = ecore_client_bench.c
ecore_client_bench_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_compose_get_example_SOURCES = ecore_compose_get_example.c
ecore_compose_get_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
$(top_builddir)/src/lib/ecore_input/libecore_input.la

ecore_con_client_example_SOURCES = ecore_con_client_example.c
ecore_con_client_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_client_simple_example_SOURCES = ecore_con_client_simple_example.c
ecore_con_client_simple_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_lookup_example_SOURCES = ecore_con_lookup_example.c
ecore_con_lookup_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_con_server_example_SOURCES = ecore_con_server_example.c
ecore_con_server_example_LDADD = $(ECORE_CON_COMMON_LDADD)

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

ecore_con_url_ftp_example_SOURCES = ecore_con_url_ftp_example.c
ecore_con_url_ftp_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_evas_basics_example_SOURCES = ecore_evas_basics_example.c
ecore_evas_basics_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_buffer_example_01_SOURCES = ecore_evas_buffer_example_01.c
ecore_evas_buffer_example_01_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_buffer_example_02_SOURCES = ecore_evas_buffer_example_02.c
ecore_evas_buffer_example_02_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_callbacks_SOURCES = ecore_evas_callbacks.c
ecore_evas_callbacks_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_cursor_example_SOURCES = ecore_evas_cursor_example.c
ecore_evas_cursor_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_extn_socket_example_SOURCES = ecore_evas_extn_socket_example.c
ecore_evas_extn_socket_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_extn_plug_example_SOURCES = ecore_evas_extn_plug_example.c
ecore_evas_extn_plug_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_ews_example_SOURCES = ecore_evas_ews_example.c
ecore_evas_ews_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_object_example_SOURCES = ecore_evas_object_example.c
ecore_evas_object_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_wayland_multiseat_example_SOURCES = ecore_evas_wayland_multiseat_example.c
ecore_evas_wayland_multiseat_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
$(top_builddir)/src/lib/ecore_input/libecore_input.la

ecore_evas_window_sizes_example_SOURCES = ecore_evas_window_sizes_example.c
ecore_evas_window_sizes_example_LDADD = $(ECORE_EVAS_COMMON_LDADD)

ecore_evas_vnc_example_SOURCES = ecore_evas_vnc_example.c
ecore_evas_vnc_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
$(top_builddir)/src/lib/ecore_input/libecore_input.la

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

if HAVE_CRYPTO_GNUTLS
EXTRA_PROGRAMS += ecore_fd_handler_gnutls_example
ecore_fd_handler_gnutls_example_SOURCES = ecore_fd_handler_gnutls_example.c
ecore_fd_handler_gnutls_example_LDADD = $(ECORE_CON_COMMON_LDADD)
endif

ecore_file_download_example_SOURCES = ecore_file_download_example.c
ecore_file_download_example_LDADD = $(ECORE_COMMON_LDADD) \
$(top_builddir)/src/lib/ecore_file/libecore_file.la

ecore_idler_example_SOURCES = ecore_idler_example.c
ecore_idler_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_imf_example_SOURCES = ecore_imf_example.c
ecore_imf_example_LDADD = $(ECORE_EVAS_COMMON_LDADD) \
$(top_builddir)/src/lib/ecore_imf/libecore_imf.la \
$(top_builddir)/src/lib/ecore_imf_evas/libecore_imf_evas.la


ecore_job_example_SOURCES = ecore_job_example.c
ecore_job_example_LDADD = $(ECORE_COMMON_LDADD)

if HAVE_GSTREAMER
EXTRA_PROGRAMS += ecore_pipe_gstreamer_example
ecore_pipe_gstreamer_example_SOURCES = ecore_pipe_gstreamer_example.c
ecore_pipe_gstreamer_example_LDADD = $(ECORE_COMMON_LDADD) @GSTREAMER_LIBS@
ecore_pipe_gstreamer_example_CFLAGS = @GSTREAMER_CFLAGS@
endif

if ! HAVE_WINDOWS
EXTRA_PROGRAMS += ecore_pipe_simple_example
ecore_pipe_simple_example_SOURCES = ecore_pipe_simple_example.c
ecore_pipe_simple_example_LDADD = $(ECORE_COMMON_LDADD)
endif

ecore_poller_example_SOURCES = ecore_poller_example.c
ecore_poller_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_promise2_example_SOURCES = ecore_promise2_example.c
ecore_promise2_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_server_bench_SOURCES = ecore_server_bench.c
ecore_server_bench_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_thread_example_SOURCES = ecore_thread_example.c
ecore_thread_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_time_functions_example_SOURCES = ecore_time_functions_example.c
ecore_time_functions_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_timer_example_SOURCES = ecore_timer_example.c
ecore_timer_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_getopt_example_SOURCES = ecore_getopt_example.c
ecore_getopt_example_LDADD = $(ECORE_COMMON_LDADD)

ecore_con_eet_client_example_SOURCES = ecore_con_eet_client_example.c \
				       ecore_con_eet_descriptor_example.c \
				       ecore_con_eet_descriptor_example.h
ecore_con_eet_client_example_LDADD = $(ECORE_CON_COMMON_LDADD) \
$(top_builddir)/src/lib/eet/libeet.la

ecore_con_eet_server_example_SOURCES = ecore_con_eet_server_example.c \
				       ecore_con_eet_descriptor_example.c \
				       ecore_con_eet_descriptor_example.h
ecore_con_eet_server_example_LDADD = $(ECORE_CON_COMMON_LDADD) \
$(top_builddir)/src/lib/eet/libeet.la


efl_io_copier_example_SOURCES = efl_io_copier_example.c
efl_io_copier_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_io_copier_simple_example_SOURCES = efl_io_copier_simple_example.c
efl_io_copier_simple_example_LDADD = $(ECORE_COMMON_LDADD)

efl_io_queue_example_SOURCES = efl_io_queue_example.c
efl_io_queue_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_io_buffered_stream_example_SOURCES = efl_io_buffered_stream_example.c
efl_io_buffered_stream_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_server_example_SOURCES = efl_net_server_example.c
efl_net_server_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_server_simple_example_SOURCES = efl_net_server_simple_example.c
efl_net_server_simple_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_dialer_http_example_SOURCES = efl_net_dialer_http_example.c
efl_net_dialer_http_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_dialer_websocket_example_SOURCES = efl_net_dialer_websocket_example.c
efl_net_dialer_websocket_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_dialer_websocket_autobahntestee_SOURCES = efl_net_dialer_websocket_autobahntestee.c
efl_net_dialer_websocket_autobahntestee_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_dialer_udp_example_SOURCES = efl_net_dialer_udp_example.c
efl_net_dialer_udp_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_dialer_simple_example_SOURCES = efl_net_dialer_simple_example.c
efl_net_dialer_simple_example_LDADD = $(ECORE_CON_COMMON_LDADD)

if HAVE_WINDOWS
EXTRA_PROGRAMS += efl_net_dialer_windows_example
efl_net_dialer_windows_example_SOURCES = efl_net_dialer_windows_example.c
efl_net_dialer_windows_example_LDADD = $(ECORE_CON_COMMON_LDADD)
else
EXTRA_PROGRAMS += efl_net_dialer_unix_example
efl_net_dialer_unix_example_SOURCES = efl_net_dialer_unix_example.c
efl_net_dialer_unix_example_LDADD = $(ECORE_CON_COMMON_LDADD)
endif

efl_net_socket_ssl_dialer_example_SOURCES = efl_net_socket_ssl_dialer_example.c
efl_net_socket_ssl_dialer_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_socket_ssl_server_example_SOURCES = efl_net_socket_ssl_server_example.c
efl_net_socket_ssl_server_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_session_example_SOURCES = efl_net_session_example.c
efl_net_session_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_control_example_SOURCES = efl_net_control_example.c
efl_net_control_example_LDADD = $(ECORE_CON_COMMON_LDADD)

efl_net_ip_address_example_SOURCES = efl_net_ip_address_example.c
efl_net_ip_address_example_LDADD = $(ECORE_CON_COMMON_LDADD)

ecore_ipc_server_example_SOURCES = ecore_ipc_server_example.c
ecore_ipc_server_example_LDADD = $(ECORE_IPC_COMMON_LDADD)

ecore_ipc_client_example_SOURCES = ecore_ipc_client_example.c
ecore_ipc_client_example_LDADD = $(ECORE_IPC_COMMON_LDADD)

SRCS = \
ecore_animator_example.c \
ecore_buffer_example.c \
ecore_buffer_consumer_example.c \
ecore_buffer_provider_example.c \
ecore_client_bench.c \
ecore_compose_get_example.c \
ecore_con_client_example.c \
ecore_con_client_simple_example.c \
ecore_con_lookup_example.c \
ecore_con_server_example.c \
ecore_con_server_http_example.c \
ecore_con_server_simple_example.c \
ecore_con_url_cookies_example.c \
ecore_con_url_download_example.c \
ecore_con_url_headers_example.c \
ecore_con_url_ftp_example.c \
ecore_evas_basics_example.c \
ecore_evas_buffer_example_01.c \
ecore_evas_buffer_example_02.c \
ecore_evas_extn_socket_example.c \
ecore_evas_extn_plug_example.c \
ecore_evas_callbacks.c \
ecore_evas_ews_example.c \
ecore_evas_object_example.c \
ecore_evas_wayland_multiseat_example.c \
ecore_evas_window_sizes_example.c \
ecore_evas_vnc_example.c \
ecore_event_example_01.c \
ecore_event_example_02.c \
ecore_exe_example.c \
ecore_exe_example_child.c \
ecore_fd_handler_example.c \
ecore_fd_handler_gnutls_example.c \
ecore_file_download_example.c \
ecore_idler_example.c \
ecore_imf_example.c \
ecore_job_example.c \
ecore_pipe_gstreamer_example.c \
ecore_pipe_simple_example.c \
ecore_poller_example.c \
ecore_promise2_example.c \
ecore_server_bench.c \
ecore_thread_example.c \
ecore_time_functions_example.c \
ecore_timer_example.c \
ecore_getopt_example.c \
ecore_con_eet_client_example.c \
ecore_con_eet_server_example.c \
ecore_con_eet_descriptor_example.c \
efl_io_copier_example.c \
efl_io_copier_simple_example.c \
efl_io_queue_example.c \
efl_io_buffered_stream_example.c \
efl_net_server_example.c \
efl_net_server_simple_example.c \
efl_net_dialer_http_example.c \
efl_net_dialer_websocket_example.c \
efl_net_dialer_websocket_autobahntestee.c \
efl_net_dialer_udp_example.c \
efl_net_dialer_simple_example.c \
efl_net_socket_ssl_dialer_example.c \
efl_net_socket_ssl_server_example.c \
efl_net_session_example.c \
efl_net_control_example.c \
ecore_ipc_server_example.c \
ecore_ipc_client_example.c

%.pem:
	echo -e "US\nOR\nPortland\nXPTO Ltd\n\nlocalhost\nroot@localhost\n" | openssl req -new -x509 -days 30 -nodes -out $@ -keyout $@

DATA_FILES = red.png Makefile.examples

EXTRA_DIST = $(DATA_FILES)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)

install-examples:
	$(MKDIR_P) $(DESTDIR)$(datadir)/ecore/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(SRCS) $(DATA_FILES) $(DESTDIR)$(datadir)/ecore/examples

uninstall-local:
	for f in $(SRCS) $(DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/ecore/examples/$$f ; \
	done

if ALWAYS_BUILD_EXAMPLES
noinst_PROGRAMS = $(EXTRA_PROGRAMS)
endif
