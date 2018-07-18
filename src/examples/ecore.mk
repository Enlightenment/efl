ECORE_COMMON_CPPFLAGS = \
-DPACKAGE_EXAMPLES_DIR=\".\"

if HAVE_ECORE_BUFFER
ECORE_COMMON_CPPFLAGS += -I$(top_srcdir)/src/lib/ecore_buffer
LDADD += $(top_builddir)/src/lib/ecore_buffer/libecore_buffer.la
endif

EXTRA_PROGRAMS += \
ecore/ecore_animator_example \
ecore/ecore_client_bench \
ecore/ecore_compose_get_example \
ecore/ecore_con_client_example \
ecore/ecore_con_client_simple_example \
ecore/ecore_con_lookup_example \
ecore/ecore_con_server_example \
ecore/ecore_con_server_http_example \
ecore/ecore_con_server_simple_example \
ecore/ecore_con_url_cookies_example \
ecore/ecore_con_url_download_example \
ecore/ecore_con_url_headers_example \
ecore/ecore_con_url_ftp_example \
ecore/ecore_evas_basics_example \
ecore/ecore_evas_buffer_example_01 \
ecore/ecore_evas_buffer_example_02 \
ecore/ecore_evas_callbacks \
ecore/ecore_evas_cursor_example \
ecore/ecore_evas_extn_socket_example \
ecore/ecore_evas_extn_plug_example \
ecore/ecore_evas_ews_example \
ecore/ecore_evas_object_example \
ecore/ecore_evas_wayland_multiseat_example \
ecore/ecore_evas_window_sizes_example \
ecore/ecore_evas_vnc_example \
ecore/ecore_event_example_01 \
ecore/ecore_event_example_02 \
ecore/ecore_exe_example \
ecore/ecore_exe_example_child \
ecore/ecore_fd_handler_example \
ecore/ecore_file_download_example \
ecore/ecore_idler_example \
ecore/ecore_imf_example \
ecore/ecore_job_example \
ecore/ecore_poller_example \
ecore/ecore_promise2_example \
ecore/ecore_server_bench \
ecore/ecore_thread_example \
ecore/ecore_time_functions_example \
ecore/ecore_timer_example \
ecore/ecore_getopt_example \
ecore/ecore_con_eet_client_example \
ecore/ecore_con_eet_server_example \
ecore/efl_io_copier_example \
ecore/efl_io_copier_simple_example \
ecore/efl_io_queue_example \
ecore/efl_io_buffered_stream_example \
ecore/efl_net_server_example \
ecore/efl_net_server_simple_example \
ecore/efl_net_dialer_http_example \
ecore/efl_net_dialer_websocket_example \
ecore/efl_net_dialer_websocket_autobahntestee \
ecore/efl_net_dialer_udp_example \
ecore/efl_net_dialer_simple_example \
ecore/efl_net_socket_ssl_dialer_example \
ecore/efl_net_socket_ssl_server_example \
ecore/efl_net_session_example \
ecore/efl_net_control_example \
ecore/efl_net_ip_address_example \
ecore/ecore_ipc_server_example \
ecore/ecore_ipc_client_example


if HAVE_ECORE_AUDIO
#EXTRA_PROGRAMS += \
#ecore_audio_playback \
#ecore_audio_to_ogg \
#ecore_audio_custom
#
#$(top_builddir)/src/lib/ecore_audio/libecore_audio.la \
#$(LDADD)
#
#ecore_audio_custom_SOURCES = ecore/ecore_audio_custom.c
#ecore_audio_custom_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

#
#ecore_audio_playback_SOURCES = ecore/ecore_audio_playback.c
#ecore_audio_playback_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

#
#ecore_audio_to_ogg_SOURCES = ecore/ecore_audio_to_ogg.c
#ecore_audio_to_ogg_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

endif

ecore_ecore_animator_example_SOURCES = ecore/ecore_animator_example.c
ecore_ecore_animator_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


if HAVE_ECORE_BUFFER
EXTRA_PROGRAMS += ecore/ecore_buffer_example \
ecore/ecore_buffer_consumer_example \
ecore/ecore_buffer_provider_example

ecore_ecore_buffer_example_SOURCES = ecore/ecore_buffer_example.c
ecore_ecore_buffer_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_buffer_consumer_example_SOURCES = ecore/ecore_buffer_consumer_example.c
ecore_ecore_buffer_consumer_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_buffer_provider_example_SOURCES = ecore/ecore_buffer_provider_example.c
ecore_ecore_buffer_provider_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

endif

ecore_ecore_client_bench_SOURCES = ecore/ecore_client_bench.c
ecore_ecore_client_bench_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_compose_get_example_SOURCES = ecore/ecore_compose_get_example.c
ecore_ecore_compose_get_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_client_example_SOURCES = ecore/ecore_con_client_example.c
ecore_ecore_con_client_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_client_simple_example_SOURCES = ecore/ecore_con_client_simple_example.c
ecore_ecore_con_client_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_lookup_example_SOURCES = ecore/ecore_con_lookup_example.c
ecore_ecore_con_lookup_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_server_example_SOURCES = ecore/ecore_con_server_example.c
ecore_ecore_con_server_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_server_http_example_SOURCES = ecore/ecore_con_server_http_example.c
ecore_ecore_con_server_http_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_server_simple_example_SOURCES = ecore/ecore_con_server_simple_example.c
ecore_ecore_con_server_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_url_cookies_example_SOURCES = ecore/ecore_con_url_cookies_example.c
ecore_ecore_con_url_cookies_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_url_download_example_SOURCES = ecore/ecore_con_url_download_example.c
ecore_ecore_con_url_download_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_url_headers_example_SOURCES = ecore/ecore_con_url_headers_example.c
ecore_ecore_con_url_headers_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_url_ftp_example_SOURCES = ecore/ecore_con_url_ftp_example.c
ecore_ecore_con_url_ftp_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_basics_example_SOURCES = ecore/ecore_evas_basics_example.c
ecore_ecore_evas_basics_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_buffer_example_01_SOURCES = ecore/ecore_evas_buffer_example_01.c
ecore_ecore_evas_buffer_example_01_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_buffer_example_02_SOURCES = ecore/ecore_evas_buffer_example_02.c
ecore_ecore_evas_buffer_example_02_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_callbacks_SOURCES = ecore/ecore_evas_callbacks.c
ecore_ecore_evas_callbacks_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_cursor_example_SOURCES = ecore/ecore_evas_cursor_example.c
ecore_ecore_evas_cursor_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_extn_socket_example_SOURCES = ecore/ecore_evas_extn_socket_example.c
ecore_ecore_evas_extn_socket_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_extn_plug_example_SOURCES = ecore/ecore_evas_extn_plug_example.c
ecore_ecore_evas_extn_plug_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_ews_example_SOURCES = ecore/ecore_evas_ews_example.c
ecore_ecore_evas_ews_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_object_example_SOURCES = ecore/ecore_evas_object_example.c
ecore_ecore_evas_object_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_wayland_multiseat_example_SOURCES = ecore/ecore_evas_wayland_multiseat_example.c
ecore_ecore_evas_wayland_multiseat_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_window_sizes_example_SOURCES = ecore/ecore_evas_window_sizes_example.c
ecore_ecore_evas_window_sizes_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_evas_vnc_example_SOURCES = ecore/ecore_evas_vnc_example.c
ecore_ecore_evas_vnc_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_event_example_01_SOURCES = ecore/ecore_event_example_01.c
ecore_ecore_event_example_01_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_event_example_02_SOURCES = ecore/ecore_event_example_02.c
ecore_ecore_event_example_02_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_exe_example_SOURCES = ecore/ecore_exe_example.c
ecore_ecore_exe_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_exe_example_child_SOURCES = ecore/ecore_exe_example_child.c
ecore_ecore_exe_example_child_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_fd_handler_example_SOURCES = ecore/ecore_fd_handler_example.c
ecore_ecore_fd_handler_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


if HAVE_CRYPTO_GNUTLS
EXTRA_PROGRAMS += ecore/ecore_fd_handler_gnutls_example
ecore_ecore_fd_handler_gnutls_example_SOURCES = ecore/ecore_fd_handler_gnutls_example.c
ecore_ecore_fd_handler_gnutls_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

endif

ecore_ecore_file_download_example_SOURCES = ecore/ecore_file_download_example.c
ecore_ecore_file_download_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_idler_example_SOURCES = ecore/ecore_idler_example.c
ecore_ecore_idler_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_imf_example_SOURCES = ecore/ecore_imf_example.c
ecore_ecore_imf_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)



ecore_ecore_job_example_SOURCES = ecore/ecore_job_example.c
ecore_ecore_job_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


if HAVE_GSTREAMER
EXTRA_PROGRAMS += ecore/ecore_pipe_gstreamer_example
ecore_ecore_pipe_gstreamer_example_SOURCES = ecore/ecore_pipe_gstreamer_example.c
ecore_ecore_pipe_gstreamer_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS) @GSTREAMER_CFLAGS@
ecore_ecore_pipe_gstreamer_example_LDADD = $(LDADD) @GSTREAMER_LIBS@
endif

if ! HAVE_WINDOWS
EXTRA_PROGRAMS += ecore/ecore_pipe_simple_example
ecore_ecore_pipe_simple_example_SOURCES = ecore/ecore_pipe_simple_example.c
ecore_ecore_pipe_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

endif

ecore_ecore_poller_example_SOURCES = ecore/ecore_poller_example.c
ecore_ecore_poller_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_promise2_example_SOURCES = ecore/ecore_promise2_example.c
ecore_ecore_promise2_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_server_bench_SOURCES = ecore/ecore_server_bench.c
ecore_ecore_server_bench_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_thread_example_SOURCES = ecore/ecore_thread_example.c
ecore_ecore_thread_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_time_functions_example_SOURCES = ecore/ecore_time_functions_example.c
ecore_ecore_time_functions_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_timer_example_SOURCES = ecore/ecore_timer_example.c
ecore_ecore_timer_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_getopt_example_SOURCES = ecore/ecore_getopt_example.c
ecore_ecore_getopt_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_con_eet_client_example_SOURCES = ecore/ecore_con_eet_client_example.c \
				       ecore/ecore_con_eet_descriptor_example.c \
				       ecore/ecore_con_eet_descriptor_example.h
ecore_ecore_con_eet_client_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

ecore_ecore_con_eet_server_example_SOURCES = ecore/ecore_con_eet_server_example.c \
				       ecore/ecore_con_eet_descriptor_example.c \
				       ecore/ecore_con_eet_descriptor_example.h
ecore_ecore_con_eet_server_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_io_copier_example_SOURCES = ecore/efl_io_copier_example.c
ecore_efl_io_copier_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_io_copier_simple_example_SOURCES = ecore/efl_io_copier_simple_example.c
ecore_efl_io_copier_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_io_queue_example_SOURCES = ecore/efl_io_queue_example.c
ecore_efl_io_queue_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_io_buffered_stream_example_SOURCES = ecore/efl_io_buffered_stream_example.c
ecore_efl_io_buffered_stream_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_server_example_SOURCES = ecore/efl_net_server_example.c
ecore_efl_net_server_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_server_simple_example_SOURCES = ecore/efl_net_server_simple_example.c
ecore_efl_net_server_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_dialer_http_example_SOURCES = ecore/efl_net_dialer_http_example.c
ecore_efl_net_dialer_http_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_dialer_websocket_example_SOURCES = ecore/efl_net_dialer_websocket_example.c
ecore_efl_net_dialer_websocket_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_dialer_websocket_autobahntestee_SOURCES = ecore/efl_net_dialer_websocket_autobahntestee.c
ecore_efl_net_dialer_websocket_autobahntestee_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_dialer_udp_example_SOURCES = ecore/efl_net_dialer_udp_example.c
ecore_efl_net_dialer_udp_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_dialer_simple_example_SOURCES = ecore/efl_net_dialer_simple_example.c
ecore_efl_net_dialer_simple_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


if HAVE_WINDOWS
EXTRA_PROGRAMS += ecore/efl_net_dialer_windows_example
ecore_efl_net_dialer_windows_example_SOURCES = ecore/efl_net_dialer_windows_example.c
ecore_efl_net_dialer_windows_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

else
EXTRA_PROGRAMS += ecore/efl_net_dialer_unix_example
ecore_efl_net_dialer_unix_example_SOURCES = ecore/efl_net_dialer_unix_example.c
ecore_efl_net_dialer_unix_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)

endif

ecore_efl_net_socket_ssl_dialer_example_SOURCES = ecore/efl_net_socket_ssl_dialer_example.c
ecore_efl_net_socket_ssl_dialer_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_socket_ssl_server_example_SOURCES = ecore/efl_net_socket_ssl_server_example.c
ecore_efl_net_socket_ssl_server_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_session_example_SOURCES = ecore/efl_net_session_example.c
ecore_efl_net_session_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_control_example_SOURCES = ecore/efl_net_control_example.c
ecore_efl_net_control_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_efl_net_ip_address_example_SOURCES = ecore/efl_net_ip_address_example.c
ecore_efl_net_ip_address_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_ipc_server_example_SOURCES = ecore/ecore_ipc_server_example.c
ecore_ecore_ipc_server_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ecore_ecore_ipc_client_example_SOURCES = ecore/ecore_ipc_client_example.c
ecore_ecore_ipc_client_example_CPPFLAGS = $(ECORE_COMMON_CPPFLAGS) $(AM_CPPFLAGS)


ECORE_SRCS = \
ecore/ecore_animator_example.c \
ecore/ecore_buffer_example.c \
ecore/ecore_buffer_consumer_example.c \
ecore/ecore_buffer_provider_example.c \
ecore/ecore_client_bench.c \
ecore/ecore_compose_get_example.c \
ecore/ecore_con_client_example.c \
ecore/ecore_con_client_simple_example.c \
ecore/ecore_con_lookup_example.c \
ecore/ecore_con_server_example.c \
ecore/ecore_con_server_http_example.c \
ecore/ecore_con_server_simple_example.c \
ecore/ecore_con_url_cookies_example.c \
ecore/ecore_con_url_download_example.c \
ecore/ecore_con_url_headers_example.c \
ecore/ecore_con_url_ftp_example.c \
ecore/ecore_evas_basics_example.c \
ecore/ecore_evas_buffer_example_01.c \
ecore/ecore_evas_buffer_example_02.c \
ecore/ecore_evas_extn_socket_example.c \
ecore/ecore_evas_extn_plug_example.c \
ecore/ecore_evas_callbacks.c \
ecore/ecore_evas_ews_example.c \
ecore/ecore_evas_object_example.c \
ecore/ecore_evas_wayland_multiseat_example.c \
ecore/ecore_evas_window_sizes_example.c \
ecore/ecore_evas_vnc_example.c \
ecore/ecore_event_example_01.c \
ecore/ecore_event_example_02.c \
ecore/ecore_exe_example.c \
ecore/ecore_exe_example_child.c \
ecore/ecore_fd_handler_example.c \
ecore/ecore_fd_handler_gnutls_example.c \
ecore/ecore_file_download_example.c \
ecore/ecore_idler_example.c \
ecore/ecore_imf_example.c \
ecore/ecore_job_example.c \
ecore/ecore_pipe_gstreamer_example.c \
ecore/ecore_pipe_simple_example.c \
ecore/ecore_poller_example.c \
ecore/ecore_promise2_example.c \
ecore/ecore_server_bench.c \
ecore/ecore_thread_example.c \
ecore/ecore_time_functions_example.c \
ecore/ecore_timer_example.c \
ecore/ecore_getopt_example.c \
ecore/ecore_con_eet_client_example.c \
ecore/ecore_con_eet_server_example.c \
ecore/ecore_con_eet_descriptor_example.c \
ecore/efl_io_copier_example.c \
ecore/efl_io_copier_simple_example.c \
ecore/efl_io_queue_example.c \
ecore/efl_io_buffered_stream_example.c \
ecore/efl_net_server_example.c \
ecore/efl_net_server_simple_example.c \
ecore/efl_net_dialer_http_example.c \
ecore/efl_net_dialer_websocket_example.c \
ecore/efl_net_dialer_websocket_autobahntestee.c \
ecore/efl_net_dialer_udp_example.c \
ecore/efl_net_dialer_simple_example.c \
ecore/efl_net_socket_ssl_dialer_example.c \
ecore/efl_net_socket_ssl_server_example.c \
ecore/efl_net_session_example.c \
ecore/efl_net_control_example.c \
ecore/ecore_ipc_server_example.c \
ecore/ecore_ipc_client_example.c

ECORE_DATA_FILES = \
ecore/red.png \
ecore/Makefile.examples

DATA_FILES += $(ECORE_DATA_FILES)

install-examples-ecore:
	$(MKDIR_P) $(DESTDIR)$(datadir)/ecore/examples
	cd $(srcdir) && $(install_sh_DATA) -c $(ECORE_SRCS) $(ECORE_DATA_FILES) $(DESTDIR)$(datadir)/ecore/examples

uninstall-local-ecore:
	for f in $(ECORE_SRCS) $(ECORE_DATA_FILES); do \
	  rm -f $(DESTDIR)$(datadir)/ecore/examples/$$f ; \
	done

INSTALL_EXAMPLES += install-examples-ecore
UNINSTALL_EXAMPLES += uninstall-local-ecore
