#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

EAPI Eo_Op ELM_APP_SERVER_VIEW_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_APP_SERVER_VIEW_CLASS

#define MY_CLASS_NAME "Elm_App_Server_View"

EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_RESUMED =
         EO_EVENT_DESCRIPTION("resume", "Called when view must be resumed");

EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_PAUSED =
         EO_EVENT_DESCRIPTION("paused", "Called when view must be paused");

EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_CLOSED =
         EO_EVENT_DESCRIPTION("closed", "Called when view must be closed");

EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_SHALLOW =
         EO_EVENT_DESCRIPTION("shallow", "Called when view state is set to shallow");

EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_SAVE =
         EO_EVENT_DESCRIPTION("save", "Called when view state should be saved");

typedef struct
{
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;

   Eina_Stringshare *id;
   Eina_Stringshare *title;
   Eina_Stringshare *icon_name;
   int new_events;
   short progress;
   Elm_App_View_State state;
   int window_id;
   struct {
      unsigned int w, h;
      Eina_Bool has_alpha;
      unsigned char *raw;
   } image_raw;
} Elm_App_Server_View_Data;

static void
_state_set(Elm_App_Server_View_Data *data, Elm_App_View_State state)
{
   if (data->state != state)
     eldbus_service_property_changed(data->iface, "State");
   data->state = state;
}

static Eldbus_Message *
_method_close(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _state_set(data, ELM_APP_VIEW_STATE_CLOSED);
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_CLOSED, NULL, NULL));

   return eldbus_message_method_return_new(message);
}

static Eldbus_Message *
_method_pause(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _state_set(data, ELM_APP_VIEW_STATE_PAUSED);
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_PAUSED, NULL, NULL));

   return eldbus_message_method_return_new(message);
}

static Eldbus_Message *
_method_resume(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _state_set(data, ELM_APP_VIEW_STATE_LIVE);
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_RESUMED, NULL, NULL));

   return eldbus_message_method_return_new(message);
}

static const Eldbus_Method _methods[] = {
   { "Close", NULL, NULL, _method_close, 0 },
   { "Pause", NULL, NULL, _method_pause, 0 },
   { "Resume", NULL, NULL, _method_resume, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static Eina_Bool
_prop_title_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 's', data->title);
   return EINA_TRUE;
}

static Eina_Bool
_prop_icon_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 's', data->icon_name);
   return EINA_TRUE;
}

/*static Eina_Bool
_prop_icon_pixels_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   //TODO
   return EINA_FALSE;
}*/

static Eina_Bool
_prop_events_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 'i', data->new_events);
   return EINA_TRUE;
}

static Eina_Bool
_prop_progress_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 'n', data->progress);
   return EINA_TRUE;
}

static Eina_Bool
_prop_state_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 's', _state_id_to_string(data->state));
   return EINA_TRUE;
}

static Eina_Bool
_prop_window_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 'i', data->window_id);
   return EINA_TRUE;
}

static const Eldbus_Property _props[] = {
   { "Title", "s", _prop_title_get, NULL, 0 },
   { "IconName", "s", _prop_icon_get, NULL, 0 },
   //{ "IconPixels", "(uubay)", _prop_icon_pixels_get, NULL, 0 },
   { "NewEvents", "i", _prop_events_get, NULL, 0 },
   { "Progress", "n", _prop_progress_get, NULL, 0 },
   { "State", "s", _prop_state_get, NULL, 0 },
   { "WindowId", "i", _prop_window_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Signal _signals[] = {
   { "Result", ELDBUS_ARGS({"a{sv}", "arguments"}), 0 },
   { NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc iface_desc = {
   "org.enlightenment.ApplicationView1", _methods, _signals, _props, NULL, NULL
};

static void
_server_resume(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;

   _state_set(data, ELM_APP_VIEW_STATE_LIVE);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_RESUMED, NULL, NULL));
}

static void
_server_pause(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;

   _state_set(data, ELM_APP_VIEW_STATE_PAUSED);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_PAUSED, NULL, NULL));
}

static void
_server_close(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;

   _state_set(data, ELM_APP_VIEW_STATE_CLOSED);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_CLOSED, NULL, NULL));
}

static void
_server_shallow(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;

   _state_set(data, ELM_APP_VIEW_STATE_SHALLOW);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_SHALLOW, NULL, NULL));
}

static void
_server_state_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;
   Elm_App_View_State *ret = va_arg(*list, Elm_App_View_State *);
   *ret = data->state;
}

static void
_server_window_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   Evas_Object *win = va_arg(*list, Evas_Object *);
   int before = data->window_id;

   data->window_id = elm_win_window_id_get(win);

   if (data->window_id != before)
     eldbus_service_property_changed(data->iface, "WindowId");
}

static void
_server_title_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   const char *title = va_arg(*list, const char *);

   title = title ? title : "";

   if (eina_stringshare_replace(&data->title, title))
     eldbus_service_property_changed(data->iface, "Title");
}

static void
_server_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   const char **ret = va_arg(*list, const char **);
   *ret = data->title;
}

static void
_server_icon_name_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   const char *icon = va_arg(*list, const char *);

   icon = icon ? icon : "";

   if (eina_stringshare_replace(&data->icon_name, icon))
     eldbus_service_property_changed(data->iface, "IconName");
}

static void
_server_icon_name_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   const char **ret = va_arg(*list, const char **);
   *ret = data->icon_name;
}

static void
_server_icon_pixels_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;
   //TODO
   eldbus_service_property_changed(data->iface, "IconPixels");
}

static void
_server_icon_pixels_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //TODO
}

static void
_server_progress_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   short progress = va_arg(*list, int);

   if (data->progress != progress)
     eldbus_service_property_changed(data->iface, "Progress");
   data->progress = progress;
}

static void
_server_progress_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   short *ret = va_arg(*list, short *);

   *ret = data->progress;
}

static void
_server_new_events_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   int events = va_arg(*list, int);

   if (data->new_events != events)
     eldbus_service_property_changed(data->iface, "NewEvents");
   data->new_events = events;
}

static void
_server_new_events_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   int *ret = va_arg(*list, int *);

   *ret = data->new_events;
}

static void
_server_id_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   Eina_Stringshare **ret = va_arg(*list, Eina_Stringshare **);

   *ret = data->id;
}

static void
_server_path_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   const char **ret = va_arg(*list, const char **);

   *ret = eldbus_service_object_path_get(data->iface);
}

static void
_app_server_view_constructor(Eo *obj, void *_pd, va_list *list)
{
   Elm_App_Server_View_Data *data = _pd;
   Elm_App_Server *server;
   const char *id = va_arg(*list, const char *), *server_path;
   char view_path[PATH_MAX];

   eo_do_super(obj, MY_CLASS, eo_constructor());

   eo_do(obj, eo_parent_get(&server));
   EINA_SAFETY_ON_TRUE_GOTO(!server || !eo_isa(server, ELM_APP_SERVER_CLASS), error);

   if (!id)
     {
        int i;
        for (i = 1; i < 99999; i++)
          {
             char buf[64];
             Eina_Bool valid;

             snprintf(buf, sizeof(buf), "view_%d", i);
             eo_do(server, elm_app_server_view_check(buf, &valid));
             if (valid)
               {
                  data->id = eina_stringshare_add(buf);
                  break;
               }
          }
     }
   else
     {
        Eina_Bool valid;
        eo_do(server, elm_app_server_view_check(id, &valid));
        if (valid)
          data->id = eina_stringshare_add(id);
     }

   EINA_SAFETY_ON_NULL_GOTO(data->id, error);

   eo_do(server, elm_app_server_path_get(&server_path));
   snprintf(view_path, sizeof(view_path), "%s/%s", server_path, data->id);

   eldbus_init();
   data->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   data->iface = eldbus_service_interface_register(data->conn, view_path,
                                                   &iface_desc);
   eldbus_service_object_data_set(data->iface, MY_CLASS_NAME, obj);

   data->state = ELM_APP_VIEW_STATE_UNKNOWN;
   data->title = eina_stringshare_add("");
   data->icon_name = eina_stringshare_add("");

   return;

error:
   eo_error_set(obj);
}

static void
_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_View_Data *data = _pd;

   eina_stringshare_del(data->title);
   eina_stringshare_del(data->icon_name);

   eina_stringshare_del(data->id);
   eldbus_service_object_unregister(data->iface);
   eldbus_connection_unref(data->conn);
   eldbus_shutdown();

   eo_do_super(obj, MY_CLASS, eo_destructor());
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
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_CONSTRUCTOR), _app_server_view_constructor),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_RESUME), _server_resume),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PAUSE), _server_pause),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_CLOSE), _server_close),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_SHALLOW), _server_shallow),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_STATE_GET), _server_state_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_WINDOW_SET), _server_window_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_SET), _server_title_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_GET), _server_title_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_SET), _server_icon_name_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_GET), _server_icon_name_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_SET), _server_icon_pixels_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_GET), _server_icon_pixels_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_SET), _server_progress_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_GET), _server_progress_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_SET), _server_new_events_set),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_GET), _server_new_events_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ID_GET), _server_id_get),
      EO_OP_FUNC(ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PATH_GET), _server_path_get),
      EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_CONSTRUCTOR,
                       "Constructor of elm_app_server_view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_RESUME, "Resume view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_PAUSE, "Pause view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_CLOSE, "Close view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_SHALLOW,
                       "Shallow view, that means that view is open but dont have a window(X or Wayland)."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_STATE_GET,
                       "Get state of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_WINDOW_SET,
                       "Set window of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_SET,
                       "Set title of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_GET,
                       "Get title of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_SET,
                       "Set icon name or path of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_GET,
                       "Get icon name or path of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_SET,
                       "Set icon to view, using the raw pixels of image."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_GET,
                       "Get raw icon of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_SET,
                       "Set progress of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_GET,
                       "Get progress of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_SET,
                       "Set new events of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_GET,
                       "Get events of view."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_ID_GET,
                       "Get view identifier."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_VIEW_SUB_ID_PATH_GET,
                       "Get view DBus path."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     ELM_APP_SERVER_VIEW_EV_RESUMED,
     ELM_APP_SERVER_VIEW_EV_PAUSED,
     ELM_APP_SERVER_VIEW_EV_CLOSED,
     ELM_APP_SERVER_VIEW_EV_SHALLOW,
     ELM_APP_SERVER_VIEW_EV_SAVE,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_APP_SERVER_VIEW_BASE_ID, op_desc, ELM_APP_SERVER_VIEW_SUB_ID_LAST),
     event_desc,
     sizeof(Elm_App_Server_View_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_app_server_view_class_get, &class_desc, EO_BASE_CLASS, NULL);
