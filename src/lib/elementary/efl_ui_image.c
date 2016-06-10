#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_IMAGE_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_widget_image.h"

#define FMT_SIZE_T "%zu"

#define MY_CLASS EFL_UI_IMAGE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Image"
#define MY_CLASS_NAME_LEGACY "elm_image"

#define NON_EXISTING (void *)-1
static const char *icon_theme = NULL;

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
static Eina_Bool _efl_ui_image_smart_internal_file_set(Eo *obj, Efl_Ui_Image_Data *sd, const char *file, const Eina_File *f, const char *key);
static void _efl_ui_image_sizing_eval(Eo *obj);

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
   Efl_Ui_Image_Data *sd = data;
   sd->preload_status = EFL_UI_IMAGE_PRELOADED;
   if (sd->show) evas_object_show(obj);
   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!wd->still_in) return;

   eo_event_callback_call(data, EFL_UI_EVENT_CLICKED, NULL);
}

static Eina_Bool
_efl_ui_image_animate_cb(void *data)
{
   EFL_UI_IMAGE_DATA_GET(data, sd);

   if (!sd->anim) return ECORE_CALLBACK_CANCEL;

   sd->cur_frame++;
   if ((sd->frame_count > 0) && (sd->cur_frame > sd->frame_count))
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

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   img = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_scale_hint_set(img, EVAS_IMAGE_SCALE_HINT_STATIC);
   evas_object_event_callback_add
     (img, EVAS_CALLBACK_IMAGE_PRELOADED, _on_image_preloaded, sd);

   evas_object_smart_member_add(img, obj);
   elm_widget_sub_object_add(obj, img);

   return img;
}

static void
_efl_ui_image_internal_sizing_eval(Evas_Object *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (!sd->img) return;

   if (eo_isa(sd->img, EDJE_OBJECT_CLASS))
     {
        evas_object_move(sd->img, sd->img_x, sd->img_y);
        evas_object_resize(sd->img, sd->img_w, sd->img_h);

        evas_object_move(sd->hit_rect, sd->img_x, sd->img_y);
        evas_object_resize(sd->hit_rect, sd->img_w, sd->img_h);
     }
   else
     {
        Evas_Coord x = 0, y = 0, w = 1, h = 1;

        int iw = 0, ih = 0, offset_x = 0, offset_y = 0;

        //1. Get the original image size (iw x ih)
        evas_object_image_size_get(sd->img, &iw, &ih);

        iw = ((double)iw) * sd->scale;
        ih = ((double)ih) * sd->scale;

        if (iw < 1) iw = 1;
        if (ih < 1) ih = 1;

        //2. Calculate internal image size (w x h)
        //   according to (iw x ih), (sd->img_w x sd->img_h), and scale_type
        switch (sd->scale_type)
          {
           case EFL_UI_IMAGE_SCALE_TYPE_NONE:
              w = iw;
              h = ih;
              break;
           case EFL_UI_IMAGE_SCALE_TYPE_FILL:
              w = sd->img_w;
              h = sd->img_h;
              break;
           case EFL_UI_IMAGE_SCALE_TYPE_FIT_INSIDE:
              w = sd->img_w;
              h = ((double)ih * w) / (double)iw;

              if (h > sd->img_h)
                {
                   h = sd->img_h;
                   w = ((double)iw * h) / (double)ih;
                }

              if (((!sd->scale_up) && (w > iw))
                  || ((!sd->scale_down) && (w < iw)))
                {
                   w = iw;
                   h = ih;
                }
              break;
           case EFL_UI_IMAGE_SCALE_TYPE_FIT_OUTSIDE:
              w = sd->img_w;
              h = ((double)ih * w) / (double)iw;
              if (h < sd->img_h)
                {
                   h = sd->img_h;
                   w = ((double)iw * h) / (double)ih;
                }

              if (((!sd->scale_up) && (w > iw))
                  || ((!sd->scale_down) && (w < iw)))
                {
                   w = iw;
                   h = ih;
                }
              break;
          }

        //3. Calculate offset according to align value
        offset_x = ((sd->img_w - w) * sd->align_x);
        offset_y = ((sd->img_h - h) * sd->align_y);

        x = sd->img_x + offset_x;
        y = sd->img_y + offset_y;

        //4. Fill, move, resize
        if (offset_x >= 0) offset_x = 0;
        if (offset_y >= 0) offset_y = 0;

        evas_object_image_fill_set(sd->img, offset_x, offset_y, w, h);

        if (offset_x < 0)
          {
             x = sd->img_x;
             w = sd->img_w;
          }
        if (offset_y < 0)
          {
             y = sd->img_y;
             h = sd->img_h;
          }

        evas_object_move(sd->img, x, y);
        evas_object_resize(sd->img, w, h);

        evas_object_move(sd->hit_rect, x, y);
        evas_object_resize(sd->hit_rect, w, h);
     }
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
_efl_ui_image_async_open_do(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   Eina_File *f;
   Async_Open_Data *todo, *done;
   void *map = NULL;
   unsigned int buf;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

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
_efl_ui_image_async_open_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   DBG("Async open thread was canceled");
   sd->async.th = NULL;
   sd->async_opening = EINA_FALSE;
   sd->async_failed = EINA_TRUE;
   // keep file, key for file_get()
}

static void
_efl_ui_image_async_open_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Object *obj = data;
   Eina_Stringshare *file, *key;
   Async_Open_Data *done;
   Eina_Bool ok;
   Eina_File *f;
   void *map;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   // async open thread can't be running now
   // locking anyways to be sure (memory barrier), see CID 1343345
   eina_spinlock_take(&sd->async.lck);

   sd->async.th = NULL;
   sd->async_failed = EINA_FALSE;

   if (sd->async.todo)
     {
        eina_spinlock_release(&sd->async.lck);
        sd->async.th = ecore_thread_run(_efl_ui_image_async_open_do,
                                        _efl_ui_image_async_open_done,
                                        _efl_ui_image_async_open_cancel, obj);

        return;
     }

   sd->async_opening = EINA_FALSE;

   done = sd->async.done;
   if (!done)
     {
        // done should be NULL only after cancel... not here
        ERR("Async open failed for an unknown reason.");
        sd->async_failed = EINA_TRUE;
        eina_spinlock_release(&sd->async.lck);
        eo_event_callback_call(obj, EFL_FILE_EVENT_ASYNC_ERROR, NULL);
        return;
     }

   DBG("Async open succeeded");
   sd->async.done = NULL;
   eina_spinlock_release(&sd->async.lck);
   key = done->key;
   map = done->map;
   f = done->f_open;
   ok = f && map;
   if (done->file)
     file = done->file;
   else
     file = f ? eina_file_filename_get(f) : NULL;

   if (ok)
     {
        if (sd->edje)
          ok = edje_object_mmap_set(sd->img, f, key);
        else
          ok = _efl_ui_image_smart_internal_file_set(obj, sd, file, f, key);
     }

   if (ok)
     {
        // TODO: Implement Efl.File async,opened event_info type
        sd->async_failed = EINA_FALSE;
        ELM_SAFE_FREE(sd->async.file, eina_stringshare_del);
        ELM_SAFE_FREE(sd->async.key, eina_stringshare_del);
        eo_event_callback_call(obj, EFL_FILE_EVENT_ASYNC_OPENED, NULL);
        _efl_ui_image_internal_sizing_eval(obj, sd);
     }
   else
     {
        // TODO: Implement Efl.File async,error event_info type
        sd->async_failed = EINA_TRUE;
        // keep file,key around for file_get
        eo_event_callback_call(obj, EFL_FILE_EVENT_ASYNC_ERROR, NULL);
     }

   // close f, map and free strings
   _async_open_data_free(done);
}

static Eina_Bool
_efl_ui_image_async_file_set(Eo *obj, Efl_Ui_Image_Data *sd,
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
        sd->async.th = ecore_thread_run(_efl_ui_image_async_open_do,
                                        _efl_ui_image_async_open_done,
                                        _efl_ui_image_async_open_cancel, obj);
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
_efl_ui_image_edje_file_set(Evas_Object *obj,
                         const char *file,
                         const Eina_File *f,
                         const char *group)
{
   Evas_Object *pclip;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

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
     return _efl_ui_image_async_file_set(obj, sd, file, f, group);

   /* FIXME: do i want to update icon on file change ? */
   _efl_ui_image_internal_sizing_eval(obj, sd);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_image_efl_image_smooth_scale_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Bool smooth)
{
   if (sd->edje) return;

   evas_object_image_smooth_scale_set(sd->img, smooth);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_image_smooth_scale_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje) return EINA_FALSE;

   return evas_object_image_smooth_scale_get(sd->img);
}

static Eina_Bool
_efl_ui_image_drag_n_drop_cb(void *elm_obj,
      Evas_Object *obj,
      Elm_Selection_Data *drop)
{
   Eina_Bool ret = EINA_FALSE;
   ret = efl_file_set(obj, drop->data, NULL);
   if (ret)
     {
        DBG("dnd: %s, %s, %s", elm_widget_type_get(elm_obj),
              SIG_DND, (char *)drop->data);

        eo_event_callback_call(elm_obj, EFL_UI_IMAGE_EVENT_DROP, drop->data);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_image_evas_object_smart_add(Eo *obj, Efl_Ui_Image_Data *priv)
{
   evas_obj_smart_add(eo_super(obj, MY_CLASS));
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
   priv->scale_up = EINA_TRUE;
   priv->scale_down = EINA_TRUE;
   priv->align_x = 0.5;
   priv->align_y = 0.5;
   eina_spinlock_new(&priv->async.lck);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   _efl_ui_image_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_del(Eo *obj, Efl_Ui_Image_Data *sd)
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
             evas_obj_smart_del(eo_super(obj, MY_CLASS));
             return;
          }
        sd->async.th = NULL;
     }

   _async_open_data_free(sd->async.done);
   _async_open_data_free(sd->async.todo);
   eina_stringshare_del(sd->async.file);
   eina_stringshare_del(sd->async.key);

   evas_obj_smart_del(eo_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_image_evas_object_smart_move(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Coord x, Evas_Coord y)
{
   evas_obj_smart_move(eo_super(obj, MY_CLASS), x, y);

   if ((sd->img_x == x) && (sd->img_y == y)) return;
   sd->img_x = x;
   sd->img_y = y;

   /* takes care of moving */
   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_resize(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Coord w, Evas_Coord h)
{
   evas_obj_smart_resize(eo_super(obj, MY_CLASS), w, h);

   if ((sd->img_w == w) && (sd->img_h == h)) return;

   sd->img_w = w;
   sd->img_h = h;

   /* takes care of resizing */
   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_show(Eo *obj, Efl_Ui_Image_Data *sd)
{
   sd->show = EINA_TRUE;
   if (sd->preload_status == EFL_UI_IMAGE_PRELOADING) return;

   evas_obj_smart_show(eo_super(obj, MY_CLASS));

   evas_object_show(sd->img);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_hide(Eo *obj, Efl_Ui_Image_Data *sd)
{
   evas_obj_smart_hide(eo_super(obj, MY_CLASS));

   sd->show = EINA_FALSE;
   evas_object_hide(sd->img);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_member_add(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Object *member)
{
   evas_obj_smart_member_add(eo_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_color_set(Eo *obj, Efl_Ui_Image_Data *sd, int r, int g, int b, int a)
{
   evas_obj_smart_color_set(eo_super(obj, MY_CLASS), r, g, b, a);

   evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
   evas_object_color_set(sd->img, r, g, b, a);
   if (sd->prev_img) evas_object_color_set(sd->prev_img, r, g, b, a);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_clip_set(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Object *clip)
{
   evas_obj_smart_clip_set(eo_super(obj, MY_CLASS), clip);

   evas_object_clip_set(sd->img, clip);
   if (sd->prev_img) evas_object_clip_set(sd->prev_img, clip);
}

EOLIAN static void
_efl_ui_image_evas_object_smart_clip_unset(Eo *obj, Efl_Ui_Image_Data *sd)
{
   evas_obj_smart_clip_unset(eo_super(obj, MY_CLASS));

   evas_object_clip_unset(sd->img);
   if (sd->prev_img) evas_object_clip_unset(sd->prev_img);
}

EOLIAN static Eina_Bool
_efl_ui_image_elm_widget_theme_apply(Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (sd->stdicon)
     _elm_theme_object_icon_set(obj, sd->stdicon, elm_widget_style_get(obj));

   int_ret = elm_obj_widget_theme_apply(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   _efl_ui_image_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   eo_event_callback_call(obj, EFL_UI_EVENT_CLICKED, NULL);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_image_elm_widget_event(Eo *obj, Efl_Ui_Image_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

// TODO: remove this function after using the widget's scale value instead of image's scale value, 

static void
_efl_ui_image_internal_scale_set(Evas_Object *obj, Efl_Ui_Image_Data *sd, double scale)
{
   sd->scale = scale;
   _efl_ui_image_internal_sizing_eval(obj, sd);
}

static void
_efl_ui_image_sizing_eval(Eo *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w = 0, h = 0;
   double ts;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   efl_image_smooth_scale_set(obj, sd->smooth);

   if (sd->no_scale)
     _efl_ui_image_internal_scale_set(obj, sd, 1.0);
   else
     _efl_ui_image_internal_scale_set(obj, sd, elm_widget_scale_get(obj) * elm_config_scale_get());

   ts = sd->scale;
   sd->scale = 1.0;
   efl_gfx_view_size_get(obj, &w, &h);

   sd->scale = ts;
   efl_gfx_size_hint_combined_min_get(obj, &minw, &minh);

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
_efl_ui_image_file_set_do(Evas_Object *obj)
{
   Evas_Object *pclip = NULL;
   int w = 0, h = 0;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

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
        efl_gfx_view_size_get((Eo *) obj, &w, &h);
        evas_object_image_load_size_set(sd->img, w, h);
     }
}

EAPI Evas_Object *
elm_image_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_efl_ui_image_eo_base_constructor(Eo *obj, Efl_Ui_Image_Data *_pd EINA_UNUSED)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   evas_obj_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_obj_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_IMAGE);

   return obj;
}

EOLIAN Eina_Bool
_efl_ui_image_efl_file_mmap_set(Eo *obj, Efl_Ui_Image_Data *sd,
                             const Eina_File *file, const char *key)
{
   Eina_Bool ret = EINA_FALSE;

   if (sd->remote) _elm_url_cancel(sd->remote);
   sd->remote = NULL;

   if (!sd->async_enable)
     ret = _efl_ui_image_smart_internal_file_set(obj, sd, eina_file_filename_get(file), file, key);
   else
     ret = _efl_ui_image_async_file_set(obj, sd, eina_file_filename_get(file), file, key);

   return ret;
}

static Eina_Bool
_efl_ui_image_smart_internal_file_set(Eo *obj, Efl_Ui_Image_Data *sd,
                                   const char *file, const Eina_File *f, const char *key)
{
   Evas_Load_Error err;

   if (eina_str_has_extension(file, ".edj"))
     return _efl_ui_image_edje_file_set(obj, file, f, key);

   _efl_ui_image_file_set_do(obj);

   if (f)
     evas_object_image_mmap_set(sd->img, f, key);
   else
     evas_object_image_file_set(sd->img, file, key);

   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        if (file || f)
          {
             if (key)
               ERR("Failed to load image '%s' '%s': %s. (%p)",
                   file ? file : eina_file_filename_get(f), key,
                   evas_load_error_str(err), obj);
             else
                ERR("Failed to load image '%s': %s. (%p)",
                    file ? file : eina_file_filename_get(f),
                    evas_load_error_str(err), obj);
          }
        else
          {
             ERR("NULL image file passed! (%p)", obj);
          }
        return EINA_FALSE;
     }

   if (sd->preload_status != EFL_UI_IMAGE_PRELOAD_DISABLED)
     {
        evas_object_hide(sd->img);
        sd->preload_status = EFL_UI_IMAGE_PRELOADING;
        evas_object_image_preload(sd->img, EINA_FALSE);
     }

   _efl_ui_image_internal_sizing_eval(obj, sd);

   return EINA_TRUE;
}

static void
_efl_ui_image_smart_download_done(void *data, Elm_Url *url, Eina_Binbuf *download)
{
   Eo *obj = data;
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Eina_File *f;
   size_t length;
   Eina_Bool ret = EINA_FALSE;

   free(sd->remote_data);
   length = eina_binbuf_length_get(download);
   sd->remote_data = eina_binbuf_string_steal(download);
   f = eina_file_virtualize(_elm_url_get(url),
                            sd->remote_data, length,
                            EINA_FALSE);
   ret = _efl_ui_image_smart_internal_file_set(obj, sd, _elm_url_get(url), f, sd->key);
   eina_file_close(f);

   sd->remote = NULL;
   if (!ret)
     {
        Efl_Ui_Image_Error err = { 0, EINA_TRUE };

        free(sd->remote_data);
        sd->remote_data = NULL;
        eo_event_callback_call(obj, EFL_UI_IMAGE_EVENT_DOWNLOAD_ERROR, &err);
     }
   else
     {
        if (sd->preload_status != EFL_UI_IMAGE_PRELOAD_DISABLED)
          {
             sd->preload_status = EFL_UI_IMAGE_PRELOADING;
             evas_object_image_preload(sd->img, EINA_FALSE);
          }

        eo_event_callback_call(obj, EFL_UI_IMAGE_EVENT_DOWNLOAD_DONE, NULL);
     }

   ELM_SAFE_FREE(sd->key, eina_stringshare_del);
}

static void
_efl_ui_image_smart_download_cancel(void *data, Elm_Url *url EINA_UNUSED, int error)
{
   Eo *obj = data;
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   Efl_Ui_Image_Error err = { error, EINA_FALSE };

   eo_event_callback_call(obj, EFL_UI_IMAGE_EVENT_DOWNLOAD_ERROR, &err);

   sd->remote = NULL;
   ELM_SAFE_FREE(sd->key, eina_stringshare_del);
}

static void
_efl_ui_image_smart_download_progress(void *data, Elm_Url *url EINA_UNUSED, double now, double total)
{
   Eo *obj = data;
   Efl_Ui_Image_Progress progress;

   progress.now = now;
   progress.total = total;
   eo_event_callback_call(obj, EFL_UI_IMAGE_EVENT_DOWNLOAD_PROGRESS, &progress);
}

static const char *remote_uri[] = {
  "http://", "https://", "ftp://"
};

EOLIAN static Eina_Bool
_efl_ui_image_efl_file_file_set(Eo *obj, Efl_Ui_Image_Data *sd, const char *file, const char *key)
{
   Eina_Bool ret = EINA_FALSE;
   unsigned int i;

   if (sd->remote) _elm_url_cancel(sd->remote);
   sd->remote = NULL;

   if (sd->anim)
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
        sd->play = EINA_FALSE;
        sd->anim = EINA_FALSE;
     }

   for (i = 0; i < sizeof (remote_uri) / sizeof (remote_uri[0]); ++i)
     if (file && !strncmp(remote_uri[i], file, strlen(remote_uri[i])))
       {
          // Found a remote target !
          evas_object_hide(sd->img);
          sd->remote = _elm_url_download(file,
                                        _efl_ui_image_smart_download_done,
                                        _efl_ui_image_smart_download_cancel,
                                        _efl_ui_image_smart_download_progress,
                                        obj);
          if (sd->remote)
            {
               eo_event_callback_call
                     (obj, EFL_UI_IMAGE_EVENT_DOWNLOAD_START, NULL);
               eina_stringshare_replace(&sd->key, key);
               return EINA_TRUE;
            }
          break;
       }

   if (!sd->async_enable)
     ret = _efl_ui_image_smart_internal_file_set(obj, sd, file, NULL, key);
   else
     ret = _efl_ui_image_async_file_set(obj, sd, file, NULL, key);

   return ret;
}

EOLIAN static void
_efl_ui_image_edje_object_signal_emit(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, const char *emission, const char *source)
{
   if (sd->edje)
     edje_object_signal_emit(sd->img, emission, source);
}

EOLIAN static void
_efl_ui_image_edje_object_size_min_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_min_get(sd->img, w, h);
   else
     efl_gfx_size_hint_combined_min_get(sd->img, w, h);
}

EOLIAN static void
_efl_ui_image_edje_object_size_max_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_max_get(sd->img, w, h);
   else
     evas_object_size_hint_max_get(sd->img, w, h);
}

EOLIAN static void
_efl_ui_image_edje_object_calc_force(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     edje_object_calc_force(sd->img);
}

EOLIAN static void
_efl_ui_image_edje_object_size_min_calc(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int *w, int *h)
{
   if (sd->edje)
     edje_object_size_min_calc(sd->img, w, h);
   else
     efl_gfx_size_hint_combined_min_get(sd->img, w, h);
}

EOLIAN static void
_efl_ui_image_efl_file_file_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, const char **file, const char **key)
{
   if (sd->async_enable && (sd->async_opening || sd->async_failed))
     {
        if (file) *file = sd->async.file;
        if (key) *key = sd->async.key;
        return;
     }

   efl_file_get(sd->img, file, key);
}

static Eina_Bool
_efl_ui_image_efl_file_async_wait(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
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
_efl_ui_image_efl_file_async_set(Eo *obj, Efl_Ui_Image_Data *pd, Eina_Bool async)
{
   if (pd->async_enable == async)
     return;

   pd->async_enable = async;
   if (!async)
     _efl_ui_image_efl_file_async_wait(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_file_async_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->async_enable;
}

EOLIAN static void
_efl_ui_image_efl_gfx_view_view_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int *w, int *h)
{
   int tw, th;
   int cw = 0, ch = 0;

   if (w) *w = 0;
   if (h) *h = 0;

   if (eo_isa(sd->img, EDJE_OBJECT_CLASS))
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


EAPI void
elm_image_prescale_set(Evas_Object *obj,
                       int size)
{
   EFL_UI_IMAGE_CHECK(obj);
   efl_image_load_size_set(obj, size, size);
}

EOLIAN static void
_efl_ui_image_efl_image_load_load_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int w, int h)
{
   if (w > h)
      sd->load_size = w;
   else
      sd->load_size = h;
}

EAPI int
elm_image_prescale_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) 0;

   int w = 0;
   efl_image_load_size_get((Eo *)obj, &w, NULL);
   return w;
}

EOLIAN static void
_efl_ui_image_efl_image_load_load_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, int *w, int *h)
{
   if (w) *w = sd->load_size;
   if (h) *h = sd->load_size;
}

EOLIAN static void
_efl_ui_image_efl_orientation_orientation_set(Eo *obj, Efl_Ui_Image_Data *sd, Efl_Orient orient)
{
   if (sd->edje) return;
   if (sd->orient == orient) return;

   efl_orientation_set(sd->img, orient);

   sd->orient = orient;
   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Efl_Orient
_efl_ui_image_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->orient;
}


EOLIAN static void
_efl_ui_image_efl_flipable_flip_set(Eo *obj, Efl_Ui_Image_Data *sd, Efl_Flip flip)
{
   if (sd->edje) return;
   if (sd->flip == flip) return;

   efl_flip_set(sd->img, flip);

   sd->flip = flip;
   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Efl_Flip
_efl_ui_image_efl_flipable_flip_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->flip;
}

/**
 * Turns on editing through drag and drop and copy and paste.
 */
EOLIAN static void
_efl_ui_image_evas_draggable_interface_drag_target_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool edit)
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
           _efl_ui_image_drag_n_drop_cb, obj);
   else
     elm_drop_target_del
       (obj, ELM_SEL_FORMAT_IMAGE,
           NULL, NULL,
           NULL, NULL,
           NULL, NULL,
           _efl_ui_image_drag_n_drop_cb, obj);
}

EOLIAN static Eina_Bool
_efl_ui_image_evas_draggable_interface_drag_target_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->edit;
}

EAPI Eina_Bool
elm_image_animated_available_get(const Evas_Object *obj)
{
   Eina_Bool ret;
   ret = efl_player_playable_get(obj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_playable_get(Eo *obj, Efl_Ui_Image_Data *sd)
{
   if (sd->edje) return EINA_FALSE;

   return evas_object_image_animated_get(elm_image_object_get(obj));
}

static void
_efl_ui_image_animated_set_internal(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool anim)
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
_efl_ui_image_animated_get_internal(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return edje_object_animation_get(sd->img);
   return sd->anim;
}

EAPI void
elm_image_animated_set(Evas_Object *obj, Eina_Bool anim)
{
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _efl_ui_image_animated_set_internal(obj, sd, anim);
}

EAPI Eina_Bool
elm_image_animated_get(const Evas_Object *obj)
{
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return _efl_ui_image_animated_get_internal(obj, sd);
}

static void
_efl_ui_image_animated_play_set_internal(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool play)
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
            (sd->frame_duration, _efl_ui_image_animate_cb, obj);
     }
   else
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
     }
}

static Eina_Bool
_efl_ui_image_animated_play_get_internal(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return edje_object_play_get(sd->img);
   return sd->play;
}

EAPI void
elm_image_animated_play_set(Elm_Image *obj, Eina_Bool play)
{
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _efl_ui_image_animated_play_set_internal(obj, sd, play);
}

EAPI Eina_Bool
elm_image_animated_play_get(const Elm_Image *obj)
{
   Efl_Ui_Image_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return _efl_ui_image_animated_play_get_internal(obj, sd);
}

EOLIAN static void
_efl_ui_image_efl_player_play_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool play)
{
   if (play && !_efl_ui_image_animated_get_internal(obj, sd))
     _efl_ui_image_animated_set_internal(obj, sd, play);
   _efl_ui_image_animated_play_set_internal(obj, sd, play);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_play_get(Eo *obj, Efl_Ui_Image_Data *sd)
{
   return _efl_ui_image_animated_play_get_internal(obj, sd);
}

static void
_efl_ui_image_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static void
_efl_ui_image_scale_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd EINA_UNUSED, Efl_Ui_Image_Scale_Type scale_type)
{
   if (scale_type == sd->scale_type) return;

   sd->scale_type = scale_type;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static Efl_Ui_Image_Scale_Type
_efl_ui_image_scale_type_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->scale_type;
}

EOLIAN static void
_efl_ui_image_scalable_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool up, Eina_Bool down)
{
   if ((up == sd->scale_up) && (down == sd->scale_down)) return;

   sd->scale_up = !!up;
   sd->scale_down = !!down;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_efl_ui_image_scalable_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Bool *scale_up, Eina_Bool *scale_down)
{
   if (scale_up) *scale_up = sd->scale_up;
   if (scale_down) *scale_down = sd->scale_down;
}

EOLIAN static void
_efl_ui_image_align_set(Eo *obj, Efl_Ui_Image_Data *sd, double align_x, double align_y)
{
   if (align_x > 1.0)
     align_x = 1.0;
   else if (align_x < 0.0)
     align_x = 0.0;

   if (align_y > 1.0)
     align_y = 1.0;
   else if (align_y < 0.0)
     align_y = 0.0;

   if ((align_x == sd->align_x) && (align_y == sd->align_y)) return;

   sd->align_x = align_x;
   sd->align_y = align_y;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EOLIAN static void
_efl_ui_image_align_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, double *align_x, double *align_y)
{
   if (align_x) *align_x = sd->align_x;
   if (align_y) *align_y = sd->align_y;
}

// A11Y

EOLIAN static void
_efl_ui_image_elm_interface_atspi_image_extents_get(Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED, Eina_Bool screen_coords, int *x, int *y, int *w, int *h)
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
_efl_ui_image_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
        { "activate", "activate", NULL, _key_action_activate },
        { NULL, NULL, NULL, NULL },
   };
   return &atspi_actions[0];
}

static Eina_Bool
_icon_standard_set(Evas_Object *obj, const char *name)
{
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   if (_elm_theme_object_icon_set(obj, name, "default"))
     {
        /* TODO: elm_unneed_efreet() */
        sd->freedesktop.use = EINA_FALSE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_icon_freedesktop_set(Evas_Object *obj, const char *name, int size)
{
   const char *path;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   elm_need_efreet();

   if (icon_theme == NON_EXISTING) return EINA_FALSE;

   if (!icon_theme)
     {
        Efreet_Icon_Theme *theme;
        /* TODO: Listen for EFREET_EVENT_ICON_CACHE_UPDATE */
        theme = efreet_icon_theme_find(elm_config_icon_theme_get());
        if (!theme)
          {
             const char **itr;
             static const char *themes[] = {
                "gnome", "Human", "oxygen", "hicolor", NULL
             };
             for (itr = themes; *itr; itr++)
               {
                  theme = efreet_icon_theme_find(*itr);
                  if (theme) break;
               }
          }

        if (!theme)
          {
             icon_theme = NON_EXISTING;
             return EINA_FALSE;
          }
        else
          icon_theme = eina_stringshare_add(theme->name.internal);
     }
   path = efreet_icon_path_find(icon_theme, name, size);
   sd->freedesktop.use = !!path;
   if (sd->freedesktop.use)
     {
        sd->freedesktop.requested_size = size;
        efl_file_set(obj, path, NULL);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline int
_icon_size_min_get(Evas_Object *image)
{
   int w, h;

   evas_object_geometry_get(image, NULL, NULL, &w, &h);

   return MAX(16, MIN(w, h));
}

/* FIXME: move this code to ecore */
#ifdef _WIN32
static Eina_Bool
_path_is_absolute(const char *path)
{
   //TODO: Check if this works with all absolute paths in windows
   return (isalpha(*path)) && (*(path + 1) == ':') &&
           ((*(path + 2) == '\\') || (*(path + 2) == '/'));
}

#else
static Eina_Bool
_path_is_absolute(const char *path)
{
   return *path == '/';
}

#endif

static Eina_Bool
_internal_efl_ui_image_icon_set(Evas_Object *obj, const char *name, Eina_Bool *fdo)
{
   char *tmp;
   Eina_Bool ret = EINA_FALSE;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   /* try locating the icon using the specified theme */
   if (!strcmp(ELM_CONFIG_ICON_THEME_ELEMENTARY, elm_config_icon_theme_get()))
     {
        ret = _icon_standard_set(obj, name);
        if (ret && fdo) *fdo = EINA_FALSE;
     }
   else
     {
        ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
        if (ret && fdo) *fdo = EINA_TRUE;
     }

   if (ret)
     {
        eina_stringshare_replace(&sd->stdicon, name);
        _efl_ui_image_sizing_eval(obj);
        return EINA_TRUE;
     }

   if (_path_is_absolute(name))
     {
        if (fdo)
          *fdo = EINA_FALSE;
        return efl_file_set(obj, name, NULL);
     }

   /* if that fails, see if icon name is in the format size/name. if so,
		try locating a fallback without the size specification */
   if (!(tmp = strchr(name, '/'))) return EINA_FALSE;
   ++tmp;
   if (*tmp) return _internal_efl_ui_image_icon_set(obj, tmp, fdo);
   /* give up */
   return EINA_FALSE;
}

static void
_efl_ui_image_icon_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EFL_UI_IMAGE_DATA_GET(data, sd);
   const char *refup = eina_stringshare_ref(sd->stdicon);
   Eina_Bool fdo = EINA_FALSE;

   if (!_internal_efl_ui_image_icon_set(obj, sd->stdicon, &fdo) || (!fdo))
     evas_object_event_callback_del_full
       (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_icon_resize_cb, data);
   eina_stringshare_del(refup);
}

EOLIAN static Eina_Bool
_efl_ui_image_icon_set(Eo *obj, Efl_Ui_Image_Data *_pd EINA_UNUSED, const char *name)
{
   Eina_Bool fdo = EINA_FALSE;

   if (!name) return EINA_FALSE;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_icon_resize_cb, obj);

   Eina_Bool int_ret = _internal_efl_ui_image_icon_set(obj, name, &fdo);

   if (fdo)
     evas_object_event_callback_add
       (obj, EVAS_CALLBACK_RESIZE, _efl_ui_image_icon_resize_cb, obj);

   return int_ret;
}

EOLIAN static const char*
_efl_ui_image_icon_get(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->stdicon;
}

EAPI void
elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth)
{
   efl_image_smooth_scale_set(obj, smooth);
   _efl_ui_image_sizing_eval(obj);
}

EAPI Eina_Bool
elm_image_smooth_get(const Evas_Object *obj)
{
   return efl_image_smooth_scale_get(obj);
}

// A11Y - END

/* Legacy deprecated functions */
EAPI void
elm_image_editable_set(Evas_Object *obj, Eina_Bool edit)
{
   evas_draggable_interface_drag_target_set(obj, edit);
}

EAPI Eina_Bool
elm_image_editable_get(const Evas_Object *obj)
{
   return evas_draggable_interface_drag_target_get(obj);
}

EAPI Eina_Bool
elm_image_file_set(Evas_Object *obj, const char *file, const char *group)
{
   Eina_Bool ret = EINA_FALSE;

   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   ret = efl_file_set(obj, file, group);
   _efl_ui_image_sizing_eval(obj);
   return ret;
}

EAPI void
elm_image_file_get(const Eo *obj, const char **file, const char **group)
{
   efl_file_get((Eo *) obj, file, group);
}

EAPI Eina_Bool
elm_image_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group)
{
   Eina_Bool ret = EINA_FALSE;

   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   ret = efl_file_mmap_set(obj, file, group);
   return ret;
}

EAPI Eina_Bool
elm_image_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key)
{
   Evas_Load_Error err;

   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   _efl_ui_image_file_set_do(obj);

   evas_object_image_memfile_set
     (sd->img, (void *)img, size, (char *)format, (char *)key);

   sd->preload_status = EFL_UI_IMAGE_PRELOADING;
   evas_object_image_preload(sd->img, EINA_FALSE);

   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        if (img)
          ERR("Failed to load image from memory block (" FMT_SIZE_T
              " bytes): %s (%p)", size, evas_load_error_str(err), sd->img);
        else
          ERR("NULL image data passed (%p)", sd->img);
        return EINA_FALSE;
     }

   _efl_ui_image_internal_sizing_eval(obj, sd);

   return EINA_TRUE;
}

// TODO: use the widget's scale value instead of image's scale value.

EAPI void
elm_image_scale_set(Evas_Object *obj,
                     double       scale)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   sd->scale = scale;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI double
elm_image_scale_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) -1;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   return sd->scale;
}

EAPI void
elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   fill_outside = !!fill_outside;

   if (sd->fill_inside == !fill_outside) return;

   sd->fill_inside = !fill_outside;

   if (sd->aspect_fixed)
     {
        if (sd->fill_inside) sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_INSIDE;
        else sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
     }
   else
     sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FILL;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI Eina_Bool
elm_image_fill_outside_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   return !sd->fill_inside;
}

// TODO: merge preload and async code
EAPI void
elm_image_preload_disabled_set(Evas_Object *obj, Eina_Bool disable)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   if (sd->edje || !sd->img) return;

   if (disable)
     {
        if (sd->preload_status == EFL_UI_IMAGE_PRELOADING)
          {
             evas_object_image_preload(sd->img, disable);
             if (sd->show) evas_object_show(sd->img);
             ELM_SAFE_FREE(sd->prev_img, evas_object_del);
          }
        sd->preload_status = EFL_UI_IMAGE_PRELOAD_DISABLED;
     }
   else if (sd->preload_status == EFL_UI_IMAGE_PRELOAD_DISABLED)
    {
       sd->preload_status = EFL_UI_IMAGE_PRELOADING;
       evas_object_image_preload(sd->img, disable);
    }
}

EAPI void
elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient)
{
   Efl_Orient dir;
   Efl_Flip flip;

   EFL_UI_IMAGE_DATA_GET(obj, sd);
   sd->image_orient = orient;

   switch (orient)
     {
      case EVAS_IMAGE_ORIENT_0:
         dir = EFL_ORIENT_0;
         flip = EFL_FLIP_NONE;
         break;
      case EVAS_IMAGE_ORIENT_90:
         dir = EFL_ORIENT_90;
         flip = EFL_FLIP_NONE;
         break;
      case EVAS_IMAGE_ORIENT_180:
         dir = EFL_ORIENT_180;
         flip = EFL_FLIP_NONE;
         break;
      case EVAS_IMAGE_ORIENT_270:
         dir = EFL_ORIENT_270;
         flip = EFL_FLIP_NONE;
         break;
      case EVAS_IMAGE_FLIP_HORIZONTAL:
         dir = EFL_ORIENT_0;
         flip = EFL_FLIP_HORIZONTAL;
         break;
      case EVAS_IMAGE_FLIP_VERTICAL:
         dir = EFL_ORIENT_0;
         flip = EFL_FLIP_VERTICAL;
         break;
      case EVAS_IMAGE_FLIP_TRANSVERSE:
         dir = EFL_ORIENT_270;
         flip = EFL_FLIP_HORIZONTAL;
         break;
      case EVAS_IMAGE_FLIP_TRANSPOSE:
         dir = EFL_ORIENT_270;
         flip = EFL_FLIP_VERTICAL;
         break;
      default:
         dir = EFL_ORIENT_0;
         flip = EFL_FLIP_NONE;
         break;
     }

   efl_orientation_set(obj, dir);
   efl_flip_set(obj, flip);
}

EAPI Elm_Image_Orient
elm_image_orient_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) ELM_IMAGE_ORIENT_NONE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   return sd->image_orient;
}

EAPI Evas_Object*
elm_image_object_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) NULL;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->img;
}

EAPI void
elm_image_object_size_get(const Evas_Object *obj, int *w, int *h)
{
   efl_gfx_view_size_get(obj, w, h);
}

EAPI void
elm_image_resize_down_set(Evas_Object *obj, Eina_Bool resize_down)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   resize_down = !!resize_down;

   if (sd->scale_down == resize_down) return;

   sd->scale_down = resize_down;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI Eina_Bool
elm_image_resize_down_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->scale_down;
}

EAPI void
elm_image_resize_up_set(Evas_Object *obj, Eina_Bool resize_up)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   resize_up = !!resize_up;

   if (sd->scale_up == resize_up) return;

   sd->scale_up = resize_up;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI Eina_Bool
elm_image_resize_up_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->scale_up;
}

EAPI void
elm_image_sizing_eval(Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj);
   _efl_ui_image_sizing_eval(obj);
}

EAPI void
elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   sd->no_scale = no_scale;

   _efl_ui_image_sizing_eval(obj);
}

EAPI Eina_Bool
elm_image_no_scale_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->no_scale;
}

EAPI void
elm_image_resizable_set(Evas_Object *obj, Eina_Bool up, Eina_Bool down)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   sd->scale_up = !!up;
   sd->scale_down = !!down;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI void
elm_image_resizable_get(const Evas_Object *obj, Eina_Bool *size_up, Eina_Bool *size_down)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   if (size_up) *size_up = sd->scale_up;
   if (size_down) *size_down = sd->scale_down;
}

EAPI void
elm_image_fill_inside_set(Evas_Object *obj, Eina_Bool fill_inside)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   fill_inside = !!fill_inside;

   if (sd->fill_inside == fill_inside) return;

   sd->fill_inside = fill_inside;

   if (sd->aspect_fixed)
     {
        if (sd->fill_inside) sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_INSIDE;
        else sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
     }
   else
     sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FILL;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI Eina_Bool
elm_image_fill_inside_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->fill_inside;
}

EAPI void
elm_image_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   fixed = !!fixed;
   if (sd->aspect_fixed == fixed) return;

   sd->aspect_fixed = fixed;

   if (sd->aspect_fixed)
     {
        if (sd->fill_inside) sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_INSIDE;
        else sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
     }
   else
     sd->scale_type = EFL_UI_IMAGE_SCALE_TYPE_FILL;

   _efl_ui_image_internal_sizing_eval(obj, sd);
}

EAPI Eina_Bool
elm_image_aspect_fixed_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->aspect_fixed;
}

#include "efl_ui_image.eo.c"
