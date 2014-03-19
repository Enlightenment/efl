#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_APP_SERVER_VIEW_CLASS

#define MY_CLASS_NAME "Elm_App_Server_View"

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
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_CLOSED, NULL, NULL));

   return eldbus_message_method_return_new(message);
}

static Eldbus_Message *
_method_pause(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _state_set(data, ELM_APP_VIEW_STATE_PAUSED);
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_PAUSED, NULL, NULL));

   return eldbus_message_method_return_new(message);
}

static Eldbus_Message *
_method_resume(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_View_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _state_set(data, ELM_APP_VIEW_STATE_LIVE);
   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_RESUMED, NULL, NULL));

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

EOLIAN static void
_elm_app_server_view_resume(Eo *obj, Elm_App_Server_View_Data *data)
{
   _state_set(data, ELM_APP_VIEW_STATE_LIVE);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_RESUMED, NULL, NULL));
}

EOLIAN static void
_elm_app_server_view_pause(Eo *obj, Elm_App_Server_View_Data *data)
{
   _state_set(data, ELM_APP_VIEW_STATE_PAUSED);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_PAUSED, NULL, NULL));
}

EOLIAN static void
_elm_app_server_view_close(Eo *obj, Elm_App_Server_View_Data *data)
{
   _state_set(data, ELM_APP_VIEW_STATE_CLOSED);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_CLOSED, NULL, NULL));
}

EOLIAN static void
_elm_app_server_view_shallow(Eo *obj, Elm_App_Server_View_Data *data)
{
   _state_set(data, ELM_APP_VIEW_STATE_SHALLOW);
   eo_do(obj, eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_SHALLOW, NULL, NULL));
}

EOLIAN static Elm_App_View_State
_elm_app_server_view_state_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->state;
}

EOLIAN static void
_elm_app_server_view_window_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data, Evas_Object *win)
{
   int before = data->window_id;

   data->window_id = elm_win_window_id_get(win);

   if (data->window_id != before)
     eldbus_service_property_changed(data->iface, "WindowId");
}

EOLIAN static void
_elm_app_server_view_title_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data, const char *title)
{
   title = title ? title : "";

   if (eina_stringshare_replace(&data->title, title))
     eldbus_service_property_changed(data->iface, "Title");
}

EOLIAN static const char*
_elm_app_server_view_title_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->title;
}

EOLIAN static void
_elm_app_server_view_icon_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data, const char *icon)
{
   icon = icon ? icon : "";

   if (eina_stringshare_replace(&data->icon_name, icon))
     eldbus_service_property_changed(data->iface, "IconName");
}

EOLIAN static const char*
_elm_app_server_view_icon_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->icon_name;
}

EOLIAN static void
_elm_app_server_view_pixels_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data,
      unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
      Eina_Bool has_alpha EINA_UNUSED, const unsigned char *pixels EINA_UNUSED)
{
   //TODO
   eldbus_service_property_changed(data->iface, "IconPixels");
}

EOLIAN static void
_elm_app_server_view_pixels_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *pd EINA_UNUSED,
      unsigned int *w EINA_UNUSED, unsigned int *h EINA_UNUSED,
      Eina_Bool *has_alpha EINA_UNUSED, const unsigned char **pixels EINA_UNUSED)
{
   //TODO
}

EOLIAN static void
_elm_app_server_view_progress_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data, short progress)
{
   if (data->progress != progress)
     eldbus_service_property_changed(data->iface, "Progress");
   data->progress = progress;
}

EOLIAN static short
_elm_app_server_view_progress_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->progress;
}

EOLIAN static void
_elm_app_server_view_new_events_set(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data, int events)
{
   if (data->new_events != events)
     eldbus_service_property_changed(data->iface, "NewEvents");
   data->new_events = events;
}

EOLIAN static int
_elm_app_server_view_new_events_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->new_events;
}

EOLIAN static Eina_Stringshare*
_elm_app_server_view_id_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return data->id;
}

EOLIAN static const char*
_elm_app_server_view_path_get(Eo *obj EINA_UNUSED, Elm_App_Server_View_Data *data)
{
   return eldbus_service_object_path_get(data->iface);
}

EOLIAN static void
_elm_app_server_view_constructor(Eo *obj, Elm_App_Server_View_Data *data, const char *id)
{
   Elm_App_Server *server;
   const char *server_path;
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

EOLIAN static void
_elm_app_server_view_eo_base_destructor(Eo *obj, Elm_App_Server_View_Data *data)
{
   eina_stringshare_del(data->title);
   eina_stringshare_del(data->icon_name);

   eina_stringshare_del(data->id);
   eldbus_service_object_unregister(data->iface);
   eldbus_connection_unref(data->conn);
   eldbus_shutdown();

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EOLIAN static void
_elm_app_server_view_eo_base_constructor(Eo *obj, Elm_App_Server_View_Data *_pd EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

#include "elm_app_server_view.eo.c"
