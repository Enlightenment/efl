#ifndef _ECORE_CON_SSH_H
#define _ECORE_CON_SSH_H

#include <time.h>
#include <libgen.h>
#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <netdb.h>
#endif
#include <Eina.h>
#include <Eo.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_CON_BUILD
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

typedef struct _Efl_Network_Ssh_Request Efl_Network_Ssh_Request;
typedef Eo Efl_Network_Ssh_Channel_Interactive_Session;

#ifndef EFL_NOLEGACY_API_SUPPORT
# include "efl_network_ssh_client.eo.legacy.h"
# include "efl_network_ssh_channel_base.eo.legacy.h"
# include "efl_network_ssh_channel_interactive_session.eo.legacy.h"
#endif
#ifdef EFL_EO_API_SUPPORT
# include "efl_network_ssh_client.eo.h"
# include "efl_network_ssh_channel_base.eo.h"
# include "efl_network_ssh_channel_interactive_session.eo.h"
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
