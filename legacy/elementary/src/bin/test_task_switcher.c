#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Eina.h>

#define APPS_COL 0
#define VIEWS_COL 1
#define PROPS_VIEW_COL 2

static Evas_Object *table = NULL;
static Elm_App_Client *app_selected = NULL;
static Eina_List *apps_list = NULL;

static void _app_view_clicked(void *data, Evas_Object *obj, void *event_info);
static void _apps_list_update(void);

static void _btn_close_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client_View *view = data;
   eo_do(view, elm_app_client_view_close(NULL, NULL));
}

static void _btn_pause_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client_View *view = data;
   eo_do(view, elm_app_client_view_pause(NULL, NULL));
}

static void _btn_resume_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client_View *view = data;
   eo_do(view, elm_app_client_view_resume(NULL, NULL));
}

static Eina_Bool
_app_view_prop_changed_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_View_State state;

   eo_do(obj, elm_app_client_view_state_get(&state));

   if (state == ELM_APP_VIEW_STATE_CLOSED)
     {
        Evas_Object *props = elm_table_child_get(table, PROPS_VIEW_COL, 0);
        elm_table_unpack(table, props);
        evas_object_del(props);
     }
   else
     _app_view_clicked(obj, NULL, NULL);

   return EINA_TRUE;
}

static void
_app_view_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client_View *view = data;
   Evas_Object *view_props, *close_btn, *resume_btn, *pause_btn;
   const char *title, *icon;
   int new_events, window;
   unsigned short progress;
   Elm_App_View_State state;
   char buffer[1024];

   view_props = elm_table_child_get(table, PROPS_VIEW_COL, 0);
   if (view_props)
     {
        Elm_App_Client_View *old_view;

        old_view = evas_object_data_del(view_props, "view");
        eo_do(old_view,
              eo_event_callback_del(ELM_APP_CLIENT_VIEW_EVENT_PROPERTY_CHANGED,
                                    _app_view_prop_changed_cb, table));
        elm_list_clear(view_props);
     }
   else
     {
        view_props = elm_list_add(table);
        evas_object_size_hint_align_set(view_props, EVAS_HINT_FILL,
                                        EVAS_HINT_FILL);
        evas_object_show(view_props);
        elm_table_pack(table, view_props, PROPS_VIEW_COL, 0, 1, 1);
     }

   evas_object_data_set(view_props, "view", view);

   eo_do(view, elm_app_client_view_title_get(&title),
         elm_app_client_view_icon_get(&icon),
         elm_app_client_view_progress_get(&progress),
         elm_app_client_view_new_events_get(&new_events),
         elm_app_client_view_window_get(&window),
         eo_event_callback_add(ELM_APP_CLIENT_VIEW_EVENT_PROPERTY_CHANGED,
                               _app_view_prop_changed_cb, table));

   snprintf(buffer, sizeof(buffer), "Title=%s", title);
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   snprintf(buffer, sizeof(buffer), "Icon=%s", icon);
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   snprintf(buffer, sizeof(buffer), "Progress=%d", progress);
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   snprintf(buffer, sizeof(buffer), "New events=%d", new_events);
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   snprintf(buffer, sizeof(buffer), "WindowID=%d", window);
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   eo_do(view, elm_app_client_view_state_get(&state));
   if (state == ELM_APP_VIEW_STATE_LIVE)
     snprintf(buffer, sizeof(buffer), "State=alive");
   else if (state == ELM_APP_VIEW_STATE_PAUSED)
     snprintf(buffer, sizeof(buffer), "State=paused");
   else if (state == ELM_APP_VIEW_STATE_CLOSED)
     snprintf(buffer, sizeof(buffer), "State=closed");
   else if (state == ELM_APP_VIEW_STATE_SHALLOW)
     snprintf(buffer, sizeof(buffer), "State=shallow");
   else
     snprintf(buffer, sizeof(buffer), "State=unknown");
   elm_list_item_append(view_props, buffer, NULL, NULL, NULL, NULL);

   close_btn = elm_button_add(view_props);
   elm_object_text_set(close_btn, "Close view");
   evas_object_smart_callback_add(close_btn, "clicked", _btn_close_cb, view);
   elm_list_item_append(view_props, NULL, close_btn, NULL, NULL, NULL);

   pause_btn = elm_button_add(view_props);
   elm_object_text_set(pause_btn, "Pause view");
   evas_object_smart_callback_add(pause_btn, "clicked", _btn_pause_cb, view);
   elm_list_item_append(view_props, NULL, pause_btn, NULL, NULL, NULL );

   resume_btn = elm_button_add(view_props);
   elm_object_text_set(resume_btn, "Resume view");
   evas_object_smart_callback_add(resume_btn, "clicked", _btn_resume_cb, view);
   elm_list_item_append(view_props, NULL, resume_btn, NULL, NULL, NULL );

   elm_list_go(view_props);
}

static void
_popup_close_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void app_client_view_open_cb(void *data EINA_UNUSED, Elm_App_Client_View *view, const char *error, const char *error_message)
{
   Evas_Object *popup, *btn_ok;
   char buf[1024];

   popup = elm_popup_add(table);

   if (view)
     {
        _app_view_clicked(view, NULL, NULL);
        return;
     }

   snprintf(buf, sizeof(buf), "Some error happen opening view: %s %s", error, error_message);
   elm_object_part_text_set(popup, "default", buf);

   btn_ok = elm_button_add(popup);
   elm_object_text_set(btn_ok, "Ok");
   elm_object_part_content_set(popup, "button1", btn_ok);
   evas_object_smart_callback_add(btn_ok, "clicked", _popup_close_cb, popup);

   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
   evas_object_show(popup);
}

static void _popup_btn_open_view_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *popup = data;
   Eina_Value *args = NULL;//TODO fill with args of popup
   Elm_App_Client *app = evas_object_data_get(popup, "app");

   eo_do(app, elm_app_client_view_open(args, app_client_view_open_cb, NULL, NULL));

   evas_object_del(popup);
}

static void
_app_view_open(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client *app = data;
   Evas_Object *popup, *btn_open, *btn_cancel, *args_box;

   popup = elm_popup_add(table);
   elm_object_part_text_set(popup, "title,text", "Open a view");

   btn_open = elm_button_add(popup);
   elm_object_text_set(btn_open, "Open");
   elm_object_part_content_set(popup, "button1", btn_open);
   evas_object_smart_callback_add(btn_open, "clicked", _popup_btn_open_view_cb, popup);
   evas_object_show(btn_open);

   btn_cancel = elm_button_add(popup);
   elm_object_text_set(btn_cancel, "Cancel");
   elm_object_part_content_set(popup, "button2", btn_cancel);
   evas_object_smart_callback_add(btn_cancel, "clicked", _popup_close_cb, popup);
   evas_object_show(btn_cancel);

   args_box = elm_box_add(popup);
   elm_object_part_content_set(popup, "default", args_box);
   elm_object_part_text_set(popup, "default", "TODO: add some entrys to add some parameters to view.");

   evas_object_data_set(popup, "app", app);
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
   evas_object_show(popup);
}

static void _app_close_all_views_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client *app = data;
   eo_do(app, elm_app_client_view_all_close());
}

static void
_app_terminate_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Client *app = data;
   eo_do(app, elm_app_client_terminate());
}

static void
_app_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *views, *btn_close_all, *btn_terminate, *btn_open_view;
   Eina_Iterator *views_iter;
   Elm_App_Client_View *client_view;
   Elm_App_Client *app = data;

   app_selected = app;

   views = elm_table_child_get(table, VIEWS_COL, 0);
   if (!views)
     {
        views = elm_list_add(table);
        evas_object_size_hint_align_set(views, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(views);
        elm_table_pack(table, views, VIEWS_COL, 0, 1, 1);
     }
   else
     elm_list_clear(views);

   btn_open_view = elm_button_add(views);
   elm_object_text_set(btn_open_view, "Open a view");
   evas_object_smart_callback_add(btn_open_view, "clicked", _app_view_open, app);
   elm_list_item_append(views, NULL, btn_open_view, NULL, NULL, NULL);

   eo_do(app, elm_app_client_views_get(&views_iter));
   EINA_ITERATOR_FOREACH(views_iter, client_view)
     {
        const char *path;

        eo_do(client_view, elm_app_client_view_path_get(&path));
        elm_list_item_append(views, path, NULL, NULL, _app_view_clicked, client_view);
     }
   eina_iterator_free(views_iter);

   btn_close_all = elm_button_add(views);
   elm_object_text_set(btn_close_all, "Close all views");
   evas_object_smart_callback_add(btn_close_all, "clicked", _app_close_all_views_cb, app);
   elm_list_item_append(views, NULL, btn_close_all, NULL, NULL, NULL);

   btn_terminate = elm_button_add(views);
   elm_object_text_set(btn_terminate, "Terminate application");
   evas_object_smart_callback_add(btn_terminate, "clicked", _app_terminate_cb, app);
   elm_list_item_append(views, NULL, btn_terminate, NULL, NULL, NULL);

   elm_list_go(views);
}

static Eina_Bool
_view_list_update_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (app_selected == obj)
     _app_clicked(obj, NULL, NULL);
   return EINA_TRUE;
}

static void
_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_App_Client *app;
   EINA_LIST_FREE(apps_list, app)
     eo_unref(app);
}

static void
_app_open(const char *package)
{
   Elm_App_Client *app;
   Eina_List *l;

   EINA_LIST_FOREACH(apps_list, l, app)
     {
        const char *app_package;

        eo_do(app, elm_app_client_package_get(&app_package));
        if (!strcmp(package, app_package))
          return;
     }

   app = eo_add_custom(ELM_APP_CLIENT_CLASS, NULL,
                       elm_app_client_constructor(package));
   eo_do(app,
         eo_event_callback_add(ELM_APP_CLIENT_EVENT_VIEW_LIST_LOADED,
                               _view_list_update_cb, table),
         eo_event_callback_add(ELM_APP_CLIENT_EVENT_VIEW_CREATED,
                               _view_list_update_cb, table),
         eo_event_callback_add(ELM_APP_CLIENT_EVENT_VIEW_DELETED,
                               _view_list_update_cb, table));
   apps_list = eina_list_append(apps_list, app);
}

static void
_btn_app_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *entry = data;
   const char *pkg = elm_object_text_get(entry);

   if (!pkg || !pkg[0])
     return;

   _app_open(pkg);
   _apps_list_update();
}

static void
_open_custom_app_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   elm_list_item_selected_set(event_info, EINA_FALSE);
   elm_object_focus_set(data, EINA_TRUE);
}

static void
_apps_list_update(void)
{
   Evas_Object *entry, *btn_open, *apps;
   Elm_App_Client *app;
   Eina_List *l;

   apps = elm_table_child_get(table, APPS_COL, 0);
   elm_list_clear(apps);

   EINA_LIST_FOREACH(apps_list, l, app)
     {
        const char *app_package;
        eo_do(app, elm_app_client_package_get(&app_package));
        elm_list_item_append(apps, app_package, NULL, NULL, _app_clicked, app);
     }

   entry = elm_entry_add(apps);
   elm_entry_single_line_set(entry, EINA_TRUE);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, 0.5);

   btn_open = elm_button_add(apps);
   elm_object_text_set(btn_open, "Open");
   evas_object_smart_callback_add(btn_open, "clicked", _btn_app_open_cb, entry);
   elm_list_item_append(apps, NULL, entry, btn_open, _open_custom_app_cb, entry);

   elm_list_go(apps);
}

void
test_task_switcher(void *data EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Evas_Object *win, *apps;

   if (apps_list)
     {
        printf("Task switcher already open.\n");
        return;
     }

   win = elm_win_util_standard_add("task switcher", "Task switcher");
   elm_win_autodel_set(win, EINA_TRUE);

   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);

   table = elm_table_add(win);
   elm_win_resize_object_add(win, table);
   elm_table_padding_set(table, 0, 0);
   elm_table_homogeneous_set(table, EINA_TRUE);
   evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);

   _app_open("org.enlightenment.message");
   _app_open("org.enlightenment.phone");

   apps = elm_list_add(table);
   evas_object_size_hint_align_set(apps, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(apps);
   elm_table_pack(table, apps, APPS_COL, 0, 1, 1);
   _apps_list_update();

   evas_object_show(table);
   evas_object_resize(win, 1024, 768);
   evas_object_show(win);
}
