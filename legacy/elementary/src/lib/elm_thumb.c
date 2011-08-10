#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *self;
   Evas_Object *frame;
   Evas_Object *view;
   const char *file;
   const char *key;
   struct
     {
        int id;
        const char *file;
        const char *key;
#ifdef HAVE_ELEMENTARY_ETHUMB
        const char *thumb_path;
        const char *thumb_key;
        Ethumb_Exists *exists;

        Ethumb_Thumb_Format format;

        Eina_Bool retry : 1;
#endif
     } thumb;
   Ecore_Event_Handler *eeh;
   Elm_Thumb_Animation_Setting anim_setting;
   Eina_Bool on_hold : 1;
   Eina_Bool is_video : 1;
   Eina_Bool was_video : 1;
   Eina_Bool edit : 1;
};

static const char *widtype = NULL;

static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_GENERATE_ERROR[] = "generate,error";
static const char SIG_GENERATE_START[] = "generate,start";
static const char SIG_GENERATE_STOP[] = "generate,stop";
static const char SIG_LOAD_ERROR[] = "load,error";
static const char SIG_PRESS[] = "press";

static const Evas_Smart_Cb_Description _signals[] =
{
     {SIG_CLICKED, ""},
     {SIG_CLICKED_DOUBLE, ""},
     {SIG_GENERATE_ERROR, ""},
     {SIG_GENERATE_START, ""},
     {SIG_GENERATE_STOP, ""},
     {SIG_LOAD_ERROR, ""},
     {SIG_PRESS, ""},
     {NULL, NULL}
};

#define EDJE_SIGNAL_GENERATE_START "elm,thumb,generate,start"
#define EDJE_SIGNAL_GENERATE_STOP "elm,thumb,generate,stop"
#define EDJE_SIGNAL_GENERATE_ERROR "elm,thumb,generate,error"
#define EDJE_SIGNAL_LOAD_ERROR "elm,thumb,load,error"
#define EDJE_SIGNAL_PULSE_START "elm,state,pulse,start"
#define EDJE_SIGNAL_PULSE_STOP "elm,state,pulse,stop"

struct _Ethumb_Client *_elm_ethumb_client = NULL;
Eina_Bool _elm_ethumb_connected = EINA_FALSE;
#ifdef HAVE_ELEMENTARY_ETHUMB
static Eina_List *retry = NULL;
static int pending_request = 0;

static void _thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread,
                          Eina_Bool exists, void *data);

#endif

EAPI int ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

#ifdef HAVE_ELEMENTARY_ETHUMB
   if (wd->thumb.id >= 0)
     {
        ethumb_client_generate_cancel(_elm_ethumb_client, wd->thumb.id,
                                      NULL, NULL, NULL);
        wd->thumb.id = -1;
     }
   if (wd->thumb.exists)
     {
        ethumb_client_thumb_exists_cancel(wd->thumb.exists, _thumb_exists, wd);
        wd->thumb.exists = NULL;
     }
   if (wd->thumb.retry)
     {
        retry = eina_list_remove(retry, wd);
        wd->thumb.retry = EINA_FALSE;
     }

   eina_stringshare_del(wd->thumb.thumb_path);
   eina_stringshare_del(wd->thumb.thumb_key);
#endif

   eina_stringshare_del(wd->file);
   eina_stringshare_del(wd->key);
   if (wd->eeh) ecore_event_handler_del(wd->eeh);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_object_set(obj, wd->frame, "thumb", "base",
                         elm_widget_style_get(obj));
}

#ifdef HAVE_ELEMENTARY_ETHUMB
static void
_mouse_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1)
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     wd->on_hold = EINA_TRUE;
   else
     wd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(wd->self, SIG_CLICKED_DOUBLE, NULL);
   else
     evas_object_smart_callback_call(wd->self, SIG_PRESS, NULL);
}

static void
_mouse_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1)
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     wd->on_hold = EINA_TRUE;
   else
     wd->on_hold = EINA_FALSE;
   if (!wd->on_hold)
     evas_object_smart_callback_call(wd->self, SIG_CLICKED, NULL);
   wd->on_hold = EINA_FALSE;
}

static void
_thumb_ready(Widget_Data *wd, const char *thumb_path, const char *thumb_key)
{
   Evas_Coord mw, mh;
   Evas_Coord aw, ah;

   evas_object_image_size_get(wd->view, &aw, &ah);
   evas_object_size_hint_aspect_set(wd->view,
				    EVAS_ASPECT_CONTROL_BOTH,
				    aw, ah);
   edje_object_part_swallow(wd->frame, "elm.swallow.content", wd->view);
   edje_object_size_min_get(wd->frame, &mw, &mh);
   edje_object_size_min_restricted_calc(wd->frame, &mw, &mh, mw, mh);
   evas_object_size_hint_min_set(wd->self, mw, mh);
   eina_stringshare_replace(&(wd->thumb.file), thumb_path);
   eina_stringshare_replace(&(wd->thumb.key), thumb_key);
   edje_object_signal_emit(wd->frame, EDJE_SIGNAL_GENERATE_STOP, "elm");
   evas_object_smart_callback_call(wd->self, SIG_GENERATE_STOP, NULL);
}

static void
_thumb_loaded(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;
   const char *thumb_path;
   const char *thumb_key;

   evas_object_image_file_get(wd->view, &thumb_path, &thumb_key);

   _thumb_ready(wd, thumb_path, thumb_key);
}

/* As we do use stat to check if a thumbnail is available, it's possible
   that we end up accessing before the file is completly written on disk.
   By retrying each time a thumbnail is finished we should be fine or not.
*/
static Eina_Bool
_retry_thumb(Widget_Data *wd)
{
   int r;

   if ((wd->is_video) && (wd->thumb.format == ETHUMB_THUMB_EET))
     {
        edje_object_file_set(wd->view, NULL, NULL);
        if (!edje_object_file_set(wd->view,
                                  wd->thumb.thumb_path,
                                  "movie/thumb"))
          {
             if (pending_request == 0)
               ERR("could not set file=%s key=%s for %s",
                   wd->thumb.thumb_path,
                   wd->thumb.thumb_key,
                   wd->file);
             goto view_err;
          }
     }
   else
     {
        evas_object_image_file_set(wd->view, NULL, NULL);
        evas_object_image_file_set(wd->view,
                                   wd->thumb.thumb_path,
                                   wd->thumb.thumb_key);
        r = evas_object_image_load_error_get(wd->view);
        if (r != EVAS_LOAD_ERROR_NONE)
          {
             if (pending_request == 0)
               ERR("%s: %s", wd->thumb.thumb_path, evas_load_error_str(r));
             goto view_err;
          }

        evas_object_event_callback_add(wd->view,
                                       EVAS_CALLBACK_IMAGE_PRELOADED,
                                       _thumb_loaded, wd);
        evas_object_image_preload(wd->view, EINA_TRUE);
        return EINA_TRUE;
     }

   _thumb_ready(wd, wd->thumb.thumb_path, wd->thumb.thumb_key);

   eina_stringshare_del(wd->thumb.thumb_path);
   wd->thumb.thumb_path = NULL;

   eina_stringshare_del(wd->thumb.thumb_key);
   wd->thumb.thumb_key = NULL;

   return EINA_TRUE;

 view_err:
   return EINA_FALSE;
}

static void
_finished_thumb(Widget_Data *wd,
                const char *thumb_path,
                const char *thumb_key)
{
   Eina_List *l, *ll;
   Evas *evas;
   int r;

   evas = evas_object_evas_get(wd->self);
   if ((wd->view) && (wd->is_video ^ wd->was_video))
     {
        evas_object_del(wd->view);
        wd->view = NULL;
     }
   wd->was_video = wd->is_video;

   if ((wd->is_video) &&
       (ethumb_client_format_get(_elm_ethumb_client) == ETHUMB_THUMB_EET))
     {
        if (!wd->view)
          {
             wd->view = edje_object_add(evas);
             if (!wd->view)
               {
                  ERR("could not create edje object");
                  goto err;
               }
             elm_widget_sub_object_add(wd->self, wd->view);
          }

        if (!edje_object_file_set(wd->view, thumb_path, thumb_key))
          {
             wd->thumb.thumb_path = eina_stringshare_ref(thumb_path);
             wd->thumb.thumb_key = eina_stringshare_ref(thumb_key);
             wd->thumb.format = ethumb_client_format_get(_elm_ethumb_client);
             wd->thumb.retry = EINA_TRUE;

             retry = eina_list_append(retry, wd);
             return ;
          }
     }
   else
     {
        if (!wd->view)
          {
             wd->view = evas_object_image_filled_add(evas);
             if (!wd->view)
               {
                  ERR("could not create image object");
                  goto err;
               }
             evas_object_event_callback_add(wd->view,
                                            EVAS_CALLBACK_IMAGE_PRELOADED,
                                            _thumb_loaded, wd);
             elm_widget_sub_object_add(wd->self, wd->view);
             evas_object_hide(wd->view);
          }

        evas_object_image_file_set(wd->view, thumb_path, thumb_key);
        r = evas_object_image_load_error_get(wd->view);
        if (r != EVAS_LOAD_ERROR_NONE)
          {
             WRN("%s: %s", thumb_path, evas_load_error_str(r));
             wd->thumb.thumb_path = eina_stringshare_ref(thumb_path);
             wd->thumb.thumb_key = eina_stringshare_ref(thumb_key);
             wd->thumb.format = ethumb_client_format_get(_elm_ethumb_client);
             wd->thumb.retry = EINA_TRUE;

             retry = eina_list_append(retry, wd);
             return ;
          }

        evas_object_image_preload(wd->view, 0);
        return ;
     }

   _thumb_ready(wd, thumb_path, thumb_key);

   EINA_LIST_FOREACH_SAFE(retry, l, ll, wd)
     if (_retry_thumb(wd))
       retry = eina_list_remove_list(retry, l);

   if (pending_request == 0)
     EINA_LIST_FREE(retry, wd)
       {
          eina_stringshare_del(wd->thumb.thumb_path);
          wd->thumb.thumb_path = NULL;

          eina_stringshare_del(wd->thumb.thumb_key);
          wd->thumb.thumb_key = NULL;

          evas_object_del(wd->view);
          wd->view = NULL;

          edje_object_signal_emit(wd->frame, EDJE_SIGNAL_LOAD_ERROR, "elm");
          evas_object_smart_callback_call(wd->self, SIG_LOAD_ERROR, NULL);
       }

   return;

err:
   edje_object_signal_emit(wd->frame, EDJE_SIGNAL_LOAD_ERROR, "elm");
   evas_object_smart_callback_call(wd->self, SIG_LOAD_ERROR, NULL);
}

static void
_finished_thumb_cb(void *data, Ethumb_Client *c __UNUSED__, int id, const char *file, const char *key, const char *thumb_path, const char *thumb_key, Eina_Bool success)
{
   Widget_Data *wd = data;

   EINA_SAFETY_ON_FALSE_RETURN(wd->thumb.id == id);
   wd->thumb.id = -1;

   pending_request--;

   edje_object_signal_emit(wd->frame, EDJE_SIGNAL_PULSE_STOP, "elm");

   if (success)
     {
        _finished_thumb(wd, thumb_path, thumb_key);
        return;
     }

   ERR("could not generate thumbnail for %s (key: %s)", file, key ? key : "");
   edje_object_signal_emit(wd->frame, EDJE_SIGNAL_GENERATE_ERROR, "elm");
   evas_object_smart_callback_call(wd->self, SIG_GENERATE_ERROR, NULL);
}

static void
_thumb_exists(Ethumb_Client *client __UNUSED__, Ethumb_Exists *thread,
              Eina_Bool exists, void *data)
{
   Widget_Data *wd = data;

   if (ethumb_client_thumb_exists_check(thread))
     return ;

   wd->thumb.exists = NULL;

   if (exists)
     {
        const char *thumb_path, *thumb_key;

        pending_request--;

        wd->thumb.id = -1;
        ethumb_client_thumb_path_get(_elm_ethumb_client, &thumb_path,
                                     &thumb_key);
        _finished_thumb(wd, thumb_path, thumb_key);
        return;
     }
   else if ((wd->thumb.id = ethumb_client_generate
             (_elm_ethumb_client, _finished_thumb_cb, wd, NULL)) != -1)
     {
        edje_object_signal_emit(wd->frame, EDJE_SIGNAL_PULSE_START, "elm");
        edje_object_signal_emit(wd->frame, EDJE_SIGNAL_GENERATE_START, "elm");
        evas_object_smart_callback_call(wd->self, SIG_GENERATE_START, NULL);
     }
   else
     {
        pending_request--;

        wd->thumb.id = -1;
        edje_object_signal_emit(wd->frame, EDJE_SIGNAL_GENERATE_ERROR, "elm");
        evas_object_smart_callback_call(wd->self, SIG_GENERATE_ERROR, NULL);
     }

}

static void
_thumb_apply(Widget_Data *wd)
{
   if (wd->thumb.id > 0)
     {
        ethumb_client_generate_cancel
           (_elm_ethumb_client, wd->thumb.id, NULL, NULL, NULL);
        wd->thumb.id = -1;
     }

   if (wd->thumb.exists)
     {
        ethumb_client_thumb_exists_cancel(wd->thumb.exists, _thumb_exists, wd);
        wd->thumb.exists = NULL;
     }

   if (wd->thumb.retry)
     {
        retry = eina_list_remove(retry, wd);
        wd->thumb.retry = EINA_FALSE;
     }

   if (!wd->file) return;

   pending_request++;
   ethumb_client_file_set(_elm_ethumb_client, wd->file, wd->key);
   wd->thumb.exists = ethumb_client_thumb_exists(_elm_ethumb_client,
                                                 _thumb_exists,
                                                 wd);
}

static Eina_Bool
_thumb_apply_cb(void *data, int type __UNUSED__, void *ev __UNUSED__)
{
   _thumb_apply(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_thumb_show(Widget_Data *wd)
{
   evas_object_show(wd->frame);

   if (elm_thumb_ethumb_client_connected())
     {
        _thumb_apply(wd);
        return;
     }

   if (!wd->eeh)
     wd->eeh = ecore_event_handler_add(ELM_ECORE_EVENT_ETHUMB_CONNECT,
                                       _thumb_apply_cb, wd);
}

static void
_thumb_show_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _thumb_show(data);
}

static void
_thumb_hide_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;

   evas_object_hide(wd->frame);

   if (wd->thumb.id >= 0)
     {
        ethumb_client_generate_cancel
           (_elm_ethumb_client, wd->thumb.id, NULL, NULL, NULL);
        wd->thumb.id = -1;

        edje_object_signal_emit(wd->frame, EDJE_SIGNAL_GENERATE_STOP, "elm");
        evas_object_smart_callback_call(wd->self, SIG_GENERATE_STOP, NULL);
     }

   if (wd->thumb.exists)
     {
        ethumb_client_thumb_exists_cancel(wd->thumb.exists, _thumb_exists, wd);
        wd->thumb.exists = NULL;
     }

   if (wd->thumb.retry)
     {
        retry = eina_list_remove(retry, wd);
        wd->thumb.retry = EINA_FALSE;
     }

   if (wd->eeh)
     {
        ecore_event_handler_del(wd->eeh);
        wd->eeh = NULL;
     }
}

#endif

#ifdef ELM_ETHUMB
static int _elm_need_ethumb = 0;

static void _on_die_cb(void *, Ethumb_Client *);

static void
_connect_cb(void *data __UNUSED__, Ethumb_Client *c, Eina_Bool success)
{
   if (success)
     {
        ethumb_client_on_server_die_callback_set(c, _on_die_cb, NULL, NULL);
        _elm_ethumb_connected = EINA_TRUE;
        ecore_event_add(ELM_ECORE_EVENT_ETHUMB_CONNECT, NULL, NULL, NULL);
     }
   else
     _elm_ethumb_client = NULL;
}

static void
_on_die_cb(void *data __UNUSED__, Ethumb_Client *c __UNUSED__)
{
   ethumb_client_disconnect(_elm_ethumb_client);
   _elm_ethumb_client = NULL;
   _elm_ethumb_connected = EINA_FALSE;
   _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
}
#endif

void
_elm_unneed_ethumb(void)
{
#ifdef ELM_ETHUMB
   if (--_elm_need_ethumb) return;

   ethumb_client_disconnect(_elm_ethumb_client);
   _elm_ethumb_client = NULL;
   ethumb_client_shutdown();
   ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;
#endif
}

static Eina_Bool
_elm_thumb_dropcb(void *data __UNUSED__, Evas_Object *o, Elm_Selection_Data *drop)
{
   if ((!o) || (!drop) || (!drop->data)) return EINA_FALSE;
   elm_thumb_file_set(o, drop->data, NULL);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_need_ethumb(void)
{
#ifdef ELM_ETHUMB
   if (_elm_need_ethumb++) return EINA_TRUE;
   ELM_ECORE_EVENT_ETHUMB_CONNECT = ecore_event_type_new();
   ethumb_client_init();
   _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI Evas_Object *
elm_thumb_add(Evas_Object *parent)
{
   Evas *e;
   Widget_Data *wd;
   Evas_Object *obj;
   Evas_Coord minw, minh;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "thumb");
   elm_widget_type_set(obj, "thumb");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->frame = edje_object_add(e);
   _elm_theme_object_set(obj, wd->frame, "thumb", "base", "default");
   elm_widget_resize_object_set(obj, wd->frame);

   edje_object_size_min_calc(obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   wd->self = obj;
   wd->view = NULL;
   wd->file = NULL;
   wd->key = NULL;
   wd->eeh = NULL;
   wd->thumb.id = -1;
   wd->on_hold = EINA_FALSE;
   wd->is_video = EINA_FALSE;
   wd->was_video = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_ETHUMB
   wd->thumb.thumb_path = NULL;
   wd->thumb.thumb_key = NULL;
   wd->thumb.exists = NULL;
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW,
                                  _thumb_show_cb, wd);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE,
                                  _thumb_hide_cb, wd);
#endif

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

EAPI void
elm_thumb_reload(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   eina_stringshare_replace(&(wd->thumb.file), NULL);
   eina_stringshare_replace(&(wd->thumb.key), NULL);

#ifdef HAVE_ELEMENTARY_ETHUMB
   if (evas_object_visible_get(obj))
     _thumb_show(wd);
#endif
}

EAPI void
elm_thumb_file_set(Evas_Object *obj, const char *file, const char *key)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_Bool file_replaced, key_replaced;
   Widget_Data *wd = elm_widget_data_get(obj);

   file_replaced = eina_stringshare_replace(&(wd->file), file);
   key_replaced = eina_stringshare_replace(&(wd->key), key);

   if (file_replaced)
     {
        int prefix_size;
        const char **ext, *ptr;
        static const char *extensions[] =
          {
             ".avi", ".mp4", ".ogv", ".mov", ".mpg", ".wmv", NULL
          };

        prefix_size = eina_stringshare_strlen(wd->file) - 4;
        if (prefix_size >= 0)
          {
             ptr = wd->file + prefix_size;
             wd->is_video = EINA_FALSE;
             for (ext = extensions; *ext; ext++)
               if (!strcasecmp(ptr, *ext))
                 {
                    wd->is_video = EINA_TRUE;
                    break;
                 }
          }
     }

   eina_stringshare_replace(&(wd->thumb.file), NULL);
   eina_stringshare_replace(&(wd->thumb.key), NULL);

#ifdef HAVE_ELEMENTARY_ETHUMB
   if (((file_replaced) || (key_replaced)) && (evas_object_visible_get(obj)))
     _thumb_show(wd);
#endif
}

EAPI void
elm_thumb_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (file)
     *file = wd->file;
   if (key)
     *key = wd->key;
}

EAPI void
elm_thumb_path_get(const Evas_Object *obj, const char **file, const char **key)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (file)
     *file = wd->thumb.file;
   if (key)
     *key = wd->thumb.key;
}

EAPI void
elm_thumb_animate_set(Evas_Object *obj, Elm_Thumb_Animation_Setting setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_TRUE_RETURN(setting >= ELM_THUMB_ANIMATION_LAST);

   wd->anim_setting = setting;
   if (setting == ELM_THUMB_ANIMATION_LOOP)
     edje_object_signal_emit(wd->view, "animate_loop", "");
   else if (setting == ELM_THUMB_ANIMATION_START)
     edje_object_signal_emit(wd->view, "animate", "");
   else if (setting == ELM_THUMB_ANIMATION_STOP)
     edje_object_signal_emit(wd->view, "animate_stop", "");
}

EAPI Elm_Thumb_Animation_Setting
elm_thumb_animate_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_THUMB_ANIMATION_LAST;
   Widget_Data *wd = elm_widget_data_get(obj);

   return wd->anim_setting;
}

EAPI void *
elm_thumb_ethumb_client_get(void)
{
   return _elm_ethumb_client;
}

EAPI Eina_Bool
elm_thumb_ethumb_client_connected(void)
{
   return _elm_ethumb_connected;
}

EAPI Eina_Bool
elm_thumb_editable_set(Evas_Object *obj, Eina_Bool edit)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   edit = !!edit;
   if (wd->edit == edit) return EINA_TRUE;

   wd->edit = edit;
   if (wd->edit)
     elm_drop_target_add(obj, ELM_SEL_FORMAT_IMAGE,
                         _elm_thumb_dropcb, obj);
   else
     elm_drop_target_del(obj);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_thumb_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->edit;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
