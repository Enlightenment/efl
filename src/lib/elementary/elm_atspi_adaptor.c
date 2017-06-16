#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>

#include "elm_atspi_cache.eo.h"
#include "elm_atspi_cache.eo.legacy.h"
#include "elm_atspi_adaptor.eo.h"

typedef struct _Elm_Atspi_Adaptor_Data 
{
   Eldbus_Connection *conn;

   Eldbus_Signal_Handler *register_hdl;
   Eldbus_Signal_Handler *unregister_hdl;

   unsigned long object_broadcast_mask;
   unsigned long object_property_broadcast_mask;
   unsigned long object_children_broadcast_mask;
   unsigned long long object_state_broadcast_mask;
   unsigned long long window_signal_broadcast_mask;

   struct {
        Eldbus_Service_Interface *accessible;
        Eldbus_Service_Interface *application;
        Eldbus_Service_Interface *action;
        Eldbus_Service_Interface *component;
        Eldbus_Service_Interface *collection;
        Eldbus_Service_Interface *editable_text;
        Eldbus_Service_Interface *image;
        Eldbus_Service_Interface *selection;
        Eldbus_Service_Interface *text;
        Eldbus_Service_Interface *value;
        Eldbus_Service_Interface *socket;
   } interfaces;
} Elm_Atspi_Adaptor_Data;

EOLIAN static void
_elm_atspi_adaptor_event_handle(Elm_Atspi_Adaptor *adaptor, Elm_Atspi_Adaptor_Data *data, const Elm_Accessible_Event *accessible_event)
{
}

EOLIAN static Eo*
_elm_atspi_adaptor_efl_object_constructor(Elm_Atspi_Adaptor *adaptor, Elm_Atspi_Adaptor_Data *pd)
{
   return adaptor;
}

EOLIAN static void
_elm_atspi_adaptor_constructor(Elm_Atspi_Adaptor *adaptor, Elm_Atspi_Adaptor_Data *pd, Eldbus_Connection *conn)
{
   pd->conn = eldbus_connection_ref(conn);
   assert (pd->conn != NULL);

   //_register_interfaces();
   //_register_update_handlers();
}

EOLIAN static void
_elm_atspi_adaptor_efl_object_destructor(Elm_Atspi_Adaptor *adaptor, Elm_Atspi_Adaptor_Data *pd)
{
   //_unregister_interfaces();
   //unregister_update_handlers();
   eldbus_connection_unref(pd->conn);
   //efl_unref(pd->cache);
}

#include "elm_atspi_adaptor.eo.c"
