#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_thumb.h"

EAPI Eo_Op ELM_OBJ_THUMB_BASE_ID = EO_NOOP;

#define MY_CLASS_NAME "Elm_Thumb"
#define MY_CLASS_NAME_LEGACY "elm_thumb"

#define MY_CLASS ELM_OBJ_THUMB_CLASS

static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_GENERATE_ERROR[] = "generate,error";
static const char SIG_GENERATE_START[] = "generate,start";
static const char SIG_GENERATE_STOP[] = "generate,stop";
static const char SIG_LOAD_ERROR[] = "load,error";
static const char SIG_PRESS[] = "press";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
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
#define EDJE_SIGNAL_GENERATE_STOP  "elm,thumb,generate,stop"
#define EDJE_SIGNAL_GENERATE_ERROR "elm,thumb,generate,error"
#define EDJE_SIGNAL_LOAD_ERROR     "elm,thumb,load,error"
#define EDJE_SIGNAL_PULSE_START    "elm,state,pulse,start"
#define EDJE_SIGNAL_PULSE_STOP     "elm,state,pulse,stop"

static struct _Ethumb_Client *_elm_ethumb_client = NULL;
static Eina_Bool _elm_ethumb_connected = EINA_FALSE;

static Eina_List *retry = NULL;
static int pending_request = 0;

EAPI int ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;

static void
_mouse_down_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info)
{
   ELM_THUMB_DATA_GET(data, sd);
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(obj, SIG_CLICKED_DOUBLE, NULL);
   else
     evas_object_smart_callback_call(obj, SIG_PRESS, NULL);
}

static void
_mouse_up_cb(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj,
             void *event_info)
{
   ELM_THUMB_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (!sd->on_hold)
     evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);

   sd->on_hold = EINA_FALSE;
}

static void
_thumb_ready_inform(Elm_Thumb_Smart_Data *sd,
                    const char *thumb_path,
                    const char *thumb_key)
{
   Evas_Coord mw, mh;
   Evas_Coord aw, ah;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   if ((sd->is_video) && (sd->thumb.format == ETHUMB_THUMB_EET))
     {
        edje_object_size_min_get(sd->view, &mw, &mh);
        edje_object_size_min_restricted_calc
          (sd->view, &mw, &mh, mw, mh);
        evas_object_size_hint_min_set(sd->view, mw, mh);
     }
   else
     {
        evas_object_image_size_get(sd->view, &aw, &ah);
        evas_object_size_hint_aspect_set
          (sd->view, EVAS_ASPECT_CONTROL_BOTH, aw, ah);
     }

   edje_object_part_swallow(wd->resize_obj, "elm.swallow.content", sd->view);
   eina_stringshare_replace(&(sd->thumb.file), thumb_path);
   eina_stringshare_replace(&(sd->thumb.key), thumb_key);
   edje_object_signal_emit
     (wd->resize_obj, EDJE_SIGNAL_PULSE_STOP, "elm");
   edje_object_signal_emit(wd->resize_obj, EDJE_SIGNAL_GENERATE_STOP, "elm");
   evas_object_smart_callback_call
     (sd->obj, SIG_GENERATE_STOP, NULL);
}

static void
_on_thumb_preloaded(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   ELM_THUMB_DATA_GET(data, sd);
   const char *thumb_path;
   const char *thumb_key;

   evas_object_image_file_get(sd->view, &thumb_path, &thumb_key);

   _thumb_ready_inform(sd, thumb_path, thumb_key);
}

/* As we do use stat to check if a thumbnail is available, it's
 * possible that we end up accessing it before the file is completly
 * written on disk. By retrying each time a thumbnail is finished we
 * should be fine or not.
 */
static Eina_Bool
_thumb_retry(Elm_Thumb_Smart_Data *sd)
{
   int r;

   if ((sd->is_video) && (sd->thumb.format == ETHUMB_THUMB_EET))
     {
        edje_object_file_set(sd->view, NULL, NULL);
        if (!edje_object_file_set
              (sd->view, sd->thumb.thumb_path, "movie/thumb"))
          {
             if (pending_request == 0)
               ERR("could not set file=%s key=%s for %s",
                   sd->thumb.thumb_path,
                   sd->thumb.thumb_key,
                   sd->file);
             goto view_err;
          }
        /* FIXME: Do we want to reload a thumbnail when the file
           changes? I think not at the moment, as this may increase
           the presure on the system a lot when using it in a file
           browser */
     }
   else
     {
        evas_object_image_file_set(sd->view, NULL, NULL);
        evas_object_image_file_set
          (sd->view, sd->thumb.thumb_path, sd->thumb.thumb_key);
        r = evas_object_image_load_error_get(sd->view);
        if (r != EVAS_LOAD_ERROR_NONE)
          {
             if (pending_request == 0)
               ERR("%s: %s", sd->thumb.thumb_path, evas_load_error_str(r));
             goto view_err;
          }

        evas_object_event_callback_add
          (sd->view, EVAS_CALLBACK_IMAGE_PRELOADED, _on_thumb_preloaded, sd->obj);
        evas_object_image_preload(sd->view, EINA_TRUE);

        return EINA_TRUE;
     }

   _thumb_ready_inform(sd, sd->thumb.thumb_path, sd->thumb.thumb_key);

   ELM_SAFE_FREE(sd->thumb.thumb_path, eina_stringshare_del);
   ELM_SAFE_FREE(sd->thumb.thumb_key, eina_stringshare_del);

   return EINA_TRUE;

view_err:
   return EINA_FALSE;
}

static void
_thumb_finish(Elm_Thumb_Smart_Data *sd,
              const char *thumb_path,
              const char *thumb_key)
{
   Eina_List *l, *ll;
   Evas *evas;
   int r;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   evas = evas_object_evas_get(sd->obj);
   if ((sd->view) && (sd->is_video ^ sd->was_video))
     {
        evas_object_del(sd->view);
        sd->view = NULL;
     }
   sd->was_video = sd->is_video;

   if ((sd->is_video) &&
       (ethumb_client_format_get(_elm_ethumb_client) == ETHUMB_THUMB_EET))
     {
        if (!sd->view)
          {
             sd->view = edje_object_add(evas);
             if (!sd->view)
               {
                  ERR("could not create edje object");
                  goto err;
               }
          }

        if (!edje_object_file_set(sd->view, thumb_path, thumb_key))
          {
             sd->thumb.thumb_path = eina_stringshare_ref(thumb_path);
             sd->thumb.thumb_key = eina_stringshare_ref(thumb_key);
             sd->thumb.format = ethumb_client_format_get(_elm_ethumb_client);
             sd->thumb.retry = EINA_TRUE;

             retry = eina_list_append(retry, sd);
             eo_data_ref(sd->obj, NULL);
             return;
          }
     }
   else
     {
        if (!sd->view)
          {
             sd->view = evas_object_image_filled_add(evas);
             if (!sd->view)
               {
                  ERR("could not create image object");
                  goto err;
               }
             evas_object_event_callback_add
               (sd->view, EVAS_CALLBACK_IMAGE_PRELOADED, _on_thumb_preloaded,
               sd->obj);
             evas_object_hide(sd->view);
          }

        evas_object_image_file_set(sd->view, thumb_path, thumb_key);
        r = evas_object_image_load_error_get(sd->view);
        if (r != EVAS_LOAD_ERROR_NONE)
          {
             WRN("%s: %s", thumb_path, evas_load_error_str(r));
             sd->thumb.thumb_path = eina_stringshare_ref(thumb_path);
             sd->thumb.thumb_key = eina_stringshare_ref(thumb_key);
             sd->thumb.format = ethumb_client_format_get(_elm_ethumb_client);
             sd->thumb.retry = EINA_TRUE;

             retry = eina_list_append(retry, sd);
             eo_data_ref(sd->obj, NULL);
             return;
          }

        evas_object_image_preload(sd->view, EINA_FALSE);
        return;
     }

   _thumb_ready_inform(sd, thumb_path, thumb_key);

   EINA_LIST_FOREACH_SAFE(retry, l, ll, sd)
     {
        if (_thumb_retry(sd))
          {
             retry = eina_list_remove_list(retry, l);
             eo_data_unref(sd->obj, sd);
          }

     }

   if (pending_request == 0)
     EINA_LIST_FREE(retry, sd)
       {
          eo_data_unref(sd->obj, sd);
          eina_stringshare_del(sd->thumb.thumb_path);
          sd->thumb.thumb_path = NULL;

          eina_stringshare_del(sd->thumb.thumb_key);
          sd->thumb.thumb_key = NULL;

          evas_object_del(sd->view);
          sd->view = NULL;

          wd = eo_data_scope_get(sd->obj, ELM_OBJ_WIDGET_CLASS);
          edje_object_signal_emit
             (wd->resize_obj, EDJE_SIGNAL_LOAD_ERROR, "elm");
          evas_object_smart_callback_call
            (sd->obj, SIG_LOAD_ERROR, NULL);
       }

   return;

err:
   edje_object_signal_emit(wd->resize_obj, EDJE_SIGNAL_LOAD_ERROR, "elm");
   evas_object_smart_callback_call
     (sd->obj, SIG_LOAD_ERROR, NULL);
}

static void
_on_ethumb_thumb_done(Ethumb_Client *client __UNUSED__,
                      const char *thumb_path,
                      const char *thumb_key,
                      void *data)
{
   ELM_THUMB_DATA_GET(data, sd);

   if (EINA_UNLIKELY(!sd->thumb.request))
     {
        ERR("Something odd happened with a thumbnail request");
        return;
     }

   pending_request--;
   sd->thumb.request = NULL;

   _thumb_finish(sd, thumb_path, thumb_key);
}

static void
_on_ethumb_thumb_error(Ethumb_Client *client __UNUSED__,
                       void *data)
{
   ELM_THUMB_DATA_GET(data, sd);

   if (EINA_UNLIKELY(!sd->thumb.request))
     {
        ERR("Something odd happened with a thumbnail request");
        return;
     }
   evas_object_event_callback_del_full
      (sd->view, EVAS_CALLBACK_IMAGE_PRELOADED, _on_thumb_preloaded,
       sd);

   pending_request--;
   sd->thumb.request = NULL;

   ERR("could not generate thumbnail for %s (key: %s)",
       sd->thumb.file, sd->thumb.key ? sd->thumb.key : "");

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   edje_object_signal_emit(wd->resize_obj, EDJE_SIGNAL_GENERATE_ERROR, "elm");
   evas_object_smart_callback_call
     (sd->obj, SIG_GENERATE_ERROR, NULL);
}

static void
_thumb_start(Elm_Thumb_Smart_Data *sd)
{
   if (sd->thumb.aspect)
     ethumb_client_aspect_set(_elm_ethumb_client, sd->thumb.aspect);
   if (sd->thumb.size)
     ethumb_client_fdo_set(_elm_ethumb_client, sd->thumb.size);
   if (sd->thumb.format)
     ethumb_client_format_set(_elm_ethumb_client, sd->thumb.format);
   if (sd->thumb.orient)
     ethumb_client_orientation_set(_elm_ethumb_client, sd->thumb.orient);
   if (sd->thumb.tw && sd->thumb.th)
     ethumb_client_size_set(_elm_ethumb_client, sd->thumb.tw, sd->thumb.th);
   if (sd->thumb.cropx && sd->thumb.cropy)
     ethumb_client_crop_align_set(_elm_ethumb_client, sd->thumb.cropx, sd->thumb.cropy);
   if (sd->thumb.quality)
     ethumb_client_quality_set(_elm_ethumb_client, sd->thumb.quality);
   if (sd->thumb.compress)
     ethumb_client_compress_set(_elm_ethumb_client, sd->thumb.compress);
   if (sd->thumb.request)
     {
        ethumb_client_thumb_async_cancel(_elm_ethumb_client, sd->thumb.request);
        sd->thumb.request = NULL;
     }
   if (sd->thumb.retry)
     {
        retry = eina_list_remove(retry, sd);
        eo_data_unref(sd->obj, sd);
        sd->thumb.retry = EINA_FALSE;
     }

   if (!sd->file) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);
   edje_object_signal_emit(wd->resize_obj, EDJE_SIGNAL_PULSE_START, "elm");
   edje_object_signal_emit(wd->resize_obj, EDJE_SIGNAL_GENERATE_START, "elm");
   evas_object_smart_callback_call
      (sd->obj, SIG_GENERATE_START, NULL);

   pending_request++;
   ethumb_client_file_set(_elm_ethumb_client, sd->file, sd->key);
   sd->thumb.request = ethumb_client_thumb_async_get
       (_elm_ethumb_client, _on_ethumb_thumb_done, _on_ethumb_thumb_error, sd->obj);
}

static Eina_Bool
_thumbnailing_available_cb(void *data,
                           int type __UNUSED__,
                           void *ev __UNUSED__)
{
   ELM_THUMB_DATA_GET(data, sd);
   _thumb_start(sd);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool _elm_need_ethumb = EINA_FALSE;
static void _on_die_cb(void *, Ethumb_Client *);

static void
_connect_cb(void *data __UNUSED__,
            Ethumb_Client *c,
            Eina_Bool success)
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
_on_die_cb(void *data __UNUSED__,
           Ethumb_Client *c __UNUSED__)
{
   if (_elm_ethumb_client)
     {
        ethumb_client_disconnect(_elm_ethumb_client);
        _elm_ethumb_client = NULL;
     }
   _elm_ethumb_connected = EINA_FALSE;
   if (pending_request > 0)
     _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
}

static void
_thumb_show(Elm_Thumb_Smart_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);
   evas_object_show(wd->resize_obj);

   if (!_elm_ethumb_client)
     _elm_ethumb_client = ethumb_client_connect(_connect_cb, NULL, NULL);
   else if (elm_thumb_ethumb_client_connected_get())
     {
        _thumb_start(sd);
        return;
     }

   if (!sd->eeh)
     sd->eeh = ecore_event_handler_add
         (ELM_ECORE_EVENT_ETHUMB_CONNECT, _thumbnailing_available_cb, sd->obj);
}

static void
_elm_thumb_smart_show(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Thumb_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   _thumb_show(sd);
}

static void
_elm_thumb_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Thumb_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

   if (sd->thumb.request)
     {
        ethumb_client_thumb_async_cancel(_elm_ethumb_client, sd->thumb.request);
        sd->thumb.request = NULL;

        edje_object_signal_emit
           (wd->resize_obj, EDJE_SIGNAL_GENERATE_STOP, "elm");
        evas_object_smart_callback_call
           (sd->obj, SIG_GENERATE_STOP, NULL);
     }

   if (sd->thumb.retry)
     {
        retry = eina_list_remove(retry, sd);
        eo_data_unref(sd->obj, sd);
        sd->thumb.retry = EINA_FALSE;
     }

   ELM_SAFE_FREE(sd->eeh, ecore_event_handler_del);
}

void
_elm_unneed_ethumb(void)
{
   if (!_elm_need_ethumb) return;
   _elm_need_ethumb = EINA_FALSE;

   if (_elm_ethumb_client)
     {
        ethumb_client_disconnect(_elm_ethumb_client);
        _elm_ethumb_client = NULL;
     }
   ethumb_client_shutdown();
   ELM_ECORE_EVENT_ETHUMB_CONNECT = 0;
}

static Eina_Bool
_elm_thumb_dnd_cb(void *data __UNUSED__,
                  Evas_Object *o,
                  Elm_Selection_Data *drop)
{
   if ((!o) || (!drop) || (!drop->data)) return EINA_FALSE;
   elm_thumb_file_set(o, drop->data, NULL);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_need_ethumb(void)
{
   if (_elm_need_ethumb) return EINA_TRUE;
   _elm_need_ethumb = EINA_TRUE;

   ELM_ECORE_EVENT_ETHUMB_CONNECT = ecore_event_type_new();
   ethumb_client_init();

   return EINA_TRUE;
}

static void
_elm_thumb_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "thumb", "base", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

static void
_elm_thumb_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Thumb_Smart_Data *sd = _pd;

   if (sd->thumb.request)
     {
        ethumb_client_thumb_async_cancel(_elm_ethumb_client, sd->thumb.request);
        sd->thumb.request = NULL;
     }
   if (sd->thumb.retry)
     {
        retry = eina_list_remove(retry, sd);
        eo_data_unref(sd->obj, sd);
        sd->thumb.retry = EINA_FALSE;
     }
   evas_object_event_callback_del_full
      (sd->view, EVAS_CALLBACK_IMAGE_PRELOADED, _on_thumb_preloaded,
       sd);

   eina_stringshare_del(sd->thumb.thumb_path);
   eina_stringshare_del(sd->thumb.thumb_key);

   eina_stringshare_del(sd->file);
   eina_stringshare_del(sd->key);

   if (sd->eeh) ecore_event_handler_del(sd->eeh);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_thumb_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set("Elm_Thumb"),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   Elm_Thumb_Smart_Data *sd = _pd;
   sd->obj = obj;
}

EAPI void
elm_thumb_reload(Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_reload());
}

static void
_reload(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Thumb_Smart_Data *sd = _pd;

   eina_stringshare_replace(&(sd->thumb.file), NULL);
   eina_stringshare_replace(&(sd->thumb.key), NULL);

   if (evas_object_visible_get(obj))
     _thumb_show(sd);
}

EAPI void
elm_thumb_file_set(Evas_Object *obj,
                   const char *file,
                   const char *key)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_file_set(file, key));
}

static void
_file_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *key = va_arg(*list, const char *);
   Eina_Bool file_replaced, key_replaced;

   Elm_Thumb_Smart_Data *sd = _pd;

   file_replaced = eina_stringshare_replace(&(sd->file), file);
   key_replaced = eina_stringshare_replace(&(sd->key), key);

   if (file_replaced)
     {
        int prefix_size;
        const char **ext, *ptr;
        static const char *extensions[] =
        {
           ".avi", ".mp4", ".ogv", ".mov", ".mpg", ".wmv", NULL
        };

        prefix_size = eina_stringshare_strlen(sd->file) - 4;
        if (prefix_size >= 0)
          {
             ptr = sd->file + prefix_size;
             sd->is_video = EINA_FALSE;
             for (ext = extensions; *ext; ext++)
               if (!strcasecmp(ptr, *ext))
                 {
                    sd->is_video = EINA_TRUE;
                    break;
                 }
          }
     }

   eina_stringshare_replace(&(sd->thumb.file), NULL);
   eina_stringshare_replace(&(sd->thumb.key), NULL);

   if (((file_replaced) || (key_replaced)) && (evas_object_visible_get(obj)))
     _thumb_show(sd);
}

EAPI void
elm_thumb_file_get(const Evas_Object *obj,
                   const char **file,
                   const char **key)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_file_get(file, key));
}

static void
_file_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **file = va_arg(*list, const char **);
   const char **key = va_arg(*list, const char **);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (file)
     *file = sd->file;
   if (key)
     *key = sd->key;
}

EAPI void
elm_thumb_path_get(const Evas_Object *obj,
                   const char **file,
                   const char **key)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_path_get(file, key));
}

static void
_path_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **file = va_arg(*list, const char **);
   const char **key = va_arg(*list, const char **);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (file)
     *file = sd->thumb.file;
   if (key)
     *key = sd->thumb.key;
}

EAPI void
elm_thumb_aspect_set(Evas_Object *obj,
                     Ethumb_Thumb_Aspect aspect)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_aspect_set(aspect));
}

static void
_aspect_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Aspect aspect = va_arg(*list, Ethumb_Thumb_Aspect);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.aspect = aspect;
}

EAPI Ethumb_Thumb_Aspect
elm_thumb_aspect_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Ethumb_Thumb_Aspect ret;
   eo_do((Eo *)obj, elm_obj_thumb_aspect_get(&ret));
   return ret;
}

static void
_aspect_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Aspect *ret = va_arg(*list, Ethumb_Thumb_Aspect *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->thumb.aspect;
}

EAPI void
elm_thumb_fdo_size_set(Evas_Object *obj,
                       Ethumb_Thumb_FDO_Size size)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_fdo_size_set(size));
}

static void
_fdo_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_FDO_Size size = va_arg(*list, Ethumb_Thumb_FDO_Size);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.size = size;
}

EAPI Ethumb_Thumb_FDO_Size
elm_thumb_fdo_size_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Ethumb_Thumb_FDO_Size ret;
   eo_do((Eo *)obj, elm_obj_thumb_fdo_size_get(&ret));
   return ret;
}

static void
_fdo_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_FDO_Size *ret = va_arg(*list, Ethumb_Thumb_FDO_Size *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->thumb.size;
}

EAPI void
elm_thumb_format_set(Evas_Object *obj,
                     Ethumb_Thumb_Format format)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_format_set(format));
}

static void
_format_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Format format = va_arg(*list, Ethumb_Thumb_Format);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.format = format;
}

EAPI Ethumb_Thumb_Format
elm_thumb_format_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Ethumb_Thumb_Format ret;
   eo_do((Eo *)obj, elm_obj_thumb_format_get(&ret));
   return ret;
}

static void
_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Format *ret = va_arg(*list, Ethumb_Thumb_Format *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->thumb.format;
}

EAPI void
elm_thumb_orientation_set(Evas_Object *obj,
                          Ethumb_Thumb_Orientation orient)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_orientation_set(orient));
}

static void
_orientation_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Orientation orient = va_arg(*list, Ethumb_Thumb_Orientation);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.orient = orient;
}

EAPI Ethumb_Thumb_Orientation
elm_thumb_orientation_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Ethumb_Thumb_Orientation ret;
   eo_do((Eo *)obj, elm_obj_thumb_orientation_get(&ret));
   return ret;
}

static void
_orientation_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ethumb_Thumb_Orientation *ret = va_arg(*list, Ethumb_Thumb_Orientation *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->thumb.orient;
}

EAPI void
elm_thumb_size_set(Evas_Object *obj,
                   int tw,
                   int th)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_size_set(tw, th));
}

static void
_thumb_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int tw = va_arg(*list, int);
   int th = va_arg(*list, int);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.tw = tw;
   sd->thumb.th = th;
}

EAPI void
elm_thumb_size_get(const Evas_Object *obj,
                   int *tw,
                   int *th)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_size_get(tw, th));
}

static void
_thumb_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *tw = va_arg(*list, int *);
   int *th = va_arg(*list, int *);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (tw)
     *tw = sd->thumb.tw;
   if (th)
     *th = sd->thumb.th;
}

EAPI void
elm_thumb_crop_align_set(Evas_Object *obj,
                         double cropx,
                         double cropy)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_crop_align_set(cropx, cropy));
}

static void
_crop_align_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double cropx = va_arg(*list, double);
   double cropy = va_arg(*list, double);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.cropx = cropx;
   sd->thumb.cropy = cropy;
}

EAPI void
elm_thumb_crop_align_get(const Evas_Object *obj,
                         double *cropx,
                         double *cropy)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_crop_align_get(cropx, cropy));
}

static void
_crop_align_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *cropx = va_arg(*list, double *);
   double *cropy = va_arg(*list, double *);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (cropx)
     *cropx = sd->thumb.cropx;
   if (cropy)
     *cropy = sd->thumb.cropy;
}

EAPI void
elm_thumb_compress_set(Evas_Object *obj,
                       int compress)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_compress_set(compress));
}

static void
_compress_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int compress = va_arg(*list, int);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.compress = compress;
}

EAPI void
elm_thumb_compress_get(const Evas_Object *obj,
                       int *compress)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_compress_get(compress));
}

static void
_compress_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *compress = va_arg(*list, int *);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (compress)
     *compress = sd->thumb.compress;
}

EAPI void
elm_thumb_quality_set(Evas_Object *obj,
                      int quality)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_quality_set(quality));
}

static void
_quality_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int quality = va_arg(*list, int);
   Elm_Thumb_Smart_Data *sd = _pd;

   sd->thumb.quality = quality;
}

EAPI void
elm_thumb_quality_get(const Evas_Object *obj,
                      int *quality)
{
   ELM_THUMB_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_thumb_quality_get(quality));
}

static void
_quality_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *quality = va_arg(*list, int *);
   Elm_Thumb_Smart_Data *sd = _pd;

   if (quality)
     *quality = sd->thumb.quality;
}

EAPI void
elm_thumb_animate_set(Evas_Object *obj,
                      Elm_Thumb_Animation_Setting setting)
{
   ELM_THUMB_CHECK(obj);
   eo_do(obj, elm_obj_thumb_animate_set(setting));
}

static void
_animate_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Thumb_Animation_Setting setting = va_arg(*list, Elm_Thumb_Animation_Setting);
   Elm_Thumb_Smart_Data *sd = _pd;

   EINA_SAFETY_ON_TRUE_RETURN(setting >= ELM_THUMB_ANIMATION_LAST);

   sd->anim_setting = setting;

   if ((sd->is_video) && (sd->thumb.format == ETHUMB_THUMB_EET))
     {
        if (setting == ELM_THUMB_ANIMATION_LOOP)
          edje_object_signal_emit(sd->view, "elm,action,animate_loop", "elm");
        else if (setting == ELM_THUMB_ANIMATION_START)
          edje_object_signal_emit(sd->view, "elm,action,animate", "elm");
        else if (setting == ELM_THUMB_ANIMATION_STOP)
          edje_object_signal_emit(sd->view, "elm,action,animate_stop", "elm");
     }
}

EAPI Elm_Thumb_Animation_Setting
elm_thumb_animate_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) ELM_THUMB_ANIMATION_LAST;
   Elm_Thumb_Animation_Setting ret;
   eo_do((Eo *) obj, elm_obj_thumb_animate_get(&ret));
   return ret;
}

static void
_animate_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Thumb_Animation_Setting *ret = va_arg(*list, Elm_Thumb_Animation_Setting *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->anim_setting;
}

EAPI void *
elm_thumb_ethumb_client_get(void)
{
   return _elm_ethumb_client;
}

EAPI Eina_Bool
elm_thumb_ethumb_client_connected_get(void)
{
   return _elm_ethumb_connected;
}

EAPI Eina_Bool
elm_thumb_editable_set(Evas_Object *obj,
                       Eina_Bool edit)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_thumb_editable_set(edit, &ret));
   return ret;
}

static void
_editable_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool edit = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Thumb_Smart_Data *sd = _pd;
   if (ret) *ret = EINA_TRUE;

   edit = !!edit;
   if (sd->edit == edit) return;

   sd->edit = edit;
   if (sd->edit)
     elm_drop_target_add(obj, ELM_SEL_FORMAT_IMAGE,
                         NULL, NULL,
                         NULL, NULL,
                         NULL, NULL,
                         _elm_thumb_dnd_cb, obj);
   else
     elm_drop_target_del(obj, ELM_SEL_FORMAT_IMAGE,
                         NULL, NULL,
                         NULL, NULL,
                         NULL, NULL,
                         _elm_thumb_dnd_cb, obj);
}

EAPI Eina_Bool
elm_thumb_editable_get(const Evas_Object *obj)
{
   ELM_THUMB_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_thumb_editable_get(&ret));
   return ret;
}

static void
_editable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Thumb_Smart_Data *sd = _pd;
   *ret = sd->edit;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_thumb_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_thumb_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_thumb_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_thumb_smart_hide),

        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_RELOAD), _reload),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FILE_SET), _file_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FILE_GET), _file_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_PATH_GET), _path_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ASPECT_SET), _aspect_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ASPECT_GET), _aspect_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FDO_SIZE_SET), _fdo_size_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FDO_SIZE_GET), _fdo_size_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FORMAT_SET), _format_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_FORMAT_GET), _format_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ORIENTATION_SET), _orientation_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ORIENTATION_GET), _orientation_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_CROP_ALIGN_SET), _crop_align_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_CROP_ALIGN_GET), _crop_align_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_SIZE_SET), _thumb_size_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_SIZE_GET), _thumb_size_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_COMPRESS_SET), _compress_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_COMPRESS_GET), _compress_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_QUALITY_SET), _quality_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_QUALITY_GET), _quality_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ANIMATE_SET), _animate_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_ANIMATE_GET), _animate_get),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_EDITABLE_SET), _editable_set),
        EO_OP_FUNC(ELM_OBJ_THUMB_ID(ELM_OBJ_THUMB_SUB_ID_EDITABLE_GET), _editable_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_RELOAD, "Reload thumbnail if it was generated before."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FILE_SET, "Set the file that will be used as thumbnail source."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FILE_GET, "Get the image or video path and key used to generate the thumbnail."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_PATH_GET, "Get the path and key to the image or video thumbnail generated by ethumb."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ASPECT_SET, "Set the aspect of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ASPECT_GET, "Get the aspect of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FDO_SIZE_SET, "Set the FDO size of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FDO_SIZE_GET, "Get the FDO size of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FORMAT_SET, "Set the format of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_FORMAT_GET, "Get the format of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ORIENTATION_SET, "Set the orientation of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ORIENTATION_GET, "Get the orientation of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_CROP_ALIGN_SET, "Set the crop of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_CROP_ALIGN_GET, "Get the crop of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_SIZE_SET, "Set the thumb size of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_SIZE_GET, "Get the thumb size of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_COMPRESS_SET, "Set the compression of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_COMPRESS_GET, "Get the compression of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_QUALITY_SET, "Set the quality of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_QUALITY_GET, "Get the quality of the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ANIMATE_SET, "Set the animation state for the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_ANIMATE_GET, "Get the animation state for the thumb object."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_EDITABLE_SET, "Make the thumbnail 'editable'."),
     EO_OP_DESCRIPTION(ELM_OBJ_THUMB_SUB_ID_EDITABLE_GET, "Make the thumbnail 'editable'."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_THUMB_BASE_ID, op_desc, ELM_OBJ_THUMB_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Thumb_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_thumb_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, EVAS_SMART_CLICKABLE_INTERFACE, NULL);

