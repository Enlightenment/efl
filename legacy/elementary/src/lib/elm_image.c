#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_image.h"

#define FMT_SIZE_T "%zu"

#define MY_CLASS ELM_IMAGE_CLASS
#define MY_CLASS_NAME "Elm_Image"
#define MY_CLASS_NAME_LEGACY "elm_image"

static const char SIG_DND[] = "drop";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_DOWNLOAD_START[] = "download,start";
static const char SIG_DOWNLOAD_PROGRESS[] = "download,progress";
static const char SIG_DOWNLOAD_DONE[] = "download,done";
static const char SIG_DOWNLOAD_ERROR[] = "download,error";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DND, ""},
   {SIG_CLICKED, ""},
   {SIG_DOWNLOAD_START, ""},
   {SIG_DOWNLOAD_PROGRESS, ""},
   {SIG_DOWNLOAD_DONE, ""},
   {SIG_DOWNLOAD_ERROR, ""},
   {NULL, NULL}
};

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);
static void _elm_image_smart_internal_file_set(Eo *obj, Elm_Image_Data *sd, const char *file, const Eina_File *f, const char *key, Eina_Bool *ret);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

struct _Async_Open_Data {
   Eina_Stringshare *file, *key;
   Eina_File *f_set, *f_open;
   void *map;
   Eina_Bool cancel;
   Eina_Bool failed;
};

static void
_on_image_preloaded(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj,
                    void *event EINA_UNUSED)
{
   Elm_Image_Data *sd = data;
   sd->preloading = EINA_FALSE;
   if (sd->show) evas_object_show(obj);
   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static Eina_Bool
_elm_image_animate_cb(void *data)
{
   ELM_IMAGE_DATA_GET(data, sd);

   if (!sd->anim) return ECORE_CALLBACK_CANCEL;

   sd->cur_frame++;
   if (sd->cur_frame > sd->frame_count)
     sd->cur_frame = sd->cur_frame % sd->frame_count;

   evas_object_image_animated_frame_set(sd->img, sd->cur_frame);

   sd->frame_duration = evas_object_image_animated_frame_duration_get
       (sd->img, sd->cur_frame, 0);

   if (sd->frame_duration > 0)
     ecore_timer_interval_set(sd->anim_timer, sd->frame_duration);

   return ECORE_CALLBACK_RENEW;
}

static Evas_Object *
_img_new(Evas_Object *obj)
{
   Evas_Object *img;

   ELM_IMAGE_DATA_GET(obj, sd);

   img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_scale_hint_set(img, EVAS_IMAGE_SCALE_HINT_STATIC);
   evas_object_event_callback_add
     (img, EVAS_CALLBACK_IMAGE_PRELOADED, _on_image_preloaded, sd);

   evas_object_smart_member_add(img, obj);
   elm_widget_sub_object_add(obj, img);

   return img;
}

static void
_elm_image_internal_sizing_eval(Evas_Object *obj, Elm_Image_Data *sd)
{
   Evas_Coord x, y, w, h;
   const char *type;

   if (!sd->img) return;

   w = sd->img_w;
   h = sd->img_h;

   type = evas_object_type_get(sd->img);
   if (!type) return;

   if (!strcmp(type, "edje"))
     {
        x = sd->img_x;
        y = sd->img_y;
        evas_object_move(sd->img, x, y);
        evas_object_resize(sd->img, w, h);
     }
   else
     {
        double alignh = 0.5, alignv = 0.5;
        int iw = 0, ih = 0, offset_w = 0, offset_h = 0;
        evas_object_image_size_get(sd->img, &iw, &ih);

        iw = ((double)iw) * sd->scale;
        ih = ((double)ih) * sd->scale;

        if (iw < 1) iw = 1;
        if (ih < 1) ih = 1;

        if (sd->aspect_fixed)
          {
             h = ((double)ih * w) / (double)iw;
             if (sd->fill_inside)
               {
                  if (h > sd->img_h)
                    {
                       h = sd->img_h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
             else
               {
                  if (h < sd->img_h)
                    {
                       h = sd->img_h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
          }
        if (!sd->resize_up)
          {
             if (w > iw) w = iw;
             if (h > ih) h = ih;
          }
        if (!sd->resize_down)
          {
             if (w < iw) w = iw;
             if (h < ih) h = ih;
          }

        evas_object_size_hint_align_get
           (obj, &alignh, &alignv);

        if (alignh == EVAS_HINT_FILL) alignh = 0.5;
        if (alignv == EVAS_HINT_FILL) alignv = 0.5;

        offset_w = ((sd->img_w - w) * alignh);
        offset_h = ((sd->img_h - h) * alignv);

        if (sd->aspect_fixed && !sd->fill_inside)
          {
             evas_object_image_fill_set(sd->img, offset_w, offset_h, w, h);

             w = sd->img_w;
             h = sd->img_h;
             x = sd->img_x;
             y = sd->img_y;
          }
        else
          {
             evas_object_image_fill_set(sd->img, 0, 0, w, h);

             x = sd->img_x + offset_w;
             y = sd->img_y + offset_h;
          }

        evas_object_move(sd->img, x, y);
        evas_object_resize(sd->img, w, h);
     }
   evas_object_move(sd->hit_rect, x, y);
   evas_object_resize(sd->hit_rect, w, h);
}

static inline void
_async_open_data_free(Async_Open_Data *data)
{
   if (!data) return;
   eina_stringshare_del(data->file);
   eina_stringshare_del(data->key);
   if (data->map) eina_file_map_free(data->f_open, data->map);
   if (data->f_open) eina_file_close(data->f_open);
   if (data->f_set) eina_file_close(data->f_set);
   free(data);
}

static void
_elm_image_async_open_do(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   Eina_File *f;
   Async_Open_Data *todo, *done;
   void *map = NULL;
   unsigned int buf;

   ELM_IMAGE_DATA_GET(obj, sd);

   eina_spinlock_take(&sd->async.lck);
   todo = sd->async.todo;
   done = sd->async.done;
   sd->async.todo = NULL;
   sd->async.done = NULL;
   eina_spinlock_release(&sd->async.lck);

   if (done) _async_open_data_free(done);
   if (!todo) return;

begin:
   if (todo->f_set)
     f = todo->f_set;
   else if (todo->file)
     {
        // blocking
        f = eina_file_open(todo->file, EINA_FALSE);
        if (!f)
          {
             todo->failed = EINA_TRUE;
             eina_spinlock_take(&sd->async.lck);
             sd->async.done = todo;
             eina_spinlock_release(&sd->async.lck);
             return;
          }
     }
   else
     {
        CRI("Async open has no input file!");
        return;
     }

   if (ecore_thread_check(sd->async.th))
     {
        if (!todo->f_set) eina_file_close(f);
        _async_open_data_free(todo);
        return;
     }

   // Read just enough data for map to actually do something.
   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (map && (eina_file_size_get(f) >= sizeof(buf)))
     memcpy(&buf, map, sizeof(buf));

   if (ecore_thread_check(sd->async.th))
     {
        if (map) eina_file_map_free(f, map);
        if (!todo->f_set) eina_file_close(f);
        _async_open_data_free(todo);
        return;
     }

   done = todo;
   todo = NULL;
   done->cancel = EINA_FALSE;
   done->failed = EINA_FALSE;
   done->f_set = NULL;
   done->f_open = f;
   done->map = map;

   eina_spinlock_take(&sd->async.lck);
   todo = sd->async.todo;
   sd->async.todo = NULL;
   if (!todo) sd->async.done = done;
   eina_spinlock_release(&sd->async.lck);

   if (todo)
     {
        DBG("Async open got a new command before finishing");
        _async_open_data_free(done);
        goto begin;
     }
}

static void
_elm_image_async_open_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   ELM_IMAGE_DATA_GET(obj, sd);

   DBG("Async open thread was canceled");
   sd->async.th = NULL;
   sd->async_opening = EINA_FALSE;
   sd->async_failed = EINA_TRUE;
   // keep file, key for file_get()
}

static void
_elm_image_async_open_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   Eina_Stringshare *file, *key;
   Async_Open_Data *done;
   Eina_Bool ok;
   Eina_File *f;
   void *map;

   ELM_IMAGE_DATA_GET(obj, sd);

   // no need to lock here, thread can't be running now

   sd->async.th = NULL;
   sd->async_failed = EINA_FALSE;

   if (sd->async.todo)
     {
        sd->async.th = ecore_thread_run(_elm_image_async_open_do,
                                        _elm_image_async_open_done,
                                        _elm_image_async_open_cancel, obj);
        return;
     }

   sd->async_opening = EINA_FALSE;

   done = sd->async.done;
   if (!done)
     {
        // done should be NULL only after cancel... not here
        ERR("Async open failed for an unknown reason.");
        sd->async_failed = EINA_TRUE;
        eo_do(obj, eo_event_callback_call(EFL_FILE_EVENT_ASYNC_ERROR, NULL));
        return;
     }

   DBG("Async open succeeded");
   sd->async.done = NULL;
   key = done->key;
   map = done->map;
   f = done->f_open;
   ok = f && map;
   if (done->file)
     file = done->file;
   else
     file = f ? eina_file_filename_get(f) : NULL;

   if (sd->edje)
     {
        if (ok) ok = edje_object_mmap_set(sd->img, f, key);
        if (!ok)
          {
             ERR("failed to open edje file '%s', group '%s': %s", file, key,
                 edje_load_error_str(edje_object_load_error_get(sd->img)));
          }
     }
   else
     {
        if (ok) _elm_image_smart_internal_file_set(obj, sd, file, f, key, &ok);
        if (!ok)
          {
             ERR("failed to open image file '%s', key '%s': %s", file, key,
                  evas_load_error_str(evas_object_image_load_error_get(sd->img)));
          }
     }

   if (ok)
     {
        // TODO: Implement Efl.File async,opened event_info type
        sd->async_failed = EINA_FALSE;
        ELM_SAFE_FREE(sd->async.file, eina_stringshare_del);
        ELM_SAFE_FREE(sd->async.key, eina_stringshare_del);
        eo_do(obj, eo_event_callback_call(EFL_FILE_EVENT_ASYNC_OPENED, NULL));
        _elm_image_internal_sizing_eval(obj, sd);
     }
   else
     {
        // TODO: Implement Efl.File async,error event_info type
        sd->async_failed = EINA_TRUE;
        // keep file,key around for file_get
        eo_do(obj, eo_event_callback_call(EFL_FILE_EVENT_ASYNC_ERROR, NULL));
     }

   // close f, map and free strings
   _async_open_data_free(done);
}

static Eina_Bool
_elm_image_async_file_set(Eo *obj, Elm_Image_Data *sd,
                          const char *file, const Eina_File *f, const char *key)
{
   Async_Open_Data *todo;
   Eina_Bool was_running;

   if (sd->async_opening &&
       ((file == sd->async.file) ||
        (file && sd->async.file && !strcmp(file, sd->async.file))) &&
       ((key == sd->async.key) ||
        (key && sd->async.key && !strcmp(key, sd->async.key))))
     return EINA_TRUE;

   sd->async_opening = EINA_TRUE;
   eina_stringshare_replace(&sd->async.file, file);
   eina_stringshare_replace(&sd->async.key, key);

   if (!sd->async.th)
     {
        was_running = EINA_FALSE;
        sd->async.th = ecore_thread_run(_elm_image_async_open_do,
                                        _elm_image_async_open_done,
                                        _elm_image_async_open_cancel, obj);
     }
   else was_running = EINA_TRUE;

   if (!sd->async.th)
     {
        DBG("Could not spawn an async thread!");
        sd->async_failed = EINA_TRUE;
        return EINA_FALSE;
     }

   todo = calloc(1, sizeof(Async_Open_Data));
   if (!todo)
     {
        sd->async_failed = EINA_TRUE;
        return EINA_FALSE;
     }

   todo->file = eina_stringshare_add(file);
   todo->key = eina_stringshare_add(key);
   todo->f_set = f ? eina_file_dup(f) : NULL;
   if (!was_running)
     sd->async.todo = todo;
   else
     {
        Async_Open_Data *prev;
        eina_spinlock_take(&sd->async.lck);
        prev = sd->async.todo;
        sd->async.todo = todo;
        eina_spinlock_release(&sd->async.lck);
        _async_open_data_free(prev);
     }

   sd->async_failed = EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_elm_image_edje_file_set(Evas_Object *obj,
                         const char *file,
                         const Eina_File *f,
                         const char *group)
{
   Evas_Object *pclip;

   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);

   if (!sd->edje)
     {
        pclip = evas_object_clip_get(sd->img);
        evas_object_del(sd->img);

        /* Edje object instead */
        sd->img = edje_object_add(evas_object_evas_get(obj));
        evas_object_smart_member_add(sd->img, obj);
        if (sd->show) evas_object_show(sd->img);
        evas_object_clip_set(sd->img, pclip);
     }

   sd->edje = EINA_TRUE;
   if (!sd->async_enable)
     {
        if (f)
          {
             if (!edje_object_mmap_set(sd->img, f, group))
               {
                  ERR("failed to set edje file '%s', group '%s': %s", file, group,
                      edje_load_error_str(edje_object_load_error_get(sd->img)));
                  return EINA_FALSE;
               }
          }
        else
          {
             if (!edje_object_file_set(sd->img, file, group))
               {
                  ERR("failed to set edje file '%s', group '%s': %s", file, group,
                      edje_load_error_str(edje_object_load_error_get(sd->img)));
                  return EINA_FALSE;
               }
          }
     }
   else
     return _elm_image_async_file_set(obj, sd, file, f, group);

   /* FIXME: do i want to update icon on file change ? */
   _elm_image_internal_sizing_eval(obj, sd);

   return EINA_TRUE;
}

EOLIAN static void
_elm_image_efl_image_smooth_scale_set(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, Eina_Bool smooth)
{
   if (sd->edje) return;

   evas_object_image_smooth_scale_set(sd->img, smooth);
}

EOLIAN static Eina_Bool
_elm_image_efl_image_smooth_scale_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   if (sd->edje) return EINA_FALSE;

   return evas_object_image_smooth_scale_get(sd->img);
}

EOLIAN static void
_elm_image_fill_inside_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool fill_inside)
{
   fill_inside = !!fill_inside;

   if (sd->fill_inside == fill_inside) return;

   sd->fill_inside = fill_inside;

   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Eina_Bool
_elm_image_fill_inside_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->fill_inside;
}

EOLIAN static void
_elm_image_resize_up_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool resize_up)
{
   resize_up = !!resize_up;

   if (sd->resize_up == resize_up) return;

   sd->resize_up = resize_up;

   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Eina_Bool
_elm_image_resize_up_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->resize_up;
}

EOLIAN static void
_elm_image_resize_down_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool resize_down)
{
   resize_down = !!resize_down;

   if (sd->resize_down == resize_down) return;

   sd->resize_down = resize_down;

   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Eina_Bool
_elm_image_resize_down_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->resize_up;
}

static Eina_Bool
_elm_image_drag_n_drop_cb(void *elm_obj,
      Evas_Object *obj,
      Elm_Selection_Data *drop)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, ret = efl_file_set(drop->data, NULL));
   if (ret)
     {
        DBG("dnd: %s, %s, %s", elm_widget_type_get(elm_obj),
              SIG_DND, (char *)drop->data);

        evas_object_smart_callback_call(elm_obj, SIG_DND, drop->data);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_elm_image_evas_object_smart_add(Eo *obj, Elm_Image_Data *priv)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);
   evas_object_propagate_events_set(priv->hit_rect, EINA_FALSE);

   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   evas_object_event_callback_add
      (priv->hit_rect, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, obj);

   /* starts as an Evas image. may switch to an Edje object */
   priv->img = _img_new(obj);

   priv->smooth = EINA_TRUE;
   priv->fill_inside = EINA_TRUE;
   priv->resize_up = EINA_TRUE;
   priv->resize_down = EINA_TRUE;
   priv->aspect_fixed = EINA_TRUE;
   priv->load_size = 0;
   priv->scale = 1.0;
   eina_spinlock_new(&priv->async.lck);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   eo_do(obj, elm_obj_image_sizing_eval());
}

EOLIAN static void
_elm_image_evas_object_smart_del(Eo *obj, Elm_Image_Data *sd)
{
   ecore_timer_del(sd->anim_timer);
   evas_object_del(sd->img);
   evas_object_del(sd->prev_img);
   if (sd->remote) _elm_url_cancel(sd->remote);
   free(sd->remote_data);
   eina_stringshare_del(sd->key);

   if (sd->async.th)
     {
        if (!ecore_thread_cancel(sd->async.th) &&
            !ecore_thread_wait(sd->async.th, 1.0))
          {
             ERR("Async open thread timed out during cancellation.");
             // skipping all other data free to avoid crashes (this leaks)
             eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
             return;
          }
        sd->async.th = NULL;
     }

   _async_open_data_free(sd->async.done);
   _async_open_data_free(sd->async.todo);
   eina_stringshare_del(sd->async.file);
   eina_stringshare_del(sd->async.key);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_image_evas_object_smart_move(Eo *obj, Elm_Image_Data *sd, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   if ((sd->img_x == x) && (sd->img_y == y)) return;
   sd->img_x = x;
   sd->img_y = y;

   /* takes care of moving */
   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_elm_image_evas_object_smart_resize(Eo *obj, Elm_Image_Data *sd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   if ((sd->img_w == w) && (sd->img_h == h)) return;

   sd->img_w = w;
   sd->img_h = h;

   /* takes care of resizing */
   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_elm_image_evas_object_smart_show(Eo *obj, Elm_Image_Data *sd)
{
   sd->show = EINA_TRUE;
   if (sd->preloading) return;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());

   evas_object_show(sd->img);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

EOLIAN static void
_elm_image_evas_object_smart_hide(Eo *obj, Elm_Image_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());

   sd->show = EINA_FALSE;
   evas_object_hide(sd->img);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

EOLIAN static void
_elm_image_evas_object_smart_member_add(Eo *obj, Elm_Image_Data *sd, Evas_Object *member)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_elm_image_evas_object_smart_color_set(Eo *obj, Elm_Image_Data *sd, int r, int g, int b, int a)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_color_set(r, g, b, a));

   evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
   evas_object_color_set(sd->img, r, g, b, a);
   if (sd->prev_img) evas_object_color_set(sd->prev_img, r, g, b, a);
}

EOLIAN static void
_elm_image_evas_object_smart_clip_set(Eo *obj, Elm_Image_Data *sd, Evas_Object *clip)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_clip_set(clip));

   evas_object_clip_set(sd->img, clip);
   if (sd->prev_img) evas_object_clip_set(sd->prev_img, clip);
}

EOLIAN static void
_elm_image_evas_object_smart_clip_unset(Eo *obj, Elm_Image_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_clip_unset());

   evas_object_clip_unset(sd->img);
   if (sd->prev_img) evas_object_clip_unset(sd->prev_img);
}

EOLIAN static Eina_Bool
_elm_image_elm_widget_theme_apply(Eo *obj, Elm_Image_Data *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   eo_do(obj, elm_obj_image_sizing_eval());

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_image_elm_widget_event(Eo *obj, Elm_Image_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static void
_elm_image_sizing_eval(Eo *obj, Elm_Image_Data *sd)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w = 0, h = 0;
   double ts;

   _elm_image_internal_sizing_eval(obj, sd);

   eo_do(obj, efl_image_smooth_scale_set(sd->smooth));

   if (sd->no_scale)
     eo_do(obj, elm_obj_image_scale_set(1.0));
   else
     eo_do(obj, efl_image_smooth_scale_set(elm_widget_scale_get(obj) * elm_config_scale_get()));

   ts = sd->scale;
   sd->scale = 1.0;
   eo_do(obj, elm_obj_image_object_size_get(&w, &h));

   sd->scale = ts;
   evas_object_size_hint_min_get(obj, &minw, &minh);

   if (sd->no_scale)
     {
        maxw = minw = w;
        maxh = minh = h;
        if ((sd->scale > 1.0) && (sd->resize_up))
          {
             maxw = minw = w * sd->scale;
             maxh = minh = h * sd->scale;
          }
        else if ((sd->scale < 1.0) && (sd->resize_down))
          {
             maxw = minw = w * sd->scale;
             maxh = minh = h * sd->scale;
          }
     }
   else
     {
        if (!sd->resize_down)
          {
             minw = w * sd->scale;
             minh = h * sd->scale;
          }
        if (!sd->resize_up)
          {
             maxw = w * sd->scale;
             maxh = h * sd->scale;
          }
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_elm_image_file_set_do(Evas_Object *obj)
{
   Evas_Object *pclip = NULL;
   int w = 0, h = 0;

   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
   if (sd->img)
     {
        pclip = evas_object_clip_get(sd->img);
        sd->prev_img = sd->img;
     }

   sd->img = _img_new(obj);

   evas_object_image_load_orientation_set(sd->img, EINA_TRUE);

   evas_object_clip_set(sd->img, pclip);

   sd->edje = EINA_FALSE;

   if (sd->load_size > 0)
     evas_object_image_load_size_set(sd->img, sd->load_size, sd->load_size);
   else
     {
        eo_do((Eo *) obj, elm_obj_image_object_size_get(&w, &h));
        evas_object_image_load_size_set(sd->img, w, h);
     }
}

EOLIAN static Eina_Bool
_elm_image_memfile_set(Eo *obj, Elm_Image_Data *sd, const void *img, size_t size, const char *format, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(img, EINA_FALSE);

   _elm_image_file_set_do(obj);

   evas_object_image_memfile_set
     (sd->img, (void *)img, size, (char *)format, (char *)key);

   sd->preloading = EINA_TRUE;
   evas_object_image_preload(sd->img, EINA_FALSE);

   if (evas_object_image_load_error_get(sd->img) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for some random " FMT_SIZE_T
            " byte chunk of memory (%p)", size, sd->img);
        return EINA_FALSE;
     }

   _elm_image_internal_sizing_eval(obj, sd);

   return EINA_TRUE;
}

EOLIAN static void
_elm_image_scale_set(Eo *obj, Elm_Image_Data *sd, double scale)
{
   sd->scale = scale;

   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static double
_elm_image_scale_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->scale;
}

EAPI Evas_Object *
elm_image_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_image_eo_base_constructor(Eo *obj, Elm_Image_Data *_pd EINA_UNUSED)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_IMAGE));

   return obj;
}

EAPI Eina_Bool
elm_image_file_set(Evas_Object *obj,
                   const char *file,
                   const char *group)
{
   Eina_Bool ret = EINA_FALSE;

   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   eo_do(obj,
         ret = efl_file_set(file, group);
         elm_obj_image_sizing_eval());
   return ret;
}

EAPI void
elm_image_file_get(const Eo *obj, const char **file, const char **group)
{
   eo_do((Eo *) obj, efl_file_get(file, group));
}

EAPI Eina_Bool
elm_image_mmap_set(Evas_Object *obj,
		   const Eina_File *file,
		   const char *group)
{
   Eina_Bool ret = EINA_FALSE;

   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   eo_do(obj, ret = efl_file_mmap_set(file, group));
   return ret;
}

EOLIAN Eina_Bool
_elm_image_efl_file_mmap_set(Eo *obj, Elm_Image_Data *pd EINA_UNUSED,
                             const Eina_File *file, const char *key)
{
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   eo_do(obj,
         ret = elm_obj_image_mmap_set(file, key),
         elm_obj_image_sizing_eval());

   return ret;
}

static void
_elm_image_smart_internal_file_set(Eo *obj, Elm_Image_Data *sd,
                                   const char *file, const Eina_File *f, const char *key, Eina_Bool *ret)
{
   if (eina_str_has_extension(file, ".edj"))
     {
        Eina_Bool int_ret = _elm_image_edje_file_set(obj, file, f, key);
        if (ret) *ret = int_ret;
        return;
     }

   _elm_image_file_set_do(obj);

   if (f)
     evas_object_image_mmap_set(sd->img, f, key);
   else
     evas_object_image_file_set(sd->img, file, key);

   evas_object_hide(sd->img);

   if (evas_object_visible_get(obj))
     {
        sd->preloading = EINA_TRUE;
        evas_object_image_preload(sd->img, EINA_FALSE);
     }

   if (evas_object_image_load_error_get(sd->img) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p)", file, sd->img);
        if (ret) *ret = EINA_FALSE;
        return;
     }

   _elm_image_internal_sizing_eval(obj, sd);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_image_smart_download_done(void *data, Elm_Url *url EINA_UNUSED, Eina_Binbuf *download)
{
   Eo *obj = data;
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Eina_File *f;
   size_t length;
   Eina_Bool ret = EINA_FALSE;

   free(sd->remote_data);
   length = eina_binbuf_length_get(download);
   sd->remote_data = eina_binbuf_string_steal(download);
   f = eina_file_virtualize(_elm_url_get(url),
                            sd->remote_data, length,
                            EINA_FALSE);
   _elm_image_smart_internal_file_set(obj, sd, _elm_url_get(url), f, sd->key, &ret);
   eina_file_close(f);

   sd->remote = NULL;
   if (!ret)
     {
        Elm_Image_Error err = { 0, EINA_TRUE };

        free(sd->remote_data);
        sd->remote_data = NULL;
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);
     }
   else
     {
        if (evas_object_visible_get(obj))
          {
             sd->preloading = EINA_TRUE;
             evas_object_image_preload(sd->img, EINA_FALSE);
          }

        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_DONE, NULL);
     }

   ELM_SAFE_FREE(sd->key, eina_stringshare_del);
}

static void
_elm_image_smart_download_cancel(void *data, Elm_Url *url EINA_UNUSED, int error)
{
   Eo *obj = data;
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Elm_Image_Error err = { error, EINA_FALSE };

   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);

   sd->remote = NULL;
   ELM_SAFE_FREE(sd->key, eina_stringshare_del);
}

static void
_elm_image_smart_download_progress(void *data, Elm_Url *url EINA_UNUSED, double now, double total)
{
   Eo *obj = data;
   Elm_Image_Progress progress;

   progress.now = now;
   progress.total = total;
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_PROGRESS, &progress);
}

static const char *remote_uri[] = {
  "http://", "https://", "ftp://"
};

EOLIAN static Eina_Bool
_elm_image_efl_file_file_set(Eo *obj, Elm_Image_Data *sd, const char *file, const char *key)
{
   Eina_Bool ret = EINA_FALSE;
   unsigned int i;

   if (sd->remote) _elm_url_cancel(sd->remote);
   sd->remote = NULL;

   for (i = 0; i < sizeof (remote_uri) / sizeof (remote_uri[0]); ++i)
     if (file && !strncmp(remote_uri[i], file, strlen(remote_uri[i])))
       {
          // Found a remote target !
          evas_object_hide(sd->img);
          sd->remote = _elm_url_download(file,
                                        _elm_image_smart_download_done,
                                        _elm_image_smart_download_cancel,
                                        _elm_image_smart_download_progress,
                                        obj);
          if (sd->remote)
            {
               evas_object_smart_callback_call(obj, SIG_DOWNLOAD_START, NULL);
               eina_stringshare_replace(&sd->key, key);
               return EINA_TRUE;
            }
          break;
       }

   if (!sd->async_enable)
     _elm_image_smart_internal_file_set(obj, sd, file, NULL, key, &ret);
   else
     ret = _elm_image_async_file_set(obj, sd, file, NULL, key);

   return ret;
}

EOLIAN static void
_elm_image_edje_object_signal_emit(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, const char *emission, const char *source)
{
   if (sd->edje)
     edje_object_signal_emit(sd->img, emission, source);
}

EOLIAN static void
_elm_image_edje_object_size_min_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_min_get(sd->img, w, h);
   else
     evas_object_size_hint_min_get(sd->img, w, h);
}

EOLIAN static void
_elm_image_edje_object_size_max_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_max_get(sd->img, w, h);
   else
     evas_object_size_hint_max_get(sd->img, w, h);
}

EOLIAN static void
_elm_image_edje_object_calc_force(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   if (sd->edje)
     edje_object_calc_force(sd->img);
}

EOLIAN static void
_elm_image_edje_object_size_min_calc(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_min_calc(sd->img, w, h);
   else
     evas_object_size_hint_min_get(sd->img, w, h);
}

EOLIAN static Eina_Bool
_elm_image_mmap_set(Eo *obj, Elm_Image_Data *sd, const Eina_File *f, const char *key)
{
  Eina_Bool ret;

  if (sd->remote) _elm_url_cancel(sd->remote);
  sd->remote = NULL;

  if (!sd->async_enable)
    {
       _elm_image_smart_internal_file_set(obj, sd,
                                          eina_file_filename_get(f), f,
                                          key, &ret);
    }
  else
    ret = _elm_image_async_file_set(obj, sd, eina_file_filename_get(f), f, key);

   return ret;
}

EOLIAN static void
_elm_image_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, const char **file, const char **key)
{
   if (sd->async_enable && (sd->async_opening || sd->async_failed))
     {
        if (file) *file = sd->async.file;
        if (key) *key = sd->async.key;
        return;
     }

   eo_do(sd->img, efl_file_get(file, key));
}

EOLIAN static void
_elm_image_smooth_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool smooth)
{
   sd->smooth = smooth;

   eo_do(obj, elm_obj_image_sizing_eval());
}

EOLIAN static Eina_Bool
_elm_image_smooth_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->smooth;
}

static Eina_Bool
_elm_image_efl_file_async_wait(Eo *obj EINA_UNUSED, Elm_Image_Data *pd)
{
   Eina_Bool ok = EINA_TRUE;
   if (!pd->async_enable) return ok;
   if (!pd->async.th) return ok;
   if (!ecore_thread_wait(pd->async.th, 1.0))
     {
        ERR("Failed to wait on async file open!");
        ok = EINA_FALSE;
     }
   return ok;
}

EOLIAN static void
_elm_image_efl_file_async_set(Eo *obj, Elm_Image_Data *pd, Eina_Bool async)
{
   if (pd->async_enable == async)
     return;

   pd->async_enable = async;
   if (!async)
     _elm_image_efl_file_async_wait(obj, pd);
}

EOLIAN static Eina_Bool
_elm_image_efl_file_async_get(Eo *obj EINA_UNUSED, Elm_Image_Data *pd)
{
   return pd->async_enable;
}

EOLIAN static void
_elm_image_object_size_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int *w, int *h)
{
   int tw, th;
   int cw = 0, ch = 0;
   const char *type;

   if (w) *w = 0;
   if (h) *h = 0;

   type = evas_object_type_get(sd->img);
   if (!type) return;

   if (!strcmp(type, "edje"))
     edje_object_size_min_get(sd->img, &tw, &th);
   else
     evas_object_image_size_get(sd->img, &tw, &th);

   if ((sd->resize_up) || (sd->resize_down))
     evas_object_geometry_get(sd->img, NULL, NULL, &cw, &ch);

   tw = tw > cw ? tw : cw;
   th = th > ch ? th : ch;
   tw = ((double)tw) * sd->scale;
   th = ((double)th) * sd->scale;
   if (w) *w = tw;
   if (h) *h = th;
}

EOLIAN static void
_elm_image_no_scale_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool no_scale)
{
   sd->no_scale = no_scale;

   eo_do(obj, elm_obj_image_sizing_eval());
}

EOLIAN static Eina_Bool
_elm_image_no_scale_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->no_scale;
}

EOLIAN static void
_elm_image_resizable_set(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, Eina_Bool up, Eina_Bool down)
{
   sd->resize_up = !!up;
   sd->resize_down = !!down;

   eo_do(obj, elm_obj_image_sizing_eval());
}

EOLIAN static void
_elm_image_resizable_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, Eina_Bool *size_up, Eina_Bool *size_down)
{
   if (size_up) *size_up = sd->resize_up;
   if (size_down) *size_down = sd->resize_down;
}

EOLIAN static void
_elm_image_fill_outside_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool fill_outside)
{
   sd->fill_inside = !fill_outside;

   eo_do(obj, elm_obj_image_sizing_eval());
}

EOLIAN static Eina_Bool
_elm_image_fill_outside_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return !sd->fill_inside;
}

EOLIAN static void
_elm_image_preload_disabled_set(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, Eina_Bool disable)
{
   if (sd->edje || !sd->preloading) return;

   //FIXME: Need to keep the disabled status for next image loading.

   evas_object_image_preload(sd->img, disable);
   sd->preloading = !disable;

   if (disable)
     {
        if (sd->show && sd->img) evas_object_show(sd->img);
        ELM_SAFE_FREE(sd->prev_img, evas_object_del);
     }
}

EAPI void
elm_image_prescale_set(Evas_Object *obj,
                       int size)
{
   ELM_IMAGE_CHECK(obj);
   eo_do(obj, efl_image_load_size_set(size, size));
}

EOLIAN static void
_elm_image_efl_image_load_size_set(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int w, int h)
{
   if (w > h)
      sd->load_size = w;
   else
      sd->load_size = h;
}

EAPI int
elm_image_prescale_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) 0;

   int w = 0;
   eo_do((Eo *)obj, efl_image_load_size_get(&w, NULL));
   return w;
}

EOLIAN static void
_elm_image_efl_image_load_size_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd, int *w, int *h)
{
   if (w) *w = sd->load_size;
   if (h) *h = sd->load_size;
}

EOLIAN static void
_elm_image_orient_set(Eo *obj, Elm_Image_Data *sd, Elm_Image_Orient orient)
{
   int iw, ih;

   if (sd->edje) return;
   if (sd->orient == orient) return;

   evas_object_image_orient_set(sd->img, orient);
   sd->orient = orient;
   evas_object_image_size_get(sd->img, &iw, &ih);
   evas_object_image_data_update_add(sd->img, 0, 0, iw, ih);
   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Elm_Image_Orient
_elm_image_orient_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->orient;
}

/**
 * Turns on editing through drag and drop and copy and paste.
 */
EOLIAN static void
_elm_image_editable_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool edit)
{
   if (sd->edje)
     {
        WRN("No editing edje objects yet (ever)\n");
        return;
     }

   edit = !!edit;

   if (edit == sd->edit) return;

   sd->edit = edit;

   if (sd->edit)
     elm_drop_target_add
       (obj, ELM_SEL_FORMAT_IMAGE,
           NULL, NULL,
           NULL, NULL,
           NULL, NULL,
           _elm_image_drag_n_drop_cb, obj);
   else
     elm_drop_target_del
       (obj, ELM_SEL_FORMAT_IMAGE,
           NULL, NULL,
           NULL, NULL,
           NULL, NULL,
           _elm_image_drag_n_drop_cb, obj);
}

EOLIAN static Eina_Bool
_elm_image_editable_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->edit;
}

EOLIAN static Evas_Object*
_elm_image_object_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->img;
}

EOLIAN static void
_elm_image_aspect_fixed_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool fixed)
{
   fixed = !!fixed;
   if (sd->aspect_fixed == fixed) return;

   sd->aspect_fixed = fixed;

   _elm_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Eina_Bool
_elm_image_aspect_fixed_get(Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   return sd->aspect_fixed;
}

EAPI Eina_Bool
elm_image_animated_available_get(const Evas_Object *obj)
{
   Eina_Bool ret;
   eo_do(obj, ret = efl_player_playable_get());
   return ret;
}

EOLIAN static Eina_Bool
_elm_image_efl_player_playable_get(Eo *obj, Elm_Image_Data *sd)
{
   if (sd->edje) return EINA_FALSE;

   return evas_object_image_animated_get(elm_image_object_get(obj));
}

static void
_elm_image_animated_set_internal(Eo *obj, Elm_Image_Data *sd, Eina_Bool anim)
{
   anim = !!anim;
   if (sd->anim == anim) return;

   sd->anim = anim;

   if (sd->edje)
     {
        edje_object_animation_set(sd->img, anim);
        return;
     }
   sd->img = elm_image_object_get(obj);
   if (!evas_object_image_animated_get(sd->img)) return;

   if (anim)
     {
        sd->frame_count = evas_object_image_animated_frame_count_get(sd->img);
        sd->cur_frame = 1;
        sd->frame_duration =
          evas_object_image_animated_frame_duration_get
            (sd->img, sd->cur_frame, 0);
        evas_object_image_animated_frame_set(sd->img, sd->cur_frame);
     }
   else
     {
        sd->frame_count = -1;
        sd->cur_frame = -1;
        sd->frame_duration = -1;
     }

   return;
}

static Eina_Bool
_elm_image_animated_get_internal(const Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   if (sd->edje)
     return edje_object_animation_get(sd->img);
   return sd->anim;
}

EAPI void
elm_image_animated_set(Evas_Object *obj, Eina_Bool anim)
{
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _elm_image_animated_set_internal(obj, sd, anim);
}

EAPI Eina_Bool
elm_image_animated_get(const Evas_Object *obj)
{
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return _elm_image_animated_get_internal(obj, sd);
}

static void
_elm_image_animated_play_set_internal(Eo *obj, Elm_Image_Data *sd, Eina_Bool play)
{
   if (!sd->anim) return;
   if (sd->play == play) return;
   sd->play = play;
   if (sd->edje)
     {
        edje_object_play_set(sd->img, play);
        return;
     }
   if (play)
     {
        sd->anim_timer = ecore_timer_add
            (sd->frame_duration, _elm_image_animate_cb, obj);
     }
   else
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
     }
}

static Eina_Bool
_elm_image_animated_play_get_internal(const Eo *obj EINA_UNUSED, Elm_Image_Data *sd)
{
   if (sd->edje)
     return edje_object_play_get(sd->img);
   return sd->play;
}

EAPI void
elm_image_animated_play_set(Elm_Image *obj, Eina_Bool play)
{
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _elm_image_animated_play_set_internal(obj, sd, play);
}

EAPI Eina_Bool
elm_image_animated_play_get(const Elm_Image *obj)
{
   Elm_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return _elm_image_animated_play_get_internal(obj, sd);
}

EOLIAN static void
_elm_image_efl_player_play_set(Eo *obj, Elm_Image_Data *sd, Eina_Bool play)
{
   if (play && !_elm_image_animated_get_internal(obj, sd))
     _elm_image_animated_set_internal(obj, sd, play);
   _elm_image_animated_play_set_internal(obj, sd, play);
}

EOLIAN static Eina_Bool
_elm_image_efl_player_play_get(Eo *obj, Elm_Image_Data *sd)
{
   return _elm_image_animated_play_get_internal(obj, sd);
}

static void
_elm_image_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

// A11Y

EOLIAN static void
_elm_image_elm_interface_atspi_image_extents_get(Eo *obj, Elm_Image_Data *sd EINA_UNUSED, Eina_Bool screen_coords, int *x, int *y, int *w, int *h)
{
   int ee_x, ee_y;
   Evas_Object *image = elm_image_object_get(obj);
   if (!image) return;

   evas_object_geometry_get(image, x, y, NULL, NULL);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(image));
        if (!ee) return;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        if (x) *x += ee_x;
        if (y) *y += ee_y;
     }
   elm_image_object_size_get(obj, w, h);
}

EOLIAN const Elm_Atspi_Action *
_elm_image_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Image_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
        { "activate", "activate", NULL, _key_action_activate },
        { NULL, NULL, NULL, NULL },
   };
   return &atspi_actions[0];
}


// A11Y - END

#include "elm_image.eo.c"
