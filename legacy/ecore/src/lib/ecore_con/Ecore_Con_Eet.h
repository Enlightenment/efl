#ifndef _ECORE_CON_EET
# define _ECORE_CON_EET

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_Con.h>

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

typedef struct _Ecore_Con_Eet Ecore_Con_Eet;
typedef struct _Ecore_Con_Reply Ecore_Con_Reply;

typedef void (*Ecore_Con_Eet_Data_Cb)(void *data, Ecore_Con_Reply *reply, const char *protocol_name, void *value);
typedef void (*Ecore_Con_Eet_Raw_Data_Cb)(void *data, Ecore_Con_Reply *reply, const char *protocol_name, const char *section, void *value, size_t length);
typedef Eina_Bool (*Ecore_Con_Eet_Client_Cb)(void *data, Ecore_Con_Reply *reply, Ecore_Con_Client *conn);
typedef Eina_Bool (*Ecore_Con_Eet_Server_Cb)(void *data, Ecore_Con_Reply *reply, Ecore_Con_Server *conn);

EAPI Ecore_Con_Eet *ecore_con_eet_server_new(Ecore_Con_Server *server);
EAPI Ecore_Con_Eet *ecore_con_eet_client_new(Ecore_Con_Server *server);
EAPI void ecore_con_eet_server_free(Ecore_Con_Eet *ece);

EAPI void ecore_con_eet_register(Ecore_Con_Eet *ece, const char *name, Eet_Data_Descriptor *edd);

EAPI void ecore_con_eet_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data);
EAPI void ecore_con_eet_data_callback_del(Ecore_Con_Eet *ece, const char *name);

EAPI void ecore_con_eet_raw_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data);
EAPI void ecore_con_eet_raw_data_callback_del(Ecore_Con_Eet *ece, const char *name);

EAPI void ecore_con_eet_client_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data);
EAPI void ecore_con_eet_client_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data);

EAPI void ecore_con_eet_client_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data);
EAPI void ecore_con_eet_client_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data);

EAPI void ecore_con_eet_server_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data);
EAPI void ecore_con_eet_server_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data);

EAPI void ecore_con_eet_server_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data);
EAPI void ecore_con_eet_server_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data);

EAPI void ecore_con_eet_data_set(Ecore_Con_Eet *ece, const void *data);
EAPI void *ecore_con_eet_data_get(Ecore_Con_Eet *ece);

EAPI Ecore_Con_Eet *ecore_con_eet_reply(Ecore_Con_Reply *reply);
EAPI void ecore_con_eet_send(Ecore_Con_Reply *reply, const char *protocol_name, void *value);
EAPI void ecore_con_eet_raw_send(Ecore_Con_Reply *reply, const char *protocol_name, const char *section, void *value, unsigned int length);

#endif
