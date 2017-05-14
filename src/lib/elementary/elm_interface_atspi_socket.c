#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

typedef struct _Elm_Inteface_Atspi_Socket_Data
{
   const char  *id;
   Eina_Bool occupied : 1;
} Elm_Interface_Atspi_Socket_Data;

EOLIAN const char*
_elm_interface_atspi_socket_id_get(Eo *socket EINA_UNUSED, Elm_Interface_Atspi_Socket_Data *pd)
{
   return pd->id;
}

EOLIAN void
_elm_interface_atspi_socket_id_set(Eo *socket, Elm_Interface_Atspi_Socket_Data *pd, const char *new_id)
{
   const char *id = eina_stringshare_add(new_id);
   if (pd->id != id)
     {
        eina_stringshare_del(pd->id);
        pd->id = id;
        efl_event_callback_call(socket, ELM_INTERFACE_ATSPI_SOCKET_EVENT_ID_CHANGED, NULL);
     }
   else
     eina_stringshare_del(id);
}

EOLIAN Eina_Bool
_elm_interface_atspi_socket_occupied_get(Eo *socket EINA_UNUSED, Elm_Interface_Atspi_Socket_Data *pd)
{
   return pd->occupied;
}

EOLIAN void
_elm_interface_atspi_socket_occupied_set(Eo *socket EINA_UNUSED, Elm_Interface_Atspi_Socket_Data *pd, Eina_Bool val)
{
   pd->occupied = val;
}

EOLIAN void
_elm_interface_atspi_socket_on_embedded(Eo *socket, Elm_Interface_Atspi_Socket_Data *pd EINA_UNUSED, Elm_Atspi_Proxy *proxy)
{
   efl_event_callback_call(socket, ELM_INTERFACE_ATSPI_SOCKET_EVENT_EMBEDDED, proxy);
}

EOLIAN void
_elm_interface_atspi_socket_on_unembedded(Eo *socket, Elm_Interface_Atspi_Socket_Data *pd EINA_UNUSED, Elm_Atspi_Proxy *proxy)
{
   efl_event_callback_call(socket, ELM_INTERFACE_ATSPI_SOCKET_EVENT_UNEMBEDDED, proxy);
}

EOLIAN void
_elm_interface_atspi_socket_on_connected(Eo *socket, Elm_Interface_Atspi_Socket_Data *pd EINA_UNUSED)
{
}

EOLIAN void
_elm_interface_atspi_socket_on_disconnected(Eo *socket, Elm_Interface_Atspi_Socket_Data *pd EINA_UNUSED)
{
}

#include "elm_interface_atspi_socket.eo.c"
