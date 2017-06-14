#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <assert.h>

#include "atspi/atspi-constants.h"
#include "elm_priv.h"
#include "elm_atspi_device_event_controller.eo.h"

typedef struct _Elm_Atspi_Device_Event_Controller_Data {
     Eldbus_Connection *conn;
     Eina_List *events_history;
     Ecore_Event_Filter *events_filter;
     Eina_List *pendings;
} Elm_Atspi_Device_Event_Controller_Data;

struct Keybard_Event_Info {
     Ecore_Event_Key event;
     int type;
     void *data;
};

static void
_key_event_info_free(struct Keybard_Event_Info *data)
{
   EINA_SAFETY_ON_NULL_RETURN(data);

   eina_stringshare_del(data->event.keyname);
   eina_stringshare_del(data->event.key);
   eina_stringshare_del(data->event.string);
   eina_stringshare_del(data->event.compose);

   free(data);
}

static short
_ecore_modifiers_2_atspi(unsigned int modifiers)
{
   short ret = 0;

   if (modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     ret |= (1 << ATSPI_MODIFIER_SHIFT);
   if (modifiers & ECORE_EVENT_MODIFIER_CAPS)
     ret |= (1 << ATSPI_MODIFIER_SHIFTLOCK);
   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     ret |= (1 << ATSPI_MODIFIER_CONTROL);
   if (modifiers & ECORE_EVENT_MODIFIER_ALT)
     ret |= (1 << ATSPI_MODIFIER_ALT);
   if (modifiers & ECORE_EVENT_MODIFIER_WIN)
     ret |= (1 << ATSPI_MODIFIER_META);
   if (modifiers & ECORE_EVENT_MODIFIER_NUM)
     ret |= (1 << ATSPI_MODIFIER_NUMLOCK);

   return ret;
}

static void
_on_event_del(void *user_data, void *func_data EINA_UNUSED)
{
   struct Keybard_Event_Info *info = user_data;
   _key_event_info_free(info);
}

static Eina_Bool
_is_in_history(Elm_Atspi_Device_Event_Controller_Data *pd, Ecore_Event_Key *event)
{
   if (eina_list_data_find(pd->events_history, event))
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

static void
_remove_from_history(Elm_Atspi_Device_Event_Controller_Data *pd, Ecore_Event_Key *event)
{
    pd->events_history = eina_list_remove(pd->events_history, event);
}

static void
_add_to_history(Elm_Atspi_Device_Event_Controller_Data *pd, Ecore_Event_Key *event)
{
   pd->events_history = eina_list_append(pd->events_history, event);
}

static void
_add_to_pendings(Elm_Atspi_Device_Event_Controller_Data *pd, Eldbus_Pending *p)
{
   pd->pendings = eina_list_append(pd->pendings, p);
}

static void
_remove_from_pendings(Elm_Atspi_Device_Event_Controller_Data *pd, Eldbus_Pending *p)
{
   pd->pendings = eina_list_remove(pd->pendings, p);
}

static void
_reemit_event(Elm_Atspi_Device_Event_Controller_Data *pd, struct Keybard_Event_Info *info)
{
   ecore_event_add(info->type, &info->event, _on_event_del, info);
   _add_to_history(pd, &info->event);
}

static void
_on_listener_answer(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   struct Keybard_Event_Info *info = data;
   const char *errname, *errmsg;
   Eina_Bool ret = EINA_TRUE;
   Elm_Atspi_Device_Event_Controller_Data *pd = info->data;

   _remove_from_pendings(pd, pending);

   if (eldbus_message_error_get(msg, &errname, &errmsg) ||
       !eldbus_message_arguments_get(msg, "b", &ret) ||
       !ret)
     {
        _reemit_event(pd, info);
     }
   else
     {
        _key_event_info_free(info);
     }
}

static struct Keybard_Event_Info*
_key_event_info_new(int event_type, const Ecore_Event_Key *data, void *user_data)
{
   struct Keybard_Event_Info *ret;
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   ret = calloc(1, sizeof(struct Keybard_Event_Info));
   if (!ret) return NULL;

   ret->type = event_type;
   ret->event = *data;

   ret->event.keyname = eina_stringshare_add(data->keyname);
   ret->event.key = eina_stringshare_add(data->key);
   ret->event.string = eina_stringshare_add(data->string);
   ret->event.compose = eina_stringshare_add(data->compose);
   ret->event.modifiers = data->modifiers;
   ret->data = user_data;

   // not sure what this field is for, but explicite keep it NULLed.
   ret->event.data = NULL;

   return ret;
}

static void
_iter_marshall_key_event(Eldbus_Message_Iter *iter, struct Keybard_Event_Info *data)
{
   Eldbus_Message_Iter *struct_iter;
   EINA_SAFETY_ON_NULL_RETURN(data);

   struct_iter = eldbus_message_iter_container_new(iter, 'r', NULL);

   const char *str = data->event.keyname ? data->event.keyname : "";
   int is_text = data->event.keyname ? 1 : 0;
   int type;
   if (data->type == ECORE_EVENT_KEY_DOWN)
     type = ATSPI_KEY_PRESSED_EVENT;
   else
     type = ATSPI_KEY_RELEASED_EVENT;

   eldbus_message_iter_arguments_append(
      struct_iter, "uinnisb", type, 0, data->event.keycode,
      _ecore_modifiers_2_atspi(data->event.modifiers), data->event.timestamp,
      str, is_text);

   eldbus_message_iter_container_close(iter, struct_iter);
}

static Eina_Bool
_is_keyboard_event(int type)
{
   if ((type != ECORE_EVENT_KEY_DOWN) &&
       (type != ECORE_EVENT_KEY_UP))
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

static Eina_Bool
_notify_listener_sync(Elm_Atspi_Device_Event_Controller_Data *pd, struct Keybard_Event_Info *ke)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter;
   Eldbus_Pending *p;

   msg = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY,
                                        ATSPI_DBUS_PATH_DEC,
                                        ATSPI_DBUS_INTERFACE_DEC,
                                        "NotifyListenersSync");
   iter = eldbus_message_iter_get(msg);
   _iter_marshall_key_event(iter, ke);

   // timeout should be kept reasonaby low to avoid delays
   p = eldbus_connection_send(pd->conn, msg, _on_listener_answer, ke, 100);
   if (p)
     {
        _add_to_pendings(pd, p);
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
}

static Eina_Bool
_ecore_filter_cb(void *data, void *loop EINA_UNUSED, int type, void *event)
{
   Ecore_Event_Key *key_event = event;
   struct Keybard_Event_Info *ke;
   Elm_Atspi_Device_Event_Controller_Data *pd = data;

   if (!_is_keyboard_event(type))
     return EINA_TRUE; // do not hold

   // check if event mark to reemission
   if (_is_in_history(pd, key_event))
     {
        _remove_from_history(pd, key_event);
        return EINA_TRUE; // do not hold
     }

   // create event description so event could be reemited after
   // confirmation from atspi2 registryd
   ke = _key_event_info_new(type, key_event, pd);
   if (!ke) return EINA_TRUE;

   // notify atspi2 registryd about event
   if (_notify_listener_sync(pd, ke))
     return EINA_FALSE; // hold event
   else
     return EINA_TRUE;
}

EOLIAN static void
_elm_atspi_device_event_controller_constructor(
   Elm_Atspi_Device_Event_Controller *obj EINA_UNUSED,
   Elm_Atspi_Device_Event_Controller_Data *pd EINA_UNUSED,
   Eldbus_Connection *conn)
{
   pd->conn = eldbus_connection_ref(conn);
   assert (pd->conn != NULL);

   pd->events_filter = ecore_event_filter_add(NULL,
                                        _ecore_filter_cb,
                                        NULL,
                                        pd);
   assert (pd->events_filter != NULL);
}

EOLIAN static void
_elm_atspi_device_event_controller_efl_object_destructor(
   Elm_Atspi_Device_Event_Controller *obj EINA_UNUSED,
   Elm_Atspi_Device_Event_Controller_Data *pd)
{
   Eldbus_Pending *p;
   // flush all holded events
   EINA_LIST_FREE(pd->pendings, p)
      eldbus_pending_cancel(p);

   eina_list_free(pd->events_history);
   ecore_event_filter_del(pd->events_filter);
   eldbus_connection_unref(pd->conn);

   efl_destructor(efl_super(obj, ELM_ATSPI_DEVICE_EVENT_CONTROLLER_CLASS));
}

#include "elm_atspi_device_event_controller.eo.c"
