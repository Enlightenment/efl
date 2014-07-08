#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_app_server_eet.h"

#define MY_CLASS ELM_APP_SERVER_CLASS

#define MY_CLASS_NAME "Elm_App_Server"

typedef struct
{
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;
   Elm_App_Server_Create_View_Cb create_view_cb;
   Eina_Hash *views;

   int PID;
   Eina_Stringshare *title;
   Eina_Stringshare *icon_name;
   Eina_Stringshare *pkg;
   struct {
      unsigned int w, h;
      Eina_Bool has_alpha;
      unsigned char *raw;
   } image_raw;
} Elm_App_Server_Data;

static Eina_Bool
_view_del_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Server_Data *cdata = data;
   eina_hash_del(cdata->views, NULL, obj);
   return EINA_TRUE;
}

static Eina_Bool
_view_append(Elm_App_Server_Data *data, Elm_App_Server_View *view)
{
   Eina_Stringshare *view_id = NULL;

   eo_do(view, view_id = elm_app_server_view_id_get());

   if (eina_hash_find(data->views, view_id))
     {
        ERR("%p already have a view with id=%s", data->pkg, view_id);
        eo_del(view);
        return EINA_FALSE;
     }

   eina_hash_add(data->views, view_id, view);
   eo_do(view, eo_event_callback_add(EO_EV_DEL, _view_del_cb, data));
   return EINA_TRUE;
}

static Eldbus_Message *
_method_create_view(const Eldbus_Service_Interface *iface, const Eldbus_Message *message)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_Data *data = eo_data_scope_get(eo, MY_CLASS);
   Eina_Value *args;
   Eina_Stringshare *error_name, *error_message, *view_path = NULL;
   Elm_App_Server_View *view;
   Eldbus_Message *reply;

   args = eldbus_message_to_eina_value(message);
   if (!args)
     return eldbus_message_error_new(message, ELM_APP_CLEINT_VIEW_OPEN_ERROR, "");

   view = data->create_view_cb(eo, args, &error_name, &error_message);
   eina_value_free(args);

   if (!view)
     {
        reply = eldbus_message_error_new(message, error_name, error_message);
        eina_stringshare_del(error_name);
        eina_stringshare_del(error_message);
        return reply;
     }

   if (!_view_append(data, view))
     {
        reply = eldbus_message_error_new(message,
                                         ELM_APP_SERVER_VIEW_CREATE_DUPLICATE,
                                         NULL);
        return reply;
     }
   eo_do(view, view_path = elm_app_server_view_path_get());

   reply = eldbus_message_method_return_new(message);
   eldbus_message_arguments_append(reply, "o", view_path);

   return reply;
}

static void
_close_all_views(Elm_App_Server_Data *data)
{
   Eina_Iterator *iter;
   Elm_App_Server_View *view;
   Eina_List *view_list = NULL;

   /*
    * need do this because hash will be modified when called
    * elm_app_server_view_close()
    */
   iter = eina_hash_iterator_data_new(data->views);
   EINA_ITERATOR_FOREACH(iter, view)
     view_list = eina_list_append(view_list, view);
   eina_iterator_free(iter);

   EINA_LIST_FREE(view_list, view)
     eo_do(view, elm_app_server_view_close());
}

static Eldbus_Message *
_method_close_all_views(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *message EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_Data *data = eo_data_scope_get(eo, MY_CLASS);

   _close_all_views(data);

   return eldbus_message_method_return_new(message);
}

static Eldbus_Message *
_method_terminate(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *message EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);

   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_EVENT_TERMINATE, NULL));

   return eldbus_message_method_return_new(message);
}

static const Eldbus_Method _methods[] = {
   { "CreateView", ELDBUS_ARGS({"a{sv}", "arguments"}),
     ELDBUS_ARGS({"o", "view_path"}), _method_create_view, 0 },
   { "CloseAllViews", NULL, NULL, _method_close_all_views, 0},
   { "Terminate", NULL, NULL, _method_terminate, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static Eina_Bool
_prop_pid_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 'i', data->PID);
   return EINA_TRUE;
}

static Eina_Bool
_prop_title_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 's', data->title);
   return EINA_TRUE;
}

static Eina_Bool
_prop_icon_get(const Eldbus_Service_Interface *iface, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   Eo *eo = eldbus_service_object_data_get(iface, MY_CLASS_NAME);
   Elm_App_Server_Data *data = eo_data_scope_get(eo, MY_CLASS);

   eldbus_message_iter_basic_append(iter, 's', data->icon_name);
   return EINA_TRUE;
}

/*static Eina_Bool
_prop_icon_pixels_get(const Eldbus_Service_Interface *iface EINA_UNUSED, const char *propname EINA_UNUSED, Eldbus_Message_Iter *iter EINA_UNUSED, const Eldbus_Message *request_msg EINA_UNUSED, Eldbus_Message **error EINA_UNUSED)
{
   //TODO
   return EINA_FALSE;
}*/

static const Eldbus_Property _props[] = {
   { "PID", "i", _prop_pid_get, NULL, 0 },
   { "Title", "s", _prop_title_get, NULL, 0 },
   { "IconName", "s", _prop_icon_get, NULL, 0 },
   //{ "IconPixels", "(uubay)", _prop_icon_pixels_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc iface_desc = {
   "org.enlightenment.Application1", _methods, NULL, _props, NULL, NULL, NULL
};

EOLIAN static void
_elm_app_server_constructor(Eo *obj, Elm_App_Server_Data *data, const char *pkg, Elm_App_Server_Create_View_Cb create_view_cb)
{
   char *path;
   Elm_App_Server_Views_Eet *views_eet;
   char buf[PATH_MAX];

   data->create_view_cb = create_view_cb;

   EINA_SAFETY_ON_NULL_GOTO(data->create_view_cb, error);
   EINA_SAFETY_ON_TRUE_GOTO(!pkg, error);

   eo_do_super(obj, MY_CLASS, eo_constructor());

   data->views = eina_hash_string_small_new(NULL);
   data->PID = getpid();
   data->title = eina_stringshare_add("");
   data->icon_name = eina_stringshare_add("");
   data->pkg = eina_stringshare_add(pkg);

   path = _dbus_package_to_path(pkg);

   eldbus_init();
   data->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   eldbus_name_request(data->conn, pkg, 0, NULL, NULL);
   data->iface = eldbus_service_interface_register(data->conn,
                                                   path,
                                                   &iface_desc);
   eldbus_service_object_data_set(data->iface, MY_CLASS_NAME, obj);
   eldbus_service_object_manager_attach(data->iface);

   free(path);

   //Check if there a saved list of views
   app_server_views_eet_init();

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "apps/%s.eet",
                                 data->pkg);
   views_eet = elm_app_server_views_eet_load(buf);
   if (views_eet)
     {
        Elm_App_Server_View_Props *view_eet_props;
        Eina_List *l, *views_list;

        views_list = elm_app_server_views_eet_views_entries_list_get(views_eet);
        if (!views_list)
          goto free_views_eet;

        EINA_LIST_FOREACH(views_list, l, view_eet_props)
          {
             Elm_App_Server_View *view;
             const char *view_id, *title, *icon_name;
             int new_events;
             short progress;

             view_id = elm_app_server_view_props_id_get(view_eet_props);

             if (eina_hash_find(data->views, view_id))
               continue;
             title = elm_app_server_view_props_title_get(view_eet_props);
             icon_name = elm_app_server_view_props_icon_name_get(view_eet_props);
             new_events = elm_app_server_view_props_new_events_get(view_eet_props);
             progress = elm_app_server_view_props_progress_get(view_eet_props);

             view = eo_add_custom(ELM_APP_SERVER_VIEW_CLASS, obj,
                                  elm_app_server_view_constructor(view_id));
             if (!view)
               continue;

             eo_do(view, elm_app_server_view_title_set(title),
                   elm_app_server_view_icon_set(icon_name),
                   elm_app_server_view_progress_set(progress),
                   elm_app_server_view_new_events_set(new_events),
                   eo_event_callback_add(EO_EV_DEL, _view_del_cb, data),
                   elm_app_server_view_shallow());

             eina_hash_add(data->views, view_id, view);
          }

free_views_eet:
         elm_app_server_views_eet_free(views_eet);
     }

   app_server_views_eet_shutdown();

   return;

error:
   eo_error_set(obj);
}

EOLIAN static void
_elm_app_server_save(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   Eina_Iterator *iter;
   Elm_App_Server_View *view;
   Elm_App_Server_Views_Eet *views_eet;
   char buf[PATH_MAX];

   app_server_views_eet_init();

   views_eet = elm_app_server_views_eet_new(1, NULL);

   iter = eina_hash_iterator_data_new(data->views);
   EINA_ITERATOR_FOREACH(iter, view)
     {
        Elm_App_Server_View_Props *view_props;
        const char *id = NULL, *title = NULL, *icon_name = NULL;
        int new_events = 0;
        short progress = 0;

        eo_do(view, id = elm_app_server_view_id_get(),
              title = elm_app_server_view_title_get(),
              icon_name = elm_app_server_view_icon_get(),
              new_events = elm_app_server_view_new_events_get(),
              progress = elm_app_server_view_progress_get(),
              eo_event_callback_call(ELM_APP_SERVER_VIEW_EVENT_SAVE, NULL));

        view_props = elm_app_server_view_props_new(id, title, icon_name,
                                                   new_events, progress);
        elm_app_server_views_eet_views_entries_add(views_eet, view_props);
     }
   eina_iterator_free(iter);

   _elm_config_user_dir_snprintf(buf, sizeof(buf), "apps/");
   if (!ecore_file_is_dir(buf))
     ecore_file_mkpath(buf);
   _elm_config_user_dir_snprintf(buf, sizeof(buf), "apps/%s.eet", data->pkg);
   elm_app_server_views_eet_save(views_eet, buf);
   elm_app_server_views_eet_free(views_eet);
   app_server_views_eet_shutdown();
}

EOLIAN static void
_elm_app_server_close_all(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   _close_all_views(data);
}

EOLIAN static Eina_Bool
_elm_app_server_view_check(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data, const char *id)
{
   const char *ptr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(id, EINA_FALSE);

   for (ptr = id; *ptr; ptr++)
     {
        if (!isalnum(*ptr))
          break;
     }

   if (!eina_hash_find(data->views, id)) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static void
_elm_app_server_title_set(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data, const char *title)
{
   title = title ? title : "";

   if (eina_stringshare_replace(&data->title, title))
     eldbus_service_property_changed(data->iface, "Title");
}

EOLIAN static Eina_Stringshare *
_elm_app_server_title_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   return data->title;
}

EOLIAN static void
_elm_app_server_icon_set(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data, const char *icon)
{
   icon = icon ? icon : "";

   if (eina_stringshare_replace(&data->icon_name, icon))
     eldbus_service_property_changed(data->iface, "IconName");
}

EOLIAN static Eina_Stringshare *
_elm_app_server_icon_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   return data->icon_name;
}

EOLIAN static void
_elm_app_server_pixels_set(Eo *obj EINA_UNUSED, Elm_App_Server_Data *pd EINA_UNUSED,
      unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
      Eina_Bool has_alpha EINA_UNUSED, const unsigned char *pixels EINA_UNUSED)
{
   //TODO
}

EOLIAN static void
_elm_app_server_pixels_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *pd EINA_UNUSED,
      unsigned int *w EINA_UNUSED, unsigned int *h EINA_UNUSED,
      Eina_Bool *has_alpha EINA_UNUSED, const unsigned char **pixels EINA_UNUSED)
{
   //TODO
}

EOLIAN static Eina_Stringshare*
_elm_app_server_package_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   return data->pkg;
}

EOLIAN static Eina_Iterator*
_elm_app_server_views_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   return eina_hash_iterator_data_new(data->views);
}

EOLIAN static const char*
_elm_app_server_path_get(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data)
{
   return eldbus_service_object_path_get(data->iface);
}

EOLIAN static void
_elm_app_server_view_add(Eo *obj EINA_UNUSED, Elm_App_Server_Data *data, Elm_App_Server_View *view)
{
   _view_append(data, view);
}

EOLIAN static void
_elm_app_server_eo_base_destructor(Eo *obj, Elm_App_Server_Data *data)
{
   Eina_Iterator *iter;
   Elm_App_Server_View *view;

   iter = eina_hash_iterator_data_new(data->views);
   EINA_ITERATOR_FOREACH(iter, view)
     {
        //do not modify hash when iterating
        eo_do(view, eo_event_callback_del(EO_EV_DEL, _view_del_cb, data));
        eo_del(view);
     }
   eina_iterator_free(iter);
   eina_hash_free(data->views);

   eina_stringshare_del(data->title);
   eina_stringshare_del(data->icon_name);
   eina_stringshare_del(data->pkg);

   eldbus_service_object_manager_detach(data->iface);
   eldbus_service_interface_unregister(data->iface);
   eldbus_connection_unref(data->conn);
   eldbus_shutdown();
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EOLIAN static void
_elm_app_server_eo_base_constructor(Eo *obj, Elm_App_Server_Data *_pd EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

#include "elm_app_server.eo.c"
