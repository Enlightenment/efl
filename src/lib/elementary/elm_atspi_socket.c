#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>

typedef struct _Elm_Atspi_Socket_Data {
} Elm_Atspi_Socket_Data;

EOLIAN void
_elm_atspi_socket_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_Socket_Data *pd)
{
   efl_destructor(efl_super(obj, ELM_ATSPI_SOCKET_CLASS));
}

EOLIAN Eo*
_elm_atspi_socket_efl_object_constructor(Eo *obj, Elm_Atspi_Socket_Data *pd EINA_UNUSED)
{
   efl_constructor(efl_super(obj, ELM_ATSPI_SOCKET_CLASS));
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_EMBEDDED);

   return obj;
}
#include "elm_atspi_socket.eo.c"
