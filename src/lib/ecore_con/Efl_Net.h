#ifndef _EFL_NET_H
#define _EFL_NET_H

#include <time.h>
#include <libgen.h>
#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <netdb.h>
#endif

#define __EFL_NET_IS_REQUIRED

#include <Efl_Core.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 *
 * @note This function already calls ecore_init() internally, so you don't need
 * to call it explicitly.
 */
EAPI int               ecore_con_init(void);

/**
 * @brief Shuts down the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @note This function already calls ecore_shutdown() internally, so you don't
 * need to call it explicitly unless you called ecore_init() explicitly too.
 */
EAPI int               ecore_con_shutdown(void);

/**
 * @brief Initializes the Ecore_Con_Url library.
 * @return Number of times the library has been initialised without being
 *          shut down.
 *
 * @note This function doesn't call ecore_con_init(). You still need to call it
 * explicitly before calling this one.
 */
EAPI int               ecore_con_url_init(void);

/**
 * @brief Shuts down the Ecore_Con_Url library.
 * @return  Number of calls that still uses Ecore_Con_Url
 *
 * @note This function doesn't call ecore_con_shutdown(). You still need to call
 * it explicitly after calling this one.
 */
EAPI int               ecore_con_url_shutdown(void);

#include "efl_net_types.eot.h"

#include "efl_net_ip_address.eo.h"

#include "efl_net_socket.eo.h"
#include "efl_net_dialer.eo.h"
#include "efl_net_server.eo.h"

#include "efl_net_socket_simple.eo.h"
#include "efl_net_dialer_simple.eo.h"
#include "efl_net_server_simple.eo.h"

#include "efl_net_socket_fd.eo.h"
#include "efl_net_server_fd.eo.h"

#include "efl_net_server_ip.eo.h"

#include "efl_net_socket_tcp.eo.h"
#include "efl_net_dialer_tcp.eo.h"
#include "efl_net_server_tcp.eo.h"

#ifdef _WIN32
#include "efl_net_socket_windows.eo.h"
#include "efl_net_dialer_windows.eo.h"
#include "efl_net_server_windows.eo.h"
#else
#include "efl_net_socket_unix.eo.h"
#include "efl_net_dialer_unix.eo.h"
#include "efl_net_server_unix.eo.h"
#endif

#include "efl_net_socket_udp.eo.h"
#include "efl_net_dialer_udp.eo.h"
#include "efl_net_server_udp.eo.h"
#include "efl_net_server_udp_client.eo.h"

#include "efl_net_http_types.eot.h"

#include "efl_net_dialer_http.eo.h"
#include "efl_net_dialer_websocket.eo.h"

#include "efl_net_ssl_types.eot.h"

#include "efl_net_ssl_context.eo.h"
#include "efl_net_socket_ssl.eo.h"
#include "efl_net_dialer_ssl.eo.h"
#include "efl_net_server_ssl.eo.h"

#include "efl_net_control_technology.eo.h"
#include "efl_net_control_access_point.eo.h"
#include "efl_net_control.eo.h"
#include "efl_net_session.eo.h"

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
