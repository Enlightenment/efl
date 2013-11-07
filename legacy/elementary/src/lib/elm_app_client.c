#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

EAPI Eo_Op ELM_APP_CLIENT_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_APP_CLIENT_CLASS

#define MY_CLASS_NAME "Elm_App_Client"

EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_CREATED =
         EO_EVENT_DESCRIPTION("view,created",
                              "Called when a view of this application is created.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_DELETED =
         EO_EVENT_DESCRIPTION("view,deleted",
                              "Called when a view of this application is deleted.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_LIST_LOADED =
         EO_EVENT_DESCRIPTION("view_list,loaded",
                              "Called when list of view is loaded.");

EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_TERMINATED =
         EO_EVENT_DESCRIPTION("application,terminated",
                              "Called when application is terminated.");

typedef struct
{
   Eldbus_Proxy *app_proxy;
   Eina_Hash *views;
} Elm_App_Client_Data;

static void
_sub_path_process(Elm_App_Client *eo, Eldbus_Message_Iter *obj_iter, Elm_App_Client_Data *data, Eina_Bool loading_list)
{
   const char *obj_path;
   Eldbus_Message_Iter *array_iface, *iface;

   eldbus_message_iter_arguments_get(obj_iter, "oa{sa{sv}}", &obj_path, &array_iface);
   while (eldbus_message_iter_get_and_next(array_iface, '{', &iface))
     {
        const char *iface_name;
        Eldbus_Message_Iter *array_props;
        Elm_App_Client_View *view;

        eldbus_message_iter_arguments_get(iface, "sa{sv}", &iface_name,
                                          &array_props);
        if (strcmp(iface_name, "org.enlightenment.ApplicationView1"))
          continue;

        view = eina_hash_find(data->views, obj_path);
        if (view)
          continue;

        view = eo_add_custom(ELM_APP_CLIENT_VIEW_CLASS, eo,
                             elm_app_client_view_constructor(obj_path));
        eina_hash_add(data->views, obj_path, view);
        if (!loading_list)
          eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EV_VIEW_CREATED, view, NULL));
     }
}

static void
_objects_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eo *eo = data;
   Elm_App_Client_Data *cdata = eo_data_scope_get(eo, MY_CLASS);
   Eldbus_Message_Iter *array_path, *path;

   if (eldbus_message_error_get(msg, NULL, NULL))
     return;

   if (!eldbus_message_arguments_get(msg, "a{oa{sa{sv}}}", &array_path))
     return;
   while (eldbus_message_iter_get_and_next(array_path, '{', &path))
     _sub_path_process(eo, path, cdata, EINA_TRUE);

   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EV_VIEW_LIST_LOADED,
                                    NULL, NULL));
}

static void _iface_add(void *data, const Eldbus_Message *msg)
{
   Eo *eo = data;
   Elm_App_Client_Data *cdata = eo_data_scope_get(eo, MY_CLASS);
   Eldbus_Message_Iter *main_iter;

   main_iter = eldbus_message_iter_get(msg);
   _sub_path_process(eo, main_iter, cdata, EINA_FALSE);
}

static void
_iface_del(void *data, const Eldbus_Message *msg)
{
   Eo *eo = data;
   Elm_App_Client_Data *cdata = eo_data_scope_get(eo, MY_CLASS);
   const char *path, *iface;
   Eldbus_Message_Iter *array_iface;

   if (!eldbus_message_arguments_get(msg, "oas", &path, &array_iface))
     return;
   while (eldbus_message_iter_get_and_next(array_iface, 's', &iface))
     {
        Elm_App_Client_View *view;
        Elm_App_View_State view_state;

        if (strcmp(iface, "org.enlightenment.ApplicationView1"))
          continue;

        view = eina_hash_find(cdata->views, path);
        if (!view)
          continue;

        eo_do(view, elm_app_client_view_state_get(&view_state));
        if (view_state != ELM_APP_VIEW_STATE_CLOSED)
          {
             elm_app_client_view_internal_state_set(view,
                                                    ELM_APP_VIEW_STATE_SHALLOW);
             continue;
          }

        eina_hash_del(cdata->views, path, NULL);
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EV_VIEW_DELETED,
                                         view, NULL));
        eo_del(view);
     }
}

static void
_pkg_name_owner_changed_cb(void *data, const char *bus EINA_UNUSED, const char *old_id EINA_UNUSED, const char *new_id)
{
   Elm_App_Client *eo = data;
   Elm_App_Client_Data *cdata = eo_data_scope_get(eo, MY_CLASS);
   Eina_Iterator *iter;
   Elm_App_Client_View *view;
   Eina_List *views_list = NULL;

   if (!new_id || (new_id[0] == '\0'))
     return;

   iter = eina_hash_iterator_data_new(cdata->views);
   EINA_ITERATOR_FOREACH(iter, view)
     views_list = eina_list_append(views_list, view);
   eina_iterator_free(iter);

   /*
    * remove all views that are closed of the views hash
    * views not closed, only set they to SHALLOW
    */
   EINA_LIST_FREE(views_list, view)
     {
        Elm_App_View_State view_state;
        const char *path;

        eo_do(view, elm_app_client_view_state_get(&view_state),
              elm_app_client_view_path_get(&path));
        if (view_state != ELM_APP_VIEW_STATE_CLOSED)
          {
             elm_app_client_view_internal_state_set(view,
                                                    ELM_APP_VIEW_STATE_SHALLOW);
             continue;
          }

        eina_hash_del(cdata->views, path, NULL);
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EV_VIEW_DELETED,
                                         view, NULL));
        eo_del(view);
     }
}

static void
_app_client_constructor(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_Data *data = _pd;
   const char *pkg = va_arg(*list, const char *);
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   char *path;

   EINA_SAFETY_ON_NULL_GOTO(pkg, error);

   eo_do_super(eo, MY_CLASS, eo_constructor());

   data->views = eina_hash_string_small_new(NULL);

   path = _dbus_package_to_path(pkg);

   eldbus_init();
   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   obj = eldbus_object_get(conn, pkg, path);
   data->app_proxy = eldbus_proxy_get(obj, "org.enlightenment.Application1");
   eldbus_object_managed_objects_get(obj, _objects_get, eo);
   eldbus_object_manager_interfaces_added(obj, _iface_add, eo);
   eldbus_object_manager_interfaces_removed(obj, _iface_del, eo);
   eldbus_name_owner_changed_callback_add(conn, pkg, _pkg_name_owner_changed_cb,
                                          eo, EINA_FALSE);

   free(path);

   return;
error:
   eo_error_set(eo);
}

static void
_create_view_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Elm_App_Client_Open_View_Cb cb = eldbus_pending_data_del(pending, "user_cb");
   void *user_data = eldbus_pending_data_del(pending, "user_data");
   const char *error_name, *error_message, *view_path;
   Elm_App_Client *eo = data;
   Elm_App_Client_View *view;
   Elm_App_Client_Data *cdata = eo_data_scope_get(eo, MY_CLASS);

   if (eldbus_message_error_get(msg, &error_name, &error_message))
     {
        if (!cb)
          return;
        if (error_name && !strcmp(error_name, ELDBUS_ERROR_PENDING_CANCELED))
          cb(user_data, NULL, ELM_APP_CLIENT_VIEW_OPEN_CANCELED, NULL);
        else if (error_name && !strcmp(error_name, ELDBUS_ERROR_PENDING_TIMEOUT))
          cb(user_data, NULL, ELM_APP_CLIENT_VIEW_OPEN_TIMEOUT, error_message);
        else
          cb(user_data, NULL, error_name, error_message);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "o", &view_path))
     {
        if (cb)
          cb(user_data, NULL, "Unknow error", NULL);
        return;
     }

   /**
    * Because a IntefaceAdd signal could arrive first
    */
   view = eina_hash_find(cdata->views, view_path);
   if (!view)
     {
        view = eo_add_custom(ELM_APP_CLIENT_VIEW_CLASS, eo,
                             elm_app_client_view_constructor(view_path));
        eina_hash_add(cdata->views, view_path, view);
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EV_VIEW_CREATED,
                                         view, NULL));
     }

   if (!view)
     {
        if (cb)
          cb(user_data, NULL, ELM_APP_CLEINT_VIEW_OPEN_ERROR, NULL);
        return;
     }
   if (cb)
     cb(user_data, view, NULL, NULL);
}

static void
_view_open(Eo *eo, void *_pd, va_list *list)
{
   Elm_App_Client_Data *data = _pd;
   Eina_Value *args = va_arg(*list, Eina_Value *);
   Elm_App_Client_Open_View_Cb cb = va_arg(*list, Elm_App_Client_Open_View_Cb);
   const void *user_data = va_arg(*list, const void *);
   Elm_App_Client_Pending **view_open_pending = va_arg(*list, Elm_App_Client_Pending**);
   Eldbus_Message *msg;
   Eldbus_Pending *pending;

   msg = eldbus_proxy_method_call_new(data->app_proxy, "CreateView");

   if (args)
     {
        if (!eldbus_message_from_eina_value("a{sv}", msg, args))
          {
             eldbus_message_unref(msg);
             //TODO test to find out what type eina_value must be
             ERR("Eina_Value of args don't have a structure of a{sv}");
             return;
          }
     }
   else
    {
       Eldbus_Message_Iter *main_iter = eldbus_message_iter_get(msg);
       Eldbus_Message_Iter *array;

       eldbus_message_iter_arguments_append(main_iter, "a{sv}", &array);
       eldbus_message_iter_container_close(main_iter, array);
    }

   pending = eldbus_proxy_send(data->app_proxy, msg, _create_view_cb, eo, -1);
   if (user_data)
     eldbus_pending_data_set(pending, "user_data", user_data);
   if (cb)
     eldbus_pending_data_set(pending, "user_cb", cb);

   if (view_open_pending)
     *view_open_pending = pending;
}

static void
_views_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_Data *data = _pd;
   Eina_Iterator **view_iter = va_arg(*list, Eina_Iterator **);

   *view_iter = eina_hash_iterator_data_new(data->views);
}

static void
_all_close(Eo *eo EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Client_Data *data = _pd;
   eldbus_proxy_call(data->app_proxy, "CloseAllViews", NULL, NULL, -1, "");
}

static void
_terminate(Eo *eo EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Client_Data *data = _pd;
   eldbus_proxy_call(data->app_proxy, "Terminate", NULL, NULL, -1, "");
}

static void
_package_get(Eo *eo EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_App_Client_Data *data = _pd;
   Eldbus_Object *obj;
   const char **package = va_arg(*list, const char **);

   obj = eldbus_proxy_object_get(data->app_proxy);
   *package = eldbus_object_bus_name_get(obj);
}

static void
_view_open_cancel(Eo *eo EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_App_Client_Pending *pending = va_arg(*list, Elm_App_Client_Pending *);
   eldbus_pending_cancel(pending);
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

static void
_destructor(Eo *eo, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_App_Client_Data *data = _pd;
   Eldbus_Object *obj;
   Eldbus_Connection *conn;
   Eina_Iterator *iter;
   Elm_App_Client_View *view;

   iter = eina_hash_iterator_data_new(data->views);
   EINA_ITERATOR_FOREACH(iter, view)
     eo_del(view);
   eina_iterator_free(iter);
   eina_hash_free(data->views);

   obj = eldbus_proxy_object_get(data->app_proxy);
   conn = eldbus_object_connection_get(obj);
   eldbus_name_owner_changed_callback_del(conn, eldbus_object_bus_name_get(obj),
                                          _pkg_name_owner_changed_cb, eo);
   eldbus_proxy_unref(data->app_proxy);
   eldbus_object_unref(obj);
   eldbus_connection_unref(conn);
   eldbus_shutdown();

   eo_do_super(eo, MY_CLASS, eo_destructor());
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_CONSTRUCTOR), _app_client_constructor),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN), _view_open),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEWS_GET), _views_get),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEWS_ALL_CLOSE), _all_close),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_TERMINATE), _terminate),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_PACKAGE_GET), _package_get),
      EO_OP_FUNC(ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN_CANCEL), _view_open_cancel),
      EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_CONSTRUCTOR,
                       "Constructor of elm_app_client."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN, "Open a view."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_VIEWS_GET,
                       "Return a iterator with all views of application."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_VIEWS_ALL_CLOSE,
                       "Close all views of application."),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_TERMINATE,
                       "Terminate applicaiton"),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_PACKAGE_GET,
                       "Return the package name of application"),
     EO_OP_DESCRIPTION(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN_CANCEL,
                       "Cancel view opening."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
     ELM_APP_CLIENT_EV_VIEW_CREATED,
     ELM_APP_CLIENT_EV_VIEW_DELETED,
     ELM_APP_CLIENT_EV_VIEW_LIST_LOADED,
     ELM_APP_CLIENT_EV_TERMINATED,
     NULL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_APP_CLIENT_BASE_ID, op_desc, ELM_APP_CLIENT_SUB_ID_LAST),
     event_desc,
     sizeof(Elm_App_Client_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_app_client_class_get, &class_desc, EO_BASE_CLASS, NULL);
