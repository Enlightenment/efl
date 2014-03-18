#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_APP_CLIENT_CLASS

#define MY_CLASS_NAME "Elm_App_Client"

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
          eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EVENT_VIEW_CREATED, view, NULL));
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

   eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EVENT_VIEW_LIST_LOADED,
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
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EVENT_VIEW_DELETED,
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
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EVENT_VIEW_DELETED,
                                         view, NULL));
        eo_del(view);
     }
}

EOLIAN static void
_elm_app_client_constructor(Eo *eo, Elm_App_Client_Data *data, const char *pkg)
{
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
        eo_do(eo, eo_event_callback_call(ELM_APP_CLIENT_EVENT_VIEW_CREATED,
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

EOLIAN static Elm_App_Client_Pending *
_elm_app_client_view_open(Eo *eo, Elm_App_Client_Data *data, Eina_Value *args, Elm_App_Client_Open_View_Cb cb, const void *user_data)
{
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
             return NULL;
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

   return pending;
}

EOLIAN static Eina_Iterator*
_elm_app_client_views_get(Eo *eo EINA_UNUSED, Elm_App_Client_Data *data)
{
   return eina_hash_iterator_data_new(data->views);
}

static void
_elm_app_client_view_all_close(Eo *obj EINA_UNUSED, Elm_App_Client_Data *data)
{
   eldbus_proxy_call(data->app_proxy, "CloseAllViews", NULL, NULL, -1, "");
}

EOLIAN static void
_elm_app_client_terminate(Eo *eo EINA_UNUSED, Elm_App_Client_Data *data)
{
   eldbus_proxy_call(data->app_proxy, "Terminate", NULL, NULL, -1, "");
}

EOLIAN static const char*
_elm_app_client_package_get(Eo *eo EINA_UNUSED, Elm_App_Client_Data *data)
{
   Eldbus_Object *obj;

   obj = eldbus_proxy_object_get(data->app_proxy);
   return eldbus_object_bus_name_get(obj);
}

EOLIAN static void
_elm_app_client_view_open_cancel(Eo *eo EINA_UNUSED, Elm_App_Client_Data *_pd EINA_UNUSED, Elm_App_Client_Pending *pending)
{
   eldbus_pending_cancel(pending);
}

EOLIAN static void
_elm_app_client_eo_base_constructor(Eo *obj, Elm_App_Client_Data *_pd EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("Only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

EOLIAN static void
_elm_app_client_eo_base_destructor(Eo *eo, Elm_App_Client_Data *data)
{
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

#include "elm_app_client.eo.c"
