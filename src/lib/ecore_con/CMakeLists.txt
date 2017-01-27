set(DESCRIPTION "E core library, Con module")

set(LIBRARIES
  ecore
  eet
  efl
  eina
  emile
  eo
  m
)

set(PUBLIC_EO_FILES
  ecore_con_eet_base.eo
  ecore_con_eet_client_obj.eo
  ecore_con_eet_server_obj.eo
  efl_net_control.eo
  efl_net_control_access_point.eo
  efl_net_control_technology.eo
  efl_net_dialer.eo
  efl_net_dialer_http.eo
  efl_net_dialer_simple.eo
  efl_net_dialer_ssl.eo
  efl_net_dialer_tcp.eo
  efl_net_dialer_udp.eo
  efl_net_dialer_websocket.eo
  efl_net_http_types.eot
  efl_net_ip_address.eo
  efl_net_server.eo
  efl_net_server_fd.eo
  efl_net_server_simple.eo
  efl_net_server_ssl.eo
  efl_net_server_tcp.eo
  efl_net_server_udp.eo
  efl_net_server_udp_client.eo
  efl_net_session.eo
  efl_net_socket.eo
  efl_net_socket_fd.eo
  efl_net_socket_simple.eo
  efl_net_socket_ssl.eo
  efl_net_socket_tcp.eo
  efl_net_socket_udp.eo
  efl_net_ssl_context.eo
  efl_net_ssl_types.eot
  efl_net_types.eot
)

set(PUBLIC_HEADERS
  Ecore_Con.h
  Ecore_Con_Eet.h
  Ecore_Con_Eet_Eo.h
  Ecore_Con_Eet_Legacy.h
  Ecore_Con_Eo.h
)

set(SOURCES
  ecore_con.c
  ecore_con_alloc.c
  ecore_con_eet.c
  ecore_con_legacy.c
  ecore_con_local.c
  ecore_con_private.h
  ecore_con_proxy_helper.c
  ecore_con_socks.c
  ecore_con_url.c
  ecore_con_url_curl.c
  ecore_con_url_curl.h
  efl_net_dialer.c
  efl_net_dialer_http.c
  efl_net_dialer_simple.c
  efl_net_dialer_ssl.c
  efl_net_dialer_tcp.c
  efl_net_dialer_udp.c
  efl_net_dialer_websocket.c
  efl_net_ip_address.c
  efl_net_server.c
  efl_net_server_fd.c
  efl_net_server_simple.c
  efl_net_server_ssl.c
  efl_net_server_tcp.c
  efl_net_server_udp.c
  efl_net_server_udp_client.c
  efl_net_socket.c
  efl_net_socket_fd.c
  efl_net_socket_simple.c
  efl_net_socket_ssl.c
  efl_net_socket_tcp.c
  efl_net_socket_udp.c
  efl_net_ssl_context.c
)

if(HAVE_SYS_UN_H)
  list(APPEND PUBLIC_EO_FILES
    efl_net_dialer_unix.eo
    efl_net_server_unix.eo
    efl_net_socket_unix.eo
  )

  list(APPEND SOURCES
    efl_net_dialer_unix.c
    efl_net_server_unix.c
    efl_net_socket_unix.c
  )
endif()

if(WITH_CRYPTO STREQUAL "gnutls")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE gnutls)
elseif(WITH_CRYPTO STREQUAL "openssl")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE openssl)
endif()

if(ENABLE_SYSTEMD)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE libsystemd)
endif()

if(WITH_EFL_NET_CONTROL_BACKEND STREQUAL "connman")
  list(APPEND SOURCES
    efl_net-connman.c
    efl_net-connman.h
    efl_net_control-connman.c
    efl_net_control_access_point-connman.c
    efl_net_control_technology-connman.c
    efl_net_session-connman.c
  )
  list(APPEND LIBRARIES eldbus)
else()
  list(APPEND SOURCES
    efl_net_control-none.c
    efl_net_control_access_point-none.c
    efl_net_control_technology-none.c
    efl_net_session-none.c
  )
endif()
