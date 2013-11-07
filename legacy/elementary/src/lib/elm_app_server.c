#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_app_server_eet.h"

EAPI Eo_Op ELM_APP_SERVER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_APP_SERVER_CLASS

#define MY_CLASS_NAME "Elm_App_Server"

EAPI const Eo_Event_Description _ELM_APP_SERVER_EV_TERMINATE =
         EO_EVENT_DESCRIPTION("terminate",
                              "Called when application must be terminated.");

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
   Eina_Stringshare *view_id;

   eo_do(view, elm_app_server_view_id_get(&view_id));

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
   Eina_Stringshare *error_name, *error_message, *view_path;
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
   eo_do(view, elm_app_server_view_path_get(&view_path));

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

   eo_do(eo, eo_event_callback_call(ELM_APP_SERVER_EV_TERMINATE, NULL, NULL));

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
   "org.enlightenment.Application1", _methods, NULL, _props, NULL, NULL
};

static void
_app_server_constructor(Eo *obj, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   const char *pkg = va_arg(*list, const char *);
   char *path;
   Elm_App_Server_Views_Eet *views_eet;
   char buf[PATH_MAX];

   data->create_view_cb = va_arg(*list, Elm_App_Server_Create_View_Cb);

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

static void
_save(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_App_Server_Data *data = _pd;
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
        const char *id, *title, *icon_name;
        int new_events;
        short progress;

        eo_do(view, elm_app_server_view_id_get(&id),
              elm_app_server_view_title_get(&title),
              elm_app_server_view_icon_get(&icon_name),
              elm_app_server_view_new_events_get(&new_events),
              elm_app_server_view_progress_get(&progress),
              eo_event_callback_call(ELM_APP_SERVER_VIEW_EV_SAVE, NULL, NULL));

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

static void
_close_all(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_Data *data = _pd;
   _close_all_views(data);
}

static void
_view_check(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   const char *id = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const char *ptr;

   *ret = EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN(id);

   for (ptr = id; *ptr; ptr++)
     {
        if (!isalnum(*ptr))
          break;
     }

   if (!eina_hash_find(data->views, id))
     *ret = EINA_TRUE;
}

static void
_title_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   const char *title = va_arg(*list, const char *);

   title = title ? title : "";

   if (eina_stringshare_replace(&data->title, title))
     eldbus_service_property_changed(data->iface, "Title");
}

static void
_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   Eina_Stringshare **ret = va_arg(*list, Eina_Stringshare **);

   *ret = data->title;
}

static void
_icon_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   const char *icon = va_arg(*list, const char *);

   icon = icon ? icon : "";

   if (eina_stringshare_replace(&data->icon_name, icon))
     eldbus_service_property_changed(data->iface, "IconName");
}

static void
_icon_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   Eina_Stringshare **ret = va_arg(*list, Eina_Stringshare **);

   *ret = data->icon_name;
}

static void
_icon_pixels_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //TODO
}

static void
_icon_pixels_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //TODO
}

static void
_package_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_App_Server_Data *data = _pd;
   Eina_Stringshare **ret = va_arg(*list, Eina_Stringshare **);

   *ret = data->pkg;
}

static void
_views_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   Eina_Iterator **ret = va_arg(*list, Eina_Iterator **);

   *ret = eina_hash_iterator_data_new(data->views);
}

static void
_path_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   const char **path = va_arg(*list, const char **);

   *path = eldbus_service_object_path_get(data->iface);
}

static void
_view_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Server_Data *data = _pd;
   Elm_App_Server_View *view = va_arg(*list, Elm_App_Server_View *);
   _view_append(data, view);
}

static void
_destructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Server_Data *data = _pd;
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
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_CONSTRUCTOR),
                 _app_server_constructor),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_SAVE), _save),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_CLOSE_ALL),
                 _close_all),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEW_CHECK),
                 _view_check),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_TITLE_SET),
                 _title_set),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_TITLE_GET),
                 _title_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_SET), _icon_set),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_GET), _icon_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_SET),
                 _icon_pixels_set),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_GET),
                 _icon_pixels_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_PACKAGE_GET),
                 _package_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEWS_GET),
                 _views_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_PATH_GET), _path_get),
      EO_OP_FUNC(ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEW_ADD), _view_add),
      EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_CONSTRUCTOR,
                       "Constructor of elm_app_server."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_SAVE,
                       "Save current state of applications views."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_CLOSE_ALL, "Close all views."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_VIEW_CHECK,
                       "Return the view dbus object path if id available."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_TITLE_SET,
                       "Set title of application."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_TITLE_GET,
                       "Get title of application."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_ICON_SET,
                       "Set icon of applicaiton."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_ICON_GET,
                       "Get icon of applicaiton."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_SET,
                       "Set icon to application, using the raw pixels of image."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_GET,
                       "Get application raw icon."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_PACKAGE_GET,
                       "Get application package."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_VIEWS_GET,
                       "Return iterator with all views of application."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_PATH_GET,
                       "Get application DBus path."),
     EO_OP_DESCRIPTION(ELM_APP_SERVER_SUB_ID_VIEW_ADD,
                       "Add a view to application."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     ELM_APP_SERVER_EV_TERMINATE,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_APP_SERVER_BASE_ID, op_desc, ELM_APP_SERVER_SUB_ID_LAST),
     event_desc,
     sizeof(Elm_App_Server_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_app_server_class_get, &class_desc, EO_BASE_CLASS, NULL);
