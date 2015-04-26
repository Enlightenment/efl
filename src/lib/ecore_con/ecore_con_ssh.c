#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Eo.h>
#include <Ecore_Con.h>
#include <Ecore_Con_Ssh.h>

typedef struct _Efl_Network_Ssh_Client_Data Efl_Network_Ssh_Client_Data;
struct _Efl_Network_Ssh_Client_Data
{
};

typedef struct _Efl_Network_Ssh_Channel_Base_Data Efl_Network_Ssh_Channel_Base_Data;
struct _Efl_Network_Ssh_Channel_Base_Data
{
};

typedef struct _Efl_Network_Ssh_Channel_Interactive_Session_Data Efl_Network_Ssh_Channel_Interactive_Session_Data;
struct _Efl_Network_Ssh_Channel_Interactive_Session_Data
{
};



static void
_efl_network_ssh_client_user_set(Eo *obj, Efl_Network_Ssh_Client_Data *pd, const char *user)
{
}

static const char *
_efl_network_ssh_client_user_get(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static void
_efl_network_ssh_client_passwd_set(Eo *obj, Efl_Network_Ssh_Client_Data *pd, const char *passwd)
{
}

static const char *
_efl_network_ssh_client_passwd_get(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static Eina_Iterator *
_efl_network_ssh_client_keys_list(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static Eina_Bool
_efl_network_ssh_client_key_add(Eo *obj, Efl_Network_Ssh_Client_Data *pd, const char *path)
{
}

static Eina_Bool
_efl_network_ssh_client_key_del(Eo *obj, Efl_Network_Ssh_Client_Data *pd, const char *path)
{
}

static Eina_Bool
_efl_network_ssh_client_hostkey(Eo *obj, Efl_Network_Ssh_Client_Data *pd, Eina_Bool validate)
{
}

static Efl_Network_Ssh_Channel_Interactive_Session *
_efl_network_ssh_client_interactive_session_new(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static void
_efl_network_ssh_client_eo_base_constructor(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static Eo *
_efl_network_ssh_client_eo_base_finalize(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static void
_efl_network_ssh_client_eo_base_destructor(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static int
_efl_network_ssh_client_ecore_con_base_send(Eo *obj, Efl_Network_Ssh_Client_Data *pd, const void *data, int size)
{
}

static void
_efl_network_ssh_client_ecore_con_base_flush(Eo *obj, Efl_Network_Ssh_Client_Data *pd)
{
}

static Eina_Bool
_efl_network_ssh_channel_base_data_send(Eo *obj, Efl_Network_Ssh_Channel_Base_Data *pd, const void *data, size_t size)
{
}

static Eina_Bool
_efl_network_ssh_channel_base_binbuf_send(Eo *obj, Efl_Network_Ssh_Channel_Base_Data *pd, const Eina_Binbuf *data)
{
}

static Eina_Bool
_efl_network_ssh_channel_base_data_ext_send(Eo *obj, Efl_Network_Ssh_Channel_Base_Data *pd, unsigned int type, const void *data, size_t size)
{
}

static Eina_Bool
_efl_network_ssh_channel_base_binbuf_ext_send(Eo *obj, Efl_Network_Ssh_Channel_Base_Data *pd, unsigned int type, const Eina_Binbuf *data)
{
}

static Efl_Network_Ssh_Request *
_efl_network_ssh_channel_interactive_session_pty_req(Eo *obj, Efl_Network_Ssh_Channel_Interactive_Session_Data *pd, const char *term, const char *modes, unsigned int columns, unsigned int rows, unsigned int width, unsigned int height)
{
}

static void
_efl_network_ssh_channel_interactive_session_env_req(Eo *obj, Efl_Network_Ssh_Channel_Interactive_Session_Data *pd, const char *name, const char *value)
{
}

static Efl_Network_Ssh_Request *
_efl_network_ssh_channel_interactive_session_shell_req(Eo *obj, Efl_Network_Ssh_Channel_Interactive_Session_Data *pd)
{
}

static Efl_Network_Ssh_Request *
_efl_network_ssh_channel_interactive_session_exec_req(Eo *obj, Efl_Network_Ssh_Channel_Interactive_Session_Data *pd, const char *command)
{
}

#include "efl_network_ssh_client.eo.c"
#include "efl_network_ssh_channel_base.eo.c"
#include "efl_network_ssh_channel_interactive_session.eo.c"
