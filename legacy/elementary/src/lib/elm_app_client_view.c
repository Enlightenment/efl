#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

EAPI Eo_Op ELM_APP_CLIENT_VIEW_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_APP_CLIENT_VIEW_CLASS

#define MY_CLASS_NAME "Elm_App_Client_View"

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED =
         EO_EVENT_DESCRIPTION("state,changed", "State of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED =
         EO_EVENT_DESCRIPTION("title,changed", "Title of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED =
         EO_EVENT_DESCRIPTION("icon,changed", "Icon of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED =
         EO_EVENT_DESCRIPTION("icon,pixels,changed", "Icons pixels of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED =
         EO_EVENT_DESCRIPTION("progress,changed", "Progress of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED =
         EO_EVENT_DESCRIPTION("new_events,changed", "New events of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED =
         EO_EVENT_DESCRIPTION("window,changed", "Window of view changed.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED =
         EO_EVENT_DESCRIPTION("property,changed", "One of view properties changed.");

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
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED,
                                      _string_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "IconName"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED,
                                      _string_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "IconPixels"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED,
                                      NULL, NULL));
   else if (!strcmp(prop_event->name, "NewEvents"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED,
                                      (void *)(uintptr_t)_int_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "Progress"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED,
                                      (void *)(uintptr_t)_short_prop_get(v), NULL));
   else if (!strcmp(prop_event->name, "State"))
     {
        cdata->state = _string_state_to_id(_string_prop_get(v));
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED,
                                         (void *)(uintptr_t)cdata->state, NULL));
     }
   else if (!strcmp(prop_event->name, "WindowId"))
     eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED,
                                      (void *)(uintptr_t)_int_prop_get(v), NULL));
   else
      return;

   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED,
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
   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED,
                                    (void *)(uintptr_t)cdata->state, NULL));
}

static void
_app_client_view_constructor(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   Elm_App_Client *parent;
   const char *package, *path = va_arg(*list, const char *);
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

   eldbus_message_error_get(msg, &error, &error_message);

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

static void
_client_resume(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   Elm_App_Client_View_Cb cb = va_arg(*list, Elm_App_Client_View_Cb);
   const void *user_data = va_arg(*list, const void *);

   _dbus_action_do(eo, data->view_proxy, "Resume", cb, user_data);
}

static void
_client_pause(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   Elm_App_Client_View_Cb cb = va_arg(*list, Elm_App_Client_View_Cb);
   const void *user_data = va_arg(*list, const void *);

   _dbus_action_do(eo, data->view_proxy, "Pause", cb, user_data);
}

static void
_client_close(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   Elm_App_Client_View_Cb cb = va_arg(*list, Elm_App_Client_View_Cb);
   const void *user_data = va_arg(*list, const void *);

   _dbus_action_do(eo, data->view_proxy, "Close", cb, user_data);
}

static void _eo_string_prop_get(void *_pd, va_list *list, const char *prop)
{
   Elm_App_Client_View_Data *data = _pd;
   const char **title = va_arg(*list, const char **);
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, prop);
   *title = _string_prop_get(v);
}

static void _client_title_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   _eo_string_prop_get(_pd, list, "Title");
}

static void _client_icon_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   _eo_string_prop_get(_pd, list, "IconName");
}

static void _client_icon_pixels_get(Eo *eo EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //Elm_App_Client_View_Data *data = _pd;
   //TODO
}

static void _client_progress_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   short *progress = va_arg(*list, short *);
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "Progress");
   *progress = _short_prop_get(v);
}

static void _client_new_events_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   int *new_events = va_arg(*list, int *);
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "NewEvents");
   *new_events = _int_prop_get(v);
}

static void _client_state_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   Elm_App_View_State *state = va_arg(*list, Elm_App_View_State *);

   *state = data->state;
}

static void _client_window_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   int *window = va_arg(*list, int *);
   Eina_Value *v;

   v = eldbus_proxy_property_local_get(data->view_proxy, "WindowId");
   *window = _int_prop_get(v);
}

static void _client_path_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   const char **ret = va_arg(*list, const char **);
   Eldbus_Object *obj;

   obj = eldbus_proxy_object_get(data->view_proxy);
   *ret = eldbus_object_path_get(obj);
}

static void _client_package_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_View_Data *data = _pd;
   const char **ret = va_arg(*list, const char **);
   Eldbus_Object *obj;

   obj = eldbus_proxy_object_get(data->view_proxy);
   *ret = eldbus_object_bus_name_get(obj);
}

static void
_destructor(Eo *eo, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Client_View_Data *data = _pd;
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

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_CONSTRUCTOR), _app_client_view_constructor),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_CLOSE), _client_close),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PAUSE), _client_pause),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_RESUME), _client_resume),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_TITLE_GET), _client_title_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_GET), _client_icon_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_PIXELS_GET), _client_icon_pixels_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PROGRESS_GET), _client_progress_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_NEW_EVENTS_GET), _client_new_events_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_STATE_GET), _client_state_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_WINDOW_GET), _client_window_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PATH_GET), _client_path_get),
      EO_OP_FUNC(ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PACKAGE_GET), _client_package_get),
      EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_CONSTRUCTOR, "Constructor of elm_app_client_view."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_CLOSE, "Close view."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_PAUSE, "Pause view."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_RESUME, "Resume view."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_TITLE_GET, "Get view title."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_GET, "Get view icon."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_PIXELS_GET, "Get view icon pixels."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_PROGRESS_GET, "Get view progress."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_NEW_EVENTS_GET, "Get view new events."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_STATE_GET, "Get view state."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_WINDOW_GET, "Get view window id."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_PATH_GET, "Get view dbus path."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_VIEW_SUB_ID_PACKAGE_GET, "Get application package."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED,
     ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_APP_CLIENT_VIEW_BASE_ID, op_desc, ELM_APP_CLIENT_VIEW_SUB_ID_LAST),
     event_desc,
     sizeof(Elm_App_Client_View_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_app_client_view_class_get, &class_desc, EO_BASE_CLASS, NULL);
