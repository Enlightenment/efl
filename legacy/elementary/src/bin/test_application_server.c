#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct {
   Evas_Object *win, *label;
   Eina_Stringshare *view_name;
} App_View_Context;

static Elm_App_Server *phone_server = NULL, *msg_server = NULL;

static void _text_update(App_View_Context *ctx, const char *state)
{
   char buffer[1024];

   if (!ctx->label)
     return;
   snprintf(buffer, sizeof(buffer), "%s - state=%s", ctx->view_name, state);
   elm_object_text_set(ctx->label, buffer);
}

static void
_window_create(App_View_Context *ctx)
{
   ctx->win = elm_win_util_standard_add("app_view", ctx->view_name);

   ctx->label = elm_label_add(ctx->win);
   evas_object_size_hint_weight_set(ctx->label, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_data_set(ctx->win, "label", ctx->label);

   _text_update(ctx, "alive");

   elm_win_resize_object_add(ctx->win, ctx->label);
   evas_object_show(ctx->label);

   evas_object_resize(ctx->win, 400, 300);
   evas_object_show(ctx->win);
}

static Eina_Bool
_close_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_View_Context *ctx = data;
   if (ctx->win)
     evas_object_del(ctx->win);
   eo_del(obj);
   return EINA_TRUE;
}

static Eina_Bool
_pause_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_View_Context *ctx = data;
   _text_update(ctx, "paused");
   return EINA_TRUE;
}

static Eina_Bool
_resume_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_View_Context *ctx = data;

   //shallow state
   if (!ctx->win)
     {
        _window_create(ctx);
        eo_do(obj, elm_app_server_view_window_set(ctx->win));
     }

   _text_update(ctx, "alive");
   return EINA_TRUE;
}

static Eina_Bool
_view_del_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_View_Context *ctx = data;

   if (ctx->win)
     evas_object_del(ctx->win);
   eo_do(obj, elm_app_server_view_window_set(NULL));
   eina_stringshare_del(ctx->view_name);
   free(ctx);
   return EINA_TRUE;
}

static Elm_App_Server_View *
_create_view_cb(Elm_App_Server *app_server, const Eina_Value *args EINA_UNUSED, Eina_Stringshare **error_name, Eina_Stringshare **error_message EINA_UNUSED)
{
   Elm_App_Server_View *view;
   const char *id, *pkg;
   App_View_Context *ctx;

   ctx = calloc(1, sizeof(App_View_Context));
   if (!ctx)
     {
        *error_name = eina_stringshare_add("No memory available");
        return NULL;
     }

   view = eo_add_custom(ELM_APP_SERVER_VIEW_CLASS, app_server,
                        elm_app_server_view_constructor(NULL));

   eo_do(view, elm_app_server_view_id_get(&id));
   eo_do(app_server, elm_app_server_package_get(&pkg));
   ctx->view_name = eina_stringshare_printf("%s %s", pkg, id);

   _window_create(ctx);

   eo_do(view, elm_app_server_view_title_set(ctx->view_name),
         elm_app_server_view_new_events_set(5),
         elm_app_server_view_window_set(ctx->win),
         elm_app_server_view_resume(),
         eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_CLOSED, _close_cb, ctx),
         eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_PAUSED, _pause_cb, ctx),
         eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_RESUMED, _resume_cb, ctx),
         eo_event_callback_add(EO_EV_DEL, _view_del_cb, ctx));

   return view;
}

static Eina_Bool
_terminate_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const char *title;

   printf("terminate cb\n");
   eo_do(obj, elm_app_server_save(),
         elm_app_server_title_get(&title));

   printf("Closing: %s\n", title);
   eo_unref(obj);
   return EINA_TRUE;
}

Elm_App_Server *
test_application_server_common(const char *pkg)
{
   Eina_Iterator *views_iter;
   Elm_App_Server_View *view;
   Elm_App_Server *server;

   server = eo_add_custom(ELM_APP_SERVER_CLASS, NULL,
                          elm_app_server_constructor(pkg, _create_view_cb));
   eo_do(server, elm_app_server_title_set(pkg),
         elm_app_server_views_get(&views_iter),
         eo_event_callback_add(ELM_APP_SERVER_EVENT_TERMINATE, _terminate_cb, NULL));

   //views create in shallow state
   EINA_ITERATOR_FOREACH(views_iter, view)
     {
        App_View_Context *ctx;
        const char *id;

        ctx = calloc(1, sizeof(App_View_Context));

        eo_do(view, elm_app_server_view_id_get(&id));
        ctx->view_name = eina_stringshare_printf("%s %s", pkg, id);

        eo_do(view,
              eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_CLOSED, _close_cb, ctx),
              eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_PAUSED, _pause_cb, ctx),
              eo_event_callback_add(ELM_APP_SERVER_VIEW_EV_RESUMED, _resume_cb, ctx),
              eo_event_callback_add(EO_EV_DEL, _view_del_cb, ctx));
     }
   eina_iterator_free(views_iter);

   return server;
}

static Eina_Bool
_server_del_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_App_Server **server = data;
   *server = NULL;
   return EINA_TRUE;
}

void
test_application_server_phone(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   if (phone_server)
     {
        printf("Phone already running\n");
        return;
     }
   printf("Starting phone\n");
   phone_server = test_application_server_common("org.enlightenment.phone");
   eo_do(phone_server, eo_event_callback_add(EO_EV_DEL, _server_del_cb, &phone_server));
}

void
test_application_server_message(void *data EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   if (msg_server)
     {
        printf("Message already running\n");
        return;
     }
   printf("Starting message\n");
   msg_server = test_application_server_common( "org.enlightenment.message");
   eo_do(msg_server, eo_event_callback_add(EO_EV_DEL, _server_del_cb, &msg_server));
}
