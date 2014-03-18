#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_APP_CLIENT_VIEW_CLASS

#define MY_CLASS_NAME "Elm_App_Client_View"

typedef struct
{
   Eldbus_Proxy *view_proxy;
   Elm_App_View_State state;
} Elm_App_Client_View_Data;

static const char *_string_prop_get(const Eina_Value *v)
{
   const char *str;

   if (!v)
     return "";

   eina_value_get(v, &str);
   return str;
}

static int _int_prop_get(const Eina_Value *v)
{
   int num;

   if (!v)
     return 0;

   eina_value_get(v, &num);
   return num;
}

static short _short_prop_get(const Eina_Value *v)
{
   short num;

   if (!v)
     return 0;

   eina_value_get(v, &num);
   return num;
}

static void
_prop_changed(void *user_data, Eldbus_Proxy *proxy EINA_UNUSED, void *event_info)
{
   Eldbus_Proxy_Event_Property_Changed *prop_event = event_info;
   Elm_App_Client_View *eo = user_data;
   const Eina_Value *v = prop_event->value;
   Elm_App_Client_View_Data *cdata = eo_data_scope_get(eo, MY_CLASS);

   if (!strcmp(prop_event->name, "Title"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_TITLE_CHANGED,
                                      _string_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "IconName"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_ICON_CHANGED,
                                      _string_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "IconPixels"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_ICON_PIXELS_CHANGED,
                                      NULL, NULL));
   else if (!strcmp(prop_event->name, "NewEvents"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_NEW_EVENTS_CHANGED,
                                      (void *)(uintptr_t)_int_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "Progress"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_PROGRESS_CHANGED,
                                      (void *)(uintptr_t)_short_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "State"))
     {
        cdata->state = _string_state_to_id(_string_prop_get(v));
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_STATE_CHANGED,
                                         (void *)(uintptr_t)cdata->state, NULL));
     }
   else if (!strcmp(prop_event->name, "WindowId"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_WINDOW_CHANGED,
                                      (void *)(uintptr_t)_int_prop_get(v), NULL));
   else
      return;

   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_PROPERTY_CHANGED,
                                    prop_event->name, NULL));
}

static void
_props_loaded(void *user_data, Eldbus_Proxy *proxy EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const Eina_Hash *props = eldbus_proxy_property_local_get_all(proxy);
   Eina_Iterator *iter;
   Eina_Hash_Tuple *t;

   iter = eina_hash_iterator_tuple_new(props);
   EINA_ITERATOR_FOREACH(iter, t)
     {
        Eldbus_Proxy_Event_Property_Changed event;

        event.name = t->key;
        event.proxy = proxy;
        event.value = t->data;

        _prop_changed(user_data, proxy, &event);
     }
   eina_iterator_free(iter);
}

void
elm_app_client_view_internal_state_set(Eo *eo, Elm_App_View_State state)
{
   Elm_App_Client_View_Data *cdata;
   Eina_Bool changed = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN(eo);
   EINA_SAFETY_ON_FALSE_RETURN(eo_isa(eo, ELM_APP_CLIENT_VIEW_CLASS));

   cdata = eo_data_scope_get(eo, MY_CLASS);
   changed = cdata->state != state;
   cdata->state = state;
   if (!changed)
     return;
   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EVENT_STATE_CHANGED,
                                    (void *)(uintptr_t)cdata->state, NULL));
}

EOLIAN static void
_elm_app_client_view_constructor(Eo *eo, Elm_App_Client_View_Data *data, const char *path)
{
   Elm_App_Client *parent;
   const char *package = path;
   Eldbus_Connection *conn;
   Eldbus_Object *obj;

   EINA_SAFETY_ON_NULL_GOTO(path, error);
   eo_do_super(eo, MY_CLASS, eo_constructor());

   eo_do(eo, eo_parent_get(&parent));
   EINA_SAFETY_ON_TRUE_GOTO((!parent) ||
                            (!eo_isa(parent, ELM_APP_CLIENT_CLASS)), error);

   eo_do(parent, elm_app_client_package_get(&package));

   eldbus_init();
   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   obj = eldbus_object_get(conn, package, path);
   data->view_proxy = eldbus_proxy_get(obj,
                                       "org.enlightenment.ApplicationView1");
   eldbus_proxy_properties_monitor(data->view_proxy, EINA_TRUE);
   eldbus_proxy_event_callback_add(data->view_proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _prop_changed, eo);
   eldbus_proxy_event_callback_add(data->view_proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
                                   _props_loaded, eo);

   return;

error:
   eo_error_set(eo);
}

static void
_dbus_state_set_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Elm_App_Client_View *eo = data;
   Elm_App_Client_View_Cb cb = eldbus_pending_data_del(pending, "cb");
   void *user_data = eldbus_pending_data_del(pending, "user_data");
   const char *error = NULL, *error_message = NULL;

   if (!eldbus_message_error_get(msg, &error, &error_message))
     error_message = NULL;

   cb(user_data, eo, error, error_message);
}

static void
_dbus_action_do(Eo *eo, Eldbus_Proxy *proxy, const char *action, Elm_App_Client_View_Cb cb, const void *data)
{
   Eldbus_Pending *pending;

   if (!cb)
     {
        eldbus_proxy_call(proxy, action, NULL, NULL, -1, "");
        return;
     }

   pending = eldbus_proxy_call(proxy, action, _dbus_state_set_cb, eo, -1, "");
   eldbus_pending_data_set(pending, "cb", cb);
   eldbus_pending_data_set(pending, "user_data", data);
}

EOLIAN static void
_elm_app_client_view_resume(Eo *eo, Elm_App_Client_View_Data *data, Elm_App_Client_View_Cb cb, const void *user_data)
{

   _dbus_action_do(eo, data->view_proxy, "Resume", cb, user_data);
}

EOLIAN static void
_elm_app_client_view_pause(Eo *eo, Elm_App_Client_View_Data *data, Elm_App_Client_View_Cb cb, const void *user_data)
{

   _dbus_action_do(eo, data->view_proxy, "Pause", cb, user_data);
}

EOLIAN static void
_elm_app_client_view_close(Eo *eo, Elm_App_Client_View_Data *data, Elm_App_Client_View_Cb cb, const void *user_data)
{

   _dbus_action_do(eo, data->view_proxy, "Close", cb, user_data);
}

EOLIAN static const char*
_elm_app_client_view_title_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "Title");
   return _string_prop_get(v);
}

EOLIAN static const char*
_elm_app_client_view_icon_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "IconName");
   return _string_prop_get(v);
}

EOLIAN static void
_elm_app_client_view_icon_pixels_get(Eo *obj EINA_UNUSED, Elm_App_Client_View_Data *pd EINA_UNUSED,
      unsigned int *w EINA_UNUSED, unsigned int *h EINA_UNUSED,
      Eina_Bool *has_alpha EINA_UNUSED, const unsigned char **pixels EINA_UNUSED)
{
   //TODO
}

EOLIAN static unsigned short
_elm_app_client_view_progress_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "Progress");
   return _short_prop_get(v);
}

EOLIAN static int
_elm_app_client_view_new_events_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "NewEvents");
   return _int_prop_get(v);
}

EOLIAN static Elm_App_View_State
_elm_app_client_view_state_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   return data->state;
}

EOLIAN static int
_elm_app_client_view_window_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "WindowId");
   return _int_prop_get(v);
}

EOLIAN static const char*
_elm_app_client_view_path_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eldbus_Object *obj;

   obj = eldbus_proxy_object_get(data->view_proxy);
   return eldbus_object_path_get(obj);
}

EOLIAN static const char*
_elm_app_client_view_package_get(Eo *eo EINA_UNUSED, Elm_App_Client_View_Data *data)
{
   Eldbus_Object *obj;

   obj = eldbus_proxy_object_get(data->view_proxy);
   return eldbus_object_bus_name_get(obj);
}

EOLIAN static void
_elm_app_client_view_eo_base_destructor(Eo *eo, Elm_App_Client_View_Data *data)
{
   Eldbus_Object *obj;
   Eldbus_Connection *conn;

   eldbus_proxy_properties_monitor(data->view_proxy, EINA_FALSE);
   obj = eldbus_proxy_object_get(data->view_proxy);
   conn = eldbus_object_connection_get(obj);

   eldbus_proxy_unref(data->view_proxy);
   eldbus_object_unref(obj);
   eldbus_connection_unref(conn);
   eldbus_shutdown();

   eo_do_super(eo, MY_CLASS, eo_destructor());
}

EOLIAN static void
_elm_app_client_view_eo_base_constructor(Eo *obj, Elm_App_Client_View_Data *_pd EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

#include "elm_app_client_view.eo.c"
