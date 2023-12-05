#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_IMAGE_PROTECTED
#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_COMPONENT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_LAYOUT_CALC_PROTECTED
#define EFL_INPUT_CLICKABLE_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_widget_image.h"

#define FMT_SIZE_T "%zu"

#define MY_CLASS EFL_UI_IMAGE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Image"

#define NON_EXISTING (void *)-1
static const char *icon_theme = NULL;

static const char SIG_DND[] = "drop";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_DOWNLOAD_START[] = "download,start";
static const char SIG_DOWNLOAD_PROGRESS[] = "download,progress";
static const char SIG_DOWNLOAD_DONE[] = "download,done";
static const char SIG_DOWNLOAD_ERROR[] = "download,error";
static const char SIG_LOAD_OPEN[] = "load,open";
static const char SIG_LOAD_READY[] = "load,ready";
static const char SIG_LOAD_ERROR[] = "load,error";
static const char SIG_LOAD_CANCEL[] = "load,cancel";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DND, ""},
   {SIG_CLICKED, ""},
   {SIG_DOWNLOAD_START, ""},
   {SIG_DOWNLOAD_PROGRESS, ""},
   {SIG_DOWNLOAD_DONE, ""},
   {SIG_DOWNLOAD_ERROR, ""},
   {SIG_LOAD_OPEN, "Triggered when the file has been opened (image size is known)"},
   {SIG_LOAD_READY, "Triggered when the image file is ready for display"},
   {SIG_LOAD_ERROR, "Triggered whenener an I/O or decoding error occurred"},
   {SIG_LOAD_CANCEL, "Triggered whenener async I/O was cancelled"},
   {NULL, NULL}
};

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);
static Eina_Error _efl_ui_image_smart_internal_file_set(Eo *obj, Efl_Ui_Image_Data *sd);
static void _efl_ui_image_remote_copier_cancel(Eo *obj, Efl_Ui_Image_Data *sd);
static void _efl_ui_image_model_properties_changed_cb(void *data, const Efl_Event *event);
static void _on_size_hints_changed(void *data, const Efl_Event *e);
static Eina_Bool _efl_ui_image_download(Eo *obj, Efl_Ui_Image_Data *sd, const char *url);
static void _update_viewmodel(Eo *obj, Efl_Ui_Image_Data *pd);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

typedef struct _Async_Open_Data Async_Open_Data;

struct _Async_Open_Data
{
   Eo               *obj;
   Eina_Stringshare *file, *key;
   Eina_File        *f_set, *f_open;
   void             *map;
};

static void
_prev_img_del(Efl_Ui_Image_Data *sd)
{
   elm_widget_sub_object_del(sd->self, sd->prev_img);
   evas_object_smart_member_del(sd->prev_img);
   evas_object_del(sd->prev_img);
   sd->prev_img = NULL;
}

static void
_recover_status(Eo *obj, Efl_Ui_Image_Data *sd)
{
   int r, g, b, a;
   Evas_Object *pclip = efl_canvas_object_clipper_get(obj);
   if (pclip) efl_canvas_object_clipper_set(sd->img, pclip);

   efl_gfx_color_get(obj, &r, &g, &b, &a);
   efl_gfx_color_set(sd->img, r, g, b, a);
   efl_gfx_entity_visible_set(sd->img, sd->show);
}

static void
_on_image_preloaded(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj,
                    void *event EINA_UNUSED)
{
   Efl_Ui_Image_Data *sd = data;
   Evas_Load_Error err;

   sd->preload_status = EFL_UI_IMAGE_PRELOADED;
   if (sd->show) evas_object_show(obj);
   _prev_img_del(sd);
   err = evas_object_image_load_error_get(obj);
   if (!err)
     {
        evas_object_smart_callback_call(sd->self, SIG_LOAD_READY, NULL);
        if (sd->autoplay) efl_player_playing_set(sd->self, EINA_TRUE);
     }
   else evas_object_smart_callback_call(sd->self, SIG_LOAD_ERROR, NULL);
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

   evas_object_smart_callback_call(data, "clicked", NULL);
}

static Eina_Bool
_efl_ui_image_animate_cb(void *data)
{
   EFL_UI_IMAGE_DATA_GET(data, sd);

   if (!sd->anim) return ECORE_CALLBACK_CANCEL;

   sd->cur_frame++;
   if ((sd->frame_count > 0) && (sd->cur_frame > sd->frame_count))
     {
        if (sd->playback_loop)
          sd->cur_frame = sd->cur_frame % sd->frame_count;
        else
          {
             sd->anim_timer = NULL;
             sd->cur_frame = 0;
             return EINA_FALSE;
          }
     }

   evas_object_image_animated_frame_set(sd->img, sd->cur_frame);

   sd->frame_duration = evas_object_image_animated_frame_duration_get
       (sd->img, sd->cur_frame, 0);

   if (sd->frame_duration > 0)
     ecore_timer_interval_set(sd->anim_timer, sd->frame_duration / sd->playback_speed);

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
_image_sizing_eval(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Object *img)
{
   Evas_Coord x = 0, y = 0, w = 1, h = 1;
   int ox, oy, ow, oh;
   Eina_Bool img_no_resize = EINA_FALSE;

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   if (efl_isa(img, EFL_CANVAS_LAYOUT_CLASS))
     {
        x = ox;
        y = oy;
        w = ow;
        h = oh;
        goto done;
     }
   else
     {
        double alignh = 0.5, alignv = 0.5;
        int iw = 0, ih = 0, offset_x = 0, offset_y = 0;

        //1. Get the original image size (iw x ih)
        evas_object_image_size_get(img, &iw, &ih);

        //Exception Case
        if ((iw == 0) || (ih == 0) || (ow == 0) || (oh == 0))
          {
             img_no_resize = EINA_TRUE;
          }

        iw = ((double)iw) * sd->scale;
        ih = ((double)ih) * sd->scale;

        if (iw < 1) iw = 1;
        if (ih < 1) ih = 1;

        //2. Calculate internal image size (w x h)
        //   according to (iw x ih), (ow x oh), and scale_type
        switch (sd->scale_type)
          {
           case EFL_GFX_IMAGE_SCALE_METHOD_NONE:
              w = iw;
              h = ih;
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_FILL:
              w = ow;
              h = oh;
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_FIT:
              w = ow;
              h = ((double)ih * w) / (double)iw;
              if (h > oh)
                {
                   h = oh;
                   w = ((double)iw * h) / (double)ih;
                }
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_FIT_WIDTH:
              w = ow;
              h = ((double)ih * w) / (double)iw;
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_FIT_HEIGHT:
              h = oh;
              w = ((double)iw * h) / (double)ih;
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_EXPAND:
              w = ow;
              h = ((double)ih * w) / (double)iw;
              if (h < oh)
                {
                   h = oh;
                   w = ((double)iw * h) / (double)ih;
                }
              break;
           case EFL_GFX_IMAGE_SCALE_METHOD_TILE:
              evas_object_geometry_get(obj, &x, &y, &w, &h);
              evas_object_image_fill_set(img, x, y, iw, ih);
              goto done;
          }
        if (((!sd->scale_up) && (w > iw))
            || ((!sd->scale_down) && (w < iw)))
          {
             w = iw;
          }
        if (((!sd->scale_up) && (h > ih))
            || ((!sd->scale_down) && (h < ih)))
          {
             h = ih;
          }
        //3. Calculate offset according to align value
        if (!elm_widget_is_legacy(sd->self))
          {
             offset_x = ((ow - w) * sd->align_x);
             offset_y = ((oh - h) * sd->align_y);
          }
        else
          {
             evas_object_size_hint_align_get(sd->self, &alignh, &alignv);
             if (EINA_DBL_EQ(alignh, EVAS_HINT_FILL)) alignh = 0.5;
             if (EINA_DBL_EQ(alignv, EVAS_HINT_FILL)) alignv = 0.5;

             offset_x = ((ow - w) * alignh);
             offset_y = ((oh - h) * alignv);
          }

        x = ox + offset_x;
        y = oy + offset_y;

        //4. Fill, move, resize
        if (offset_x >= 0) offset_x = 0;
        if (offset_y >= 0) offset_y = 0;

        evas_object_image_fill_set(img, offset_x, offset_y, w, h);

        if (ow - w < 0)
          {
             x = ox;
             w = ow;
          }
        if (oh - h < 0)
          {
             y = oy;
             h = oh;
          }
     }
done:
   if (img_no_resize)
     evas_object_geometry_set(img, 0, 0, 0, 0);
   else
     evas_object_geometry_set(img, x, y, w, h);

   evas_object_geometry_set(sd->hit_rect, x, y, w, h);
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
_efl_ui_image_async_open_do(void *data, Ecore_Thread *thread)
{
   Async_Open_Data *todo = data;
   Eina_File *f;
   void *map = NULL;
   size_t size;

   if (ecore_thread_check(thread)) return;

   if (todo->f_set) f = eina_file_dup(todo->f_set);
   else if (todo->file)
     {
        // blocking
        f = eina_file_open(todo->file, EINA_FALSE);
        if (!f) return;
     }
   else
     {
        CRI("Async open has no input file!");
        return;
     }
   if (ecore_thread_check(thread))
     {
        if (!todo->f_set) eina_file_close(f);
        return;
     }

   // Read just enough data for map to actually do something.
   size = eina_file_size_get(f);
   // Read and ensure all pages are in memory for sure first just
   // 1 byte per page will do. also keep a limit on how much we will
   // blindly load in here to let's say 32KB (Should be enough to get
   // image headers without getting to much data from the hard drive).
   size = size > 32 * 1024 ? 32 * 1024 : size;
   map = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   eina_file_map_populate(f, EINA_FILE_POPULATE, map, 0, size);

   if (ecore_thread_check(thread))
     {
        if (map) eina_file_map_free(f, map);
        if (!todo->f_set) eina_file_close(f);
        return;
     }
   todo->f_open = f;
   todo->map = map;
}

static void
_async_clear(Efl_Ui_Image_Data *sd)
{
   sd->async.th = NULL;
   sd->async.todo = NULL;
   eina_stringshare_del(sd->async.file);
   eina_stringshare_del(sd->async.key);
   sd->async.file = NULL;
   sd->async.key = NULL;
}

static void
_async_cancel(Efl_Ui_Image_Data *sd)
{
   if (sd->async.th)
     {
        ecore_thread_cancel(sd->async.th);
        ((Async_Open_Data *)(sd->async.todo))->obj = NULL;
        _async_clear(sd);
     }
}

static void
_efl_ui_image_async_open_cancel(void *data, Ecore_Thread *thread)
{
   Async_Open_Data *todo = data;

   DBG("Async open thread was canceled");
   if (todo->obj)
     {
        EFL_UI_IMAGE_DATA_GET(todo->obj, sd);
        if (sd)
          {
             evas_object_smart_callback_call(todo->obj, SIG_LOAD_CANCEL, NULL);
             if (thread == sd->async.th) _async_clear(sd);
          }
     }
   _async_open_data_free(todo);
}

static void
_efl_ui_image_async_open_done(void *data, Ecore_Thread *thread)
{
   Async_Open_Data *todo = data;
   const char *key;
   Eina_Bool ok;
   Eina_File *f;
   void *map;

   if (todo->obj)
     {
        EFL_UI_IMAGE_DATA_GET(todo->obj, sd);
        if (sd)
          {
             if (thread == sd->async.th)
               {
                  DBG("Async open succeeded");
                  _async_clear(sd);
                  key = todo->key;
                  map = todo->map;
                  f = todo->f_open;
                  ok = f && map;

                  if (ok)
                    {
                       efl_file_key_set(sd->self, key);
                       ok = !efl_file_mmap_set(sd->self, f);
                       if (ok)
                         {
                            if (sd->edje)
                              {
                                 _prev_img_del(sd);
                                 ok = edje_object_mmap_set(sd->img, f, key);
                              }
                            else
                              ok = !_efl_ui_image_smart_internal_file_set(sd->self, sd);
                         }
                    }
                  if (ok)
                    {
                       evas_object_smart_callback_call(sd->self, SIG_LOAD_OPEN, NULL);
                       if (sd->autoplay) efl_player_playing_set(sd->self, EINA_TRUE);
                    }
                  else evas_object_smart_callback_call(sd->self, SIG_LOAD_ERROR, NULL);
               }
          }
     }
   // close f, map and free strings
   _async_open_data_free(todo);
}

static Eina_Error
_efl_ui_image_async_file_set(Eo *obj, Efl_Ui_Image_Data *sd)
{
   Async_Open_Data *todo;
   const char *file = efl_file_get(obj);
   const char *key = efl_file_key_get(obj);
   const Eina_File *f = efl_file_mmap_get(obj);

   if (sd->async.th &&
       ((file == sd->async.file) ||
        (file && sd->async.file && !strcmp(file, sd->async.file))) &&
       ((key == sd->async.key) ||
        (key && sd->async.key && !strcmp(key, sd->async.key))))
     return 0;

   todo = calloc(1, sizeof(Async_Open_Data));
   if (!todo) return EINA_FALSE;

   _async_cancel(sd);

   todo->obj = obj;
   todo->file = eina_stringshare_add(file);
   todo->key = eina_stringshare_add(key);
   todo->f_set = f ? eina_file_dup(f) : NULL;

   eina_stringshare_replace(&sd->async.file, file);
   eina_stringshare_replace(&sd->async.key, key);

   sd->async.todo = todo;
   sd->async.th = ecore_thread_run(_efl_ui_image_async_open_do,
                                   _efl_ui_image_async_open_done,
                                   _efl_ui_image_async_open_cancel, todo);
   if (sd->async.th) return 0;

   _async_open_data_free(todo);
   _async_clear(sd);
   DBG("Could not spawn an async thread!");
   return EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;
}

static Eina_Error
_efl_ui_image_edje_file_set(Evas_Object *obj)
{
   Eina_Error err;
   const Eina_File *f;
   const char *key;

   EFL_UI_IMAGE_DATA_GET(obj, sd);

   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;

   f = efl_file_mmap_get(obj);
   key = efl_file_key_get(obj);
   _prev_img_del(sd);

   if (!sd->edje)
     {
        evas_object_del(sd->img);

        /* Edje object instead */
        sd->img = edje_object_add(evas_object_evas_get(obj));
        _recover_status(obj, sd);
        sd->edje = EINA_TRUE;
        evas_object_smart_member_add(sd->img, obj);
     }

   _async_cancel(sd);

   if (!sd->async_enable)
     {
        efl_file_key_set(sd->img, key);
        err = efl_file_mmap_set(sd->img, f);
        if (!err) err = efl_file_load(sd->img);
        if (err)
          {
             ERR("failed to set edje file '%s', group '%s': %s", eina_file_filename_get(f), key,
                 edje_load_error_str(edje_object_load_error_get(sd->img)));
             return err;
          }
     }
   else
     return _efl_ui_image_async_file_set(obj, sd);
   if (sd->autoplay) efl_player_playing_set(sd->self, EINA_TRUE);

   /* FIXME: do i want to update icon on file change ? */
   efl_canvas_group_change(obj);

   return 0;
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_smooth_scale_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Bool smooth)
{
   sd->smooth = smooth;
   if (sd->img && (!sd->edje)) evas_object_image_smooth_scale_set(sd->img, smooth);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_smooth_scale_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->smooth;
}

static Eina_Bool
_efl_ui_image_drag_n_drop_cb(void *elm_obj,
      Evas_Object *obj,
      Elm_Selection_Data *drop)
{
   Eina_Bool ret = EINA_FALSE;
   ret = efl_file_simple_load(obj, drop->data, NULL);
   if (ret)
     {
        DBG("dnd: %s, %s, %s", elm_widget_type_get(elm_obj),
              SIG_DND, (char *)drop->data);

        efl_event_callback_legacy_call(elm_obj, EFL_UI_IMAGE_EVENT_DROP, drop->data);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_image_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Image_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   if (elm_widget_is_legacy(obj))
     {
        evas_object_event_callback_add
          (priv->hit_rect, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, obj);
        priv->playback_loop = EINA_TRUE;
     }
   else
     {
        efl_ui_action_connector_bind_clickable_to_object(priv->hit_rect, obj);
     }

   priv->smooth = EINA_TRUE;
   priv->fill_inside = EINA_TRUE;
   priv->aspect_fixed = EINA_TRUE;
   priv->load_size = EINA_SIZE2D(0, 0);
   priv->scale = 1.0;
   priv->scale_up = EINA_TRUE;
   priv->scale_down = EINA_TRUE;
   priv->align_x = 0.5;
   priv->align_y = 0.5;

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

EOLIAN static void
_efl_ui_image_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Image_Data *sd)
{
   Efl_Model *model;

   if (elm_widget_is_legacy(obj))
     efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                            _on_size_hints_changed, sd);
   ecore_timer_del(sd->anim_timer);
   evas_object_del(sd->img);
   _prev_img_del(sd);
   _async_cancel(sd);
   if (sd->remote.copier) _efl_ui_image_remote_copier_cancel(obj, sd);
   if (sd->remote.binbuf) ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);
   ELM_SAFE_FREE(sd->remote.key, eina_stringshare_del);

   model = efl_ui_view_model_get(obj);
   if (model)
     {
         efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                                _efl_ui_image_model_properties_changed_cb, obj);
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

/* this function exists solely to call efl_canvas_group_change */
EOLIAN static void
_efl_ui_image_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_canvas_group_change(obj);
}

/* this function exists solely to call efl_canvas_group_change */
EOLIAN static void
_efl_ui_image_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_canvas_group_change(obj);
}

static void
_efl_ui_image_show(Eo *obj, Efl_Ui_Image_Data *sd)
{
   sd->show = EINA_TRUE;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_TRUE);

   if (sd->preload_status == EFL_UI_IMAGE_PRELOADING) return;
   if (sd->img) efl_gfx_entity_visible_set(sd->img, EINA_TRUE);
   _prev_img_del(sd);
}

static void
_efl_ui_image_hide(Eo *obj, Efl_Ui_Image_Data *sd)
{
   sd->show = EINA_FALSE;
   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_FALSE);
   if (sd->img) efl_gfx_entity_visible_set(sd->img, EINA_FALSE);
   _prev_img_del(sd);
}

EOLIAN static void
_efl_ui_image_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   if (vis) _efl_ui_image_show(obj, sd);
   else _efl_ui_image_hide(obj, sd);
}

EOLIAN static void
_efl_ui_image_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_efl_ui_image_efl_gfx_color_color_set(Eo *obj, Efl_Ui_Image_Data *sd, int r, int g, int b, int a)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_COLOR_SET, 0, r, g, b, a))
     return;

   efl_gfx_color_set(efl_super(obj, MY_CLASS), r, g, b, a);

   evas_object_color_set(sd->hit_rect, 0, 0, 0, 0);
   if (sd->img) evas_object_color_set(sd->img, r, g, b, a);
   if (sd->prev_img) evas_object_color_set(sd->prev_img, r, g, b, a);
}

EOLIAN static void
_efl_ui_image_efl_canvas_object_clipper_set(Eo *obj, Efl_Ui_Image_Data *sd, Evas_Object *clip)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_CLIP_SET, 0, clip))
     return;

   efl_canvas_object_clipper_set(efl_super(obj, MY_CLASS), clip);

   if (sd->img) evas_object_clip_set(sd->img, clip);
   if (sd->prev_img) evas_object_clip_set(sd->prev_img, clip);
}

EOLIAN static Eina_Error
_efl_ui_image_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   if (sd->stdicon)
     _elm_theme_object_icon_set(obj, sd->stdicon, elm_widget_style_get(obj));

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   efl_canvas_group_change(obj);

   return int_ret;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   if (elm_widget_is_legacy(obj))
     evas_object_smart_callback_call(obj, "clicked", NULL);
   else
     efl_event_callback_call(obj, EFL_INPUT_EVENT_CLICKED, NULL);
   return EINA_TRUE;
}

static void
_efl_ui_image_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Image_Data *sd)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Eina_Size2D sz;
   double ts;

   sd->in_calc = EINA_TRUE;
   // TODO: remove this function after using the widget's scale value instead of image's scale value,
   if (sd->no_scale)
     sd->scale = 1.0;
   else
     sd->scale = efl_gfx_entity_scale_get(obj) * elm_config_scale_get();

   ts = sd->scale;
   sd->scale = 1.0;
   sz = efl_gfx_view_size_get(obj);

   sd->scale = ts;
   evas_object_size_hint_combined_min_get(obj, &minw, &minh);

   if (sd->no_scale)
     {
        maxw = minw = sz.w;
        maxh = minh = sz.h;
        if ((sd->scale > 1.0) && (sd->scale_up))
          {
             maxw = minw = sz.w * sd->scale;
             maxh = minh = sz.h * sd->scale;
          }
        else if ((sd->scale < 1.0) && (sd->scale_down))
          {
             maxw = minw = sz.w * sd->scale;
             maxh = minh = sz.h * sd->scale;
          }
     }
   else
     {
        if (!sd->scale_down)
          {
             minw = sz.w * sd->scale;
             minh = sz.h * sd->scale;
          }
        if (!sd->scale_up)
          {
             maxw = sz.w * sd->scale;
             maxh = sz.h * sd->scale;
          }
     }

   efl_gfx_hint_size_restricted_min_set(obj, EINA_SIZE2D(minw, minh));
   efl_gfx_hint_size_restricted_max_set(obj, EINA_SIZE2D(maxw, maxh));

   if (sd->img)
     {
        //Retained way. Nothing does, if either way hasn't been changed.
        if (!sd->edje)
          efl_gfx_image_orientation_set(sd->img, sd->orient);

        _image_sizing_eval(obj, sd, sd->img);
        if (sd->prev_img) _image_sizing_eval(obj, sd, sd->prev_img);
     }
   sd->in_calc = EINA_FALSE;
}

static void
_efl_ui_image_load_size_set_internal(Evas_Object *obj, Efl_Ui_Image_Data *sd)
{
   Eina_Size2D sz = sd->load_size;

   if ((sz.w <= 0) || (sz.h <= 0))
     sz = efl_gfx_view_size_get(obj);
   evas_object_image_load_size_set(sd->img, sz.w, sz.h);
}

static void
_efl_ui_image_file_set_do(Evas_Object *obj)
{
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   ELM_SAFE_FREE(sd->prev_img, evas_object_del);

   sd->prev_img = sd->img;
   sd->img = _img_new(obj);
   _recover_status(obj, sd);

   sd->edje = EINA_FALSE;
   evas_object_image_smooth_scale_set(sd->img, sd->smooth);
   evas_object_image_load_orientation_set(sd->img, EINA_TRUE);
   _efl_ui_image_load_size_set_internal(obj, sd);
}

static void
_on_size_hints_changed(void *data, const Efl_Event *ev)
{
   Efl_Ui_Image_Data *pd = data;
   if (!pd->in_calc)
     efl_canvas_group_change(ev->object);
}

static void
_efl_ui_image_model_changed(void *data, const Efl_Event *event)
{
   Efl_Model_Changed_Event *ev = event->info;

   if (ev->previous)
     efl_event_callback_del(ev->previous, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                            _efl_ui_image_model_properties_changed_cb, event->object);
   if (ev->current)
     efl_event_callback_add(ev->current, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                            _efl_ui_image_model_properties_changed_cb, event->object);

   _update_viewmodel(event->object, data);
}

EOLIAN static Eo *
_efl_ui_image_efl_object_constructor(Eo *obj, Efl_Ui_Image_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_IMAGE);

   pd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_FIT;
   pd->self = obj;
   /* legacy elm_image starts paused */
   pd->paused = elm_widget_is_legacy(obj);
   pd->playback_speed = 1;

   return obj;
}

EOLIAN static void
_efl_ui_image_efl_object_invalidate(Eo *obj, Efl_Ui_Image_Data *pd EINA_UNUSED)
{
   Efl_Model *model;

   if (pd->property_watch)
     efl_event_callback_del(obj, EFL_UI_VIEW_EVENT_MODEL_CHANGED,
                            _efl_ui_image_model_changed, pd);

   model = efl_ui_view_model_get(obj);
   if (model)
     efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                            _efl_ui_image_model_properties_changed_cb, obj);
   efl_invalidate(efl_super(obj, EFL_UI_IMAGE_CLASS));
}

static const Eina_Slice remote_uri[] = {
  EINA_SLICE_STR_LITERAL("http://"),
  EINA_SLICE_STR_LITERAL("https://"),
  EINA_SLICE_STR_LITERAL("ftp://"),
  { }
};

static inline Eina_Bool
_efl_ui_image_is_remote(const char *file)
{
   Eina_Slice s = EINA_SLICE_STR(file);
   const Eina_Slice *itr;

   for (itr = remote_uri; itr->mem; itr++)
     if (eina_slice_startswith(s, *itr))
       return EINA_TRUE;

   return EINA_FALSE;
}

EOLIAN Eina_Error
_efl_ui_image_efl_file_load(Eo *obj, Efl_Ui_Image_Data *sd)
{
   Eina_Error ret;
   const char *file = efl_file_get(obj);

   if (efl_file_loaded_get(obj)) return 0;
   _async_cancel(sd);

   /* stop preloading as it may hit to-be-freed memory */
   if ((sd->img) && (sd->preload_status == EFL_UI_IMAGE_PRELOADING) && (!sd->edje))
     evas_object_image_preload(sd->img, EINA_TRUE);

   if (sd->remote.copier) _efl_ui_image_remote_copier_cancel(obj, sd);
   if (sd->remote.binbuf) ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);

   if (sd->anim)
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
        sd->paused = elm_widget_is_legacy(obj);
        sd->anim = EINA_FALSE;
        sd->frame_count = -1;
        sd->cur_frame = -1;
        sd->frame_duration = -1;
     }

   if (file && _efl_ui_image_is_remote(file))
     {
        evas_object_hide(sd->img);
        if (_efl_ui_image_download(obj, sd, file))
          {
             evas_object_smart_callback_call(obj, SIG_DOWNLOAD_START, NULL);
             return 0;
          }
     }

   if (!sd->async_enable)
     ret = _efl_ui_image_smart_internal_file_set(obj, sd);
   else
     ret = _efl_ui_image_async_file_set(obj, sd);

   return ret;
}

EOLIAN void
_efl_ui_image_efl_file_unload(Eo *obj, Efl_Ui_Image_Data *sd)
{
   _async_cancel(sd);

   /* stop preloading as it may hit to-be-freed memory */
   if (sd->img && sd->preload_status == EFL_UI_IMAGE_PRELOADING)
     evas_object_image_preload(sd->img, EINA_TRUE);

   if (sd->remote.copier) _efl_ui_image_remote_copier_cancel(obj, sd);
   if (sd->remote.binbuf) ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);

   if (sd->anim)
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
        sd->paused = elm_widget_is_legacy(obj);
        sd->anim = EINA_FALSE;
        sd->frame_count = -1;
        sd->cur_frame = -1;
        sd->frame_duration = -1;
     }

   if (sd->prev_img)
     _prev_img_del(sd);
   _efl_ui_image_file_set_do(obj);
   efl_file_unload(sd->img);
   efl_file_unload(efl_super(obj, MY_CLASS));

   efl_canvas_group_change(obj);
}

static Eina_Error
_efl_ui_image_smart_internal_file_set(Eo *obj, Efl_Ui_Image_Data *sd)
{
   Eina_Error err;
   const Eina_File *f;
   const char *key;
   const char *file = efl_file_get(obj);

   if (eina_str_has_extension(file, ".edj"))
     return _efl_ui_image_edje_file_set(obj);

   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;

   f = efl_file_mmap_get(obj);
   key = efl_file_key_get(obj);

   _efl_ui_image_file_set_do(obj);

   evas_object_image_mmap_set(sd->img, f, key);

   err = evas_object_image_load_error_get(sd->img);
   if (err)
     {
        if (file || f)
          {
             if (key)
               ERR("Failed to load image '%s' '%s': %s. (%p)",
                   eina_file_filename_get(f), key,
                   evas_load_error_str(err), obj);
             else
                ERR("Failed to load image '%s': %s. (%p)",
                    eina_file_filename_get(f),
                    evas_load_error_str(err), obj);
          }
        else
          {
             ERR("NULL image file passed! (%p)", obj);
          }
        _prev_img_del(sd);
        return err;
     }

   if (sd->preload_status == EFL_UI_IMAGE_PRELOAD_DISABLED)
     {
        _prev_img_del(sd);
        if (sd->autoplay) efl_player_playing_set(sd->self, EINA_TRUE);
     }
   else
     {
        evas_object_hide(sd->img);
        sd->preload_status = EFL_UI_IMAGE_PRELOADING;
        evas_object_image_preload(sd->img, EINA_FALSE);
     }

   efl_canvas_group_change(obj);

   return 0;
}

static void
_efl_ui_image_remote_copier_del(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *dialer = efl_io_copier_source_get(event->object);
   efl_del(dialer);
}

static void
_efl_ui_image_remote_copier_cancel(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   Eo *copier = sd->remote.copier;

   if (!copier) return;
   /* copier is flagged as close_on_invalidate, thus:
    * efl_del()
    *  -> efl_io_closer_close()
    *      -> "done" event
    *          -> _efl_ui_image_remote_copier_done()
    *
    * flag sd->remote.copier = NULL so _efl_ui_image_remote_copier_done()
    * knows about it.
    */
   sd->remote.copier = NULL;
   efl_del(copier);
}

static void
_efl_ui_image_remote_copier_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_File *f;
   Eo *dialer;
   const char *url;
   Eina_Error ret;

   /* we're called from _efl_ui_image_remote_copier_cancel() */
   if (!sd->remote.copier) return;

   /* stop preloading as it may hit to-be-freed memory */
   if (sd->img && sd->preload_status == EFL_UI_IMAGE_PRELOADING)
     evas_object_image_preload(sd->img, EINA_TRUE);

   if (sd->remote.binbuf) eina_binbuf_free(sd->remote.binbuf);
   sd->remote.binbuf = efl_io_copier_binbuf_steal(sd->remote.copier);

   dialer = efl_io_copier_source_get(sd->remote.copier);
   url = efl_net_dialer_address_dial_get(dialer);
   f = eina_file_virtualize(url,
                            eina_binbuf_string_get(sd->remote.binbuf),
                            eina_binbuf_length_get(sd->remote.binbuf),
                            EINA_FALSE);
   efl_file_mmap_set(obj, f);
   ret = _efl_ui_image_smart_internal_file_set(obj, sd);
   eina_file_close(f);

   if (ret)
     {
        Efl_Ui_Image_Error err = { 0, EINA_TRUE };

        ELM_SAFE_FREE(sd->remote.binbuf, eina_binbuf_free);
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);
     }
   else
     {
        if (sd->preload_status != EFL_UI_IMAGE_PRELOAD_DISABLED)
          {
             sd->preload_status = EFL_UI_IMAGE_PRELOADING;
             evas_object_image_preload(sd->img, EINA_FALSE);
          }
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_DONE, NULL);
     }

   ELM_SAFE_FREE(sd->remote.key, eina_stringshare_del);
   ELM_SAFE_FREE(sd->remote.copier, efl_del);
}

static void
_efl_ui_image_remote_copier_error(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Eina_Error *perr = event->info;
   Efl_Ui_Image_Error err = { *perr, EINA_FALSE };

   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &err);

   _efl_ui_image_remote_copier_cancel(obj, sd);
   ELM_SAFE_FREE(sd->remote.key, eina_stringshare_del);
}

static void
_efl_ui_image_remote_copier_progress(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Image_Progress progress;
   uint64_t now, total;

   efl_io_copier_progress_get(event->object, &now, NULL, &total);

   progress.now = now;
   progress.total = total;
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_PROGRESS, &progress);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_ui_image_remote_copier_cbs,
                           { EFL_EVENT_DEL, _efl_ui_image_remote_copier_del },
                           { EFL_IO_COPIER_EVENT_DONE, _efl_ui_image_remote_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _efl_ui_image_remote_copier_error },
                           { EFL_IO_COPIER_EVENT_PROGRESS, _efl_ui_image_remote_copier_progress });

static Eina_Bool
_efl_ui_image_download(Eo *obj, Efl_Ui_Image_Data *sd, const char *url)
{
   Eo *dialer;
   Efl_Ui_Image_Error img_err = { ENOSYS, EINA_FALSE };
   Eina_Error err;
   const char *key = efl_file_key_get(obj);

   dialer = efl_add(EFL_NET_DIALER_HTTP_CLASS, obj,
                    efl_net_dialer_http_allow_redirects_set(efl_added, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(dialer, error_dialer);

   sd->remote.copier = efl_add(EFL_IO_COPIER_CLASS, obj,
                               efl_io_copier_source_set(efl_added, dialer),
                               efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                               efl_event_callback_array_add(efl_added, _efl_ui_image_remote_copier_cbs(), obj));
   EINA_SAFETY_ON_NULL_GOTO(sd->remote.copier, error_copier);
   eina_stringshare_replace(&sd->remote.key, key);

   err = efl_net_dialer_dial(dialer, url);
   if (err)
     {
        img_err.status = err;
        ERR("Could not download %s: %s", url, eina_error_msg_get(err));
        evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
        goto error_dial;
     }
   return EINA_TRUE;

 error_dial:
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
   _efl_ui_image_remote_copier_cancel(obj, sd);
   return EINA_FALSE;

 error_copier:
   efl_del(dialer);
 error_dialer:
   evas_object_smart_callback_call(obj, SIG_DOWNLOAD_ERROR, &img_err);
   return EINA_FALSE;
}

EOLIAN static const char*
_efl_ui_image_efl_layout_group_group_data_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, const char *key)
{
  if (sd->edje)
    return edje_object_data_get(sd->img, key);
  return NULL;
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_layout_group_part_exist_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, const char *part)
{
   if (sd->edje)
     return edje_object_part_exists(sd->img, part);
   return EINA_FALSE;
}


EOLIAN static void
_efl_ui_image_efl_layout_signal_signal_emit(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, const char *emission, const char *source)
{
   if (sd->edje)
     edje_object_signal_emit(sd->img, emission, source);
}

EOLIAN static void
_efl_ui_image_efl_layout_signal_message_send(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, int id, const Eina_Value msg)
{
   if (pd->edje)
     efl_layout_signal_message_send(pd->img, id, msg);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, const char *emission, const char *source, void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb)
{
   if (pd->edje)
     return efl_layout_signal_callback_add(pd->img, emission, source, func_data, func, func_free_cb);
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, const char *emission, const char *source, void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb)
{
   if (pd->edje)
     return efl_layout_signal_callback_del(pd->img, emission, source, func_data, func, func_free_cb);
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_image_efl_layout_signal_signal_process(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Bool recurse)
{
   if (pd->edje)
     efl_layout_signal_process(pd->img, recurse);
}


EOLIAN static Eina_Size2D
_efl_ui_image_efl_layout_group_group_size_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return efl_layout_group_size_min_get(sd->img);
   else
     return EINA_SIZE2D(0, 0);
}

EOLIAN static Eina_Size2D
_efl_ui_image_efl_layout_group_group_size_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return efl_layout_group_size_max_get(sd->img);
   else
     return EINA_SIZE2D(0, 0);
}

EOLIAN static void
_efl_ui_image_efl_layout_calc_calc_force(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     edje_object_calc_force(sd->img);
}

EOLIAN static Eina_Size2D
_efl_ui_image_efl_layout_calc_calc_size_min(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Size2D restricted)
{
   if (sd->edje)
     return efl_layout_calc_size_min(sd->img, restricted);
   else
     {
        // Ignore restricted here? Combine with min? Hmm...
        return efl_gfx_hint_size_combined_min_get(sd->img);
     }
}

EOLIAN Eina_Rect
_efl_ui_image_efl_layout_calc_calc_parts_extends(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return efl_layout_calc_parts_extends(sd->img);
   return efl_gfx_entity_geometry_get(sd->img);
}

EOLIAN static int
_efl_ui_image_efl_layout_calc_calc_freeze(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje) return edje_object_freeze(sd->img);
   return 0;
}

EOLIAN static int
_efl_ui_image_efl_layout_calc_calc_thaw(Eo *obj, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     {
        int ret = edje_object_thaw(sd->img);
        efl_canvas_group_calculate(obj);
        return ret;
     }
   return 0;
}

EOLIAN void
_efl_ui_image_efl_layout_calc_calc_auto_update_hints_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Bool update)
{
   if (sd->edje)
     efl_layout_calc_auto_update_hints_set(sd->img, update);
}

EOLIAN Eina_Bool
_efl_ui_image_efl_layout_calc_calc_auto_update_hints_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje) return efl_layout_calc_auto_update_hints_get(sd->img);
   return EINA_TRUE;
}

#if 0
// Kept for reference: wait for async open to complete - probably unused.
static Eina_Bool
_efl_ui_image_efl_file_async_wait(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   Eina_Bool ok = EINA_TRUE;
   if (!pd->async.th) return ok;
   if (!ecore_thread_wait(pd->async.th, 1.0))
     {
        ERR("Failed to wait on async file open!");
        ok = EINA_FALSE;
     }
   return ok;
}
#endif

/* Legacy style async API. While legacy only, this is new from 1.19.
 * Tizen has used elm_image_async_open_set() internally for a while, despite
 * EFL upstream not exposing a proper async API. */

EAPI void
elm_image_async_open_set(Eo *obj, Eina_Bool async)
{
   Efl_Ui_Image_Data *pd;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, MY_CLASS));
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (pd->async_enable == async) return;
   pd->async_enable = async;
   if (!async) _async_cancel(pd);
}

EOLIAN static Eina_Size2D
_efl_ui_image_efl_gfx_view_view_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   int tw, th;

   if (!sd->img)
     {
        tw = 0; th = 0;
     }
   else if (efl_isa(sd->img, EFL_CANVAS_LAYOUT_CLASS))
     edje_object_size_min_get(sd->img, &tw, &th);
   else
     evas_object_image_size_get(sd->img, &tw, &th);

   return EINA_SIZE2D(tw, th);
}

EOLIAN static Eina_Size2D
_efl_ui_image_efl_gfx_image_image_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (!sd->img || sd->edje)
     return EINA_SIZE2D(0, 0);

   return efl_gfx_image_size_get(sd->img);
}

EOLIAN static double
_efl_ui_image_efl_gfx_image_ratio_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return 1.0; //documented value for the case that ratio cannot be calculated
   return efl_gfx_image_ratio_get(pd->img);
}

EOLIAN static Eina_Rect
_efl_ui_image_efl_gfx_image_content_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje)
     {
        Eina_Size2D size = efl_gfx_entity_size_get(pd->img);
        return EINA_RECT(0, 0, size.w, size.h);
     }

   return efl_gfx_image_content_region_get(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_border_insets_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, int l, int r, int t, int b)
{
   if (pd->edje) return;
   efl_gfx_image_border_insets_set(pd->img, l, r, t, b);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_border_insets_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, int *l, int *r, int *t, int *b)
{
   if (pd->edje)
     {
        if (l) *l = 0;
        if (r) *r = 0;
        if (t) *t = 0;
        if (b) *b = 0;
     }
   else
     {
        efl_gfx_image_border_insets_get(pd->img, l, r, t, b);
     }
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_border_insets_scale_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, double scale)
{
   if (pd->edje) return;
   efl_gfx_image_border_insets_scale_set(pd->img, scale);
}

EOLIAN static double
_efl_ui_image_efl_gfx_image_border_insets_scale_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return 0.0;
   return efl_gfx_image_border_insets_scale_get(pd->img);
}


EOLIAN static void
_efl_ui_image_efl_gfx_image_center_fill_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Efl_Gfx_Center_Fill_Mode fill)
{
   if (pd->edje) return;
   efl_gfx_image_center_fill_mode_set(pd->img, fill);
}


EOLIAN static Efl_Gfx_Center_Fill_Mode
_efl_ui_image_efl_gfx_image_center_fill_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return EFL_GFX_CENTER_FILL_MODE_DEFAULT;
   return efl_gfx_image_center_fill_mode_get(pd->img);
}

EOLIAN static Eina_Error
_efl_ui_image_efl_gfx_image_stretch_region_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Iterator *horizontal, Eina_Iterator *vertical)
{
   if (pd->edje)
     {
        eina_iterator_free(horizontal);
        eina_iterator_free(vertical);
        return EINA_ERROR_NO_ERROR;
     }
   else
     {
        return efl_gfx_image_stretch_region_set(pd->img, horizontal, vertical);
     }
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_stretch_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Iterator **horizontal, Eina_Iterator **vertical)
{
   if (pd->edje)
     {
        if (horizontal) *horizontal = NULL;
        if (vertical) *vertical = NULL;
     }
   else
     {
        efl_gfx_image_stretch_region_get(pd->img, horizontal, vertical);
     }
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_scale_hint_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Efl_Gfx_Image_Scale_Hint hint)
{
   if (pd->edje) return;
   return efl_gfx_image_scale_hint_set(pd->img, hint);
}

EOLIAN static Efl_Gfx_Image_Scale_Hint
_efl_ui_image_efl_gfx_image_scale_hint_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
  if (pd->edje) return EFL_GFX_IMAGE_SCALE_HINT_NONE;
  return efl_gfx_image_scale_hint_get(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_content_hint_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Efl_Gfx_Image_Content_Hint hint)
{
   if (pd->edje) return;
   return efl_gfx_image_content_hint_set(pd->img, hint);
}

EOLIAN static Efl_Gfx_Image_Content_Hint
_efl_ui_image_efl_gfx_image_content_hint_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
  if (pd->edje) return EFL_GFX_IMAGE_CONTENT_HINT_NONE;
  return efl_gfx_image_content_hint_get(pd->img);
}

EOLIAN static Eina_Error
_efl_ui_image_efl_gfx_image_image_load_error_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return EINA_ERROR_NO_ERROR;

   return efl_gfx_image_load_error_get(pd->img);
}

EAPI void
elm_image_prescale_set(Evas_Object *obj,
                       int size)
{
   EFL_UI_IMAGE_CHECK(obj);
   efl_gfx_image_load_controller_load_size_set(obj, EINA_SIZE2D(size, size));
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_size_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Size2D sz)
{
   sd->load_size = sz;

   if (!sd->img) return;
   _efl_ui_image_load_size_set_internal(obj, sd);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_async_start(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return;
   efl_gfx_image_load_controller_load_async_start(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_async_cancel(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return;
   efl_gfx_image_load_controller_load_async_cancel(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_dpi_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, double dpi)
{
   if (pd->edje) return;
   efl_gfx_image_load_controller_load_dpi_set(pd->img, dpi);
}


EOLIAN static double
_efl_ui_image_efl_gfx_image_load_controller_load_dpi_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return 0.0;
   return efl_gfx_image_load_controller_load_dpi_get(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_orientation_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Bool enable)
{
   if (pd->edje) return;
   efl_gfx_image_load_controller_load_orientation_set(pd->img, enable);
}


EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_load_controller_load_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return EINA_FALSE;
   return efl_gfx_image_load_controller_load_orientation_get(pd->img);
}


EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_scale_down_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, int div)
{
   if (pd->edje) return;
   efl_gfx_image_load_controller_load_scale_down_set(pd->img, div);
}

EOLIAN static int
_efl_ui_image_efl_gfx_image_load_controller_load_scale_down_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return EINA_FALSE;
   return efl_gfx_image_load_controller_load_scale_down_get(pd->img);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_skip_header_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Bool skip)
{
   if (pd->edje) return;
   return efl_gfx_image_load_controller_load_skip_header_set(pd->img, skip);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_load_controller_load_skip_header_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
   if (pd->edje) return EINA_FALSE;
   return efl_gfx_image_load_controller_load_skip_header_get(pd->img);
}


EOLIAN static void
_efl_ui_image_efl_gfx_image_load_controller_load_region_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd, Eina_Rect region)
{
   if (pd->edje) return;
   return efl_gfx_image_load_controller_load_region_set(pd->img, region);
}


EOLIAN static Eina_Rect
_efl_ui_image_efl_gfx_image_load_controller_load_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{

   if (pd->edje) return EINA_RECT(0, 0, 0, 0);
   return efl_gfx_image_load_controller_load_region_get(pd->img);
}


EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_load_controller_load_region_support_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd)
{
  if (pd->edje) return EINA_FALSE;
  return EINA_TRUE;
}

EAPI int
elm_image_prescale_get(const Evas_Object *obj)
{
   Eina_Size2D sz;
   EFL_UI_IMAGE_CHECK(obj) 0;

   sz = efl_gfx_image_load_controller_load_size_get(obj);

   return MAX(sz.w, sz.h);
}

EOLIAN static Eina_Size2D
_efl_ui_image_efl_gfx_image_load_controller_load_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->load_size;
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_orientable_image_orientation_set(Eo *obj, Efl_Ui_Image_Data *sd, Efl_Gfx_Image_Orientation orient)
{
   if (sd->edje) return;
   if (sd->orient == orient) return;

   sd->orient = orient;
   efl_canvas_group_change(obj);
}

EOLIAN static Efl_Gfx_Image_Orientation
_efl_ui_image_efl_gfx_image_orientable_image_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->orient;
}

/**
 * Turns on editing through drag and drop and copy and paste.
 */
EOLIAN static void
_efl_ui_image_efl_ui_draggable_drag_target_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool edit)
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
_efl_ui_image_efl_ui_draggable_drag_target_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->edit;
}

EAPI Eina_Bool
elm_image_animated_available_get(const Evas_Object *obj)
{
   return efl_playable_get(obj);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_playable_playable_get(const Eo *obj, Efl_Ui_Image_Data *sd)
{
   if (sd->edje) return EINA_TRUE;

   return evas_object_image_animated_get(elm_image_object_get(obj));
}

static Eina_Bool
_efl_ui_image_animated_set_internal(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool anim)
{
   anim = !!anim;
   if (sd->anim == anim) return EINA_TRUE;

   if (sd->edje)
     {
        edje_object_animation_set(sd->img, anim);
        sd->anim = anim;
        efl_player_playback_loop_set(sd->img, sd->playback_loop);
        return EINA_TRUE;
     }
   sd->img = elm_image_object_get(obj);

   if (elm_widget_is_legacy(obj)) sd->anim = anim; //legacy

   if (!evas_object_image_animated_get(sd->img)) return EINA_FALSE;
   sd->anim = anim;

   if (anim)
     {
        sd->frame_count = evas_object_image_animated_frame_count_get(sd->img);
        sd->cur_frame = 1;
        sd->frame_duration =
          evas_object_image_animated_frame_duration_get
            (sd->img, sd->cur_frame, 0);
        evas_object_image_animated_frame_set(sd->img, sd->cur_frame);
        if (!sd->paused)//legacy
          sd->anim_timer = ecore_timer_add
              (sd->frame_duration / sd->playback_speed, _efl_ui_image_animate_cb, obj);
     }
   else
     {
        sd->frame_count = -1;
        sd->cur_frame = -1;
        sd->frame_duration = -1;
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
     }
   return EINA_TRUE;
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
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _efl_ui_image_animated_set_internal(obj, sd, anim);
}

EAPI Eina_Bool
elm_image_animated_get(const Evas_Object *obj)
{
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return _efl_ui_image_animated_get_internal(obj, sd);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_playing_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool playing)
{
   return _efl_ui_image_animated_set_internal(obj, sd, playing);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_playing_get(const Eo *obj, Efl_Ui_Image_Data *sd)
{
   return _efl_ui_image_animated_get_internal(obj, sd);
}

EOLIAN static void
_efl_ui_image_efl_player_playback_loop_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, Eina_Bool playback_loop)
{
   playback_loop = !!playback_loop;
   sd->playback_loop = playback_loop;
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_playback_loop_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->playback_loop;
}

EOLIAN static void
_efl_ui_image_efl_player_autoplay_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool autoplay)
{
   autoplay = !!autoplay;
   if (sd->autoplay == autoplay) return;
   sd->autoplay = autoplay;
   if (sd->img && (!sd->edje))
     {
        /* filter cases where we aren't going to immediately start playing */
        if (!autoplay) return;
        if ((sd->preload_status != EFL_UI_IMAGE_PRELOADED) &&
            (sd->preload_status != EFL_UI_IMAGE_PRELOAD_DISABLED))
          return;
     }
   efl_player_playing_set(obj, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_autoplay_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->autoplay;
}

EOLIAN static void
_efl_ui_image_efl_player_playback_speed_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, double factor)
{
   EINA_SAFETY_ON_TRUE_RETURN(factor < 0.0);
   EINA_SAFETY_ON_TRUE_RETURN(EINA_DBL_EQ(factor, 0.0));
   if (EINA_DBL_EQ(sd->playback_speed, factor)) return;
   sd->playback_speed = factor;
   if (sd->edje)
     efl_player_playback_speed_set(sd->img, factor);
   else if (sd->anim_timer)
     {
        ecore_timer_interval_set(sd->anim_timer, sd->frame_duration / sd->playback_speed);
        ecore_timer_reset(sd->anim_timer);
     }
}

EOLIAN static double
_efl_ui_image_efl_player_playback_speed_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->playback_speed;
}

EOLIAN static void
_efl_ui_image_efl_player_playback_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, double sec)
{
   EINA_SAFETY_ON_TRUE_RETURN(sec < 0.0);
   if (sd->edje)
     efl_player_playback_position_set(sd->img, sec);
   else if ((sd->frame_count > 0) && (sd->frame_duration > 0.0))
     {
        /* validate total animation time */
        EINA_SAFETY_ON_TRUE_RETURN(sd->frame_count * sd->frame_duration < sec);
        sd->cur_frame = lround(sec / sd->frame_duration);
     }
}

EOLIAN static double
_efl_ui_image_efl_player_playback_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     efl_player_playback_position_get(sd->img);
   else if ((sd->frame_count > 0) && (sd->frame_duration > 0.0))
     return sd->cur_frame * sd->frame_duration;
   return 0.0;
}

EOLIAN static double
_efl_ui_image_efl_player_playback_progress_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     efl_player_playback_progress_get(sd->img);
   else if (sd->frame_count > 1)
     return ((double)sd->cur_frame - 1.0) / ((double)sd->frame_count - 1.0);
   return 0.0;
}

EOLIAN static void
_efl_ui_image_efl_player_playback_progress_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, double progress)
{
   EINA_SAFETY_ON_TRUE_RETURN(progress > 1 || progress < 0);
   if (sd->edje)
     efl_player_playback_progress_set(sd->img, progress);
   else if (sd->frame_count > 0)
     sd->cur_frame = (lround(progress * (sd->frame_count - 1)) + 1);
   else
     sd->cur_frame = 1;
}

static Eina_Bool
_efl_ui_image_animated_paused_set_internal(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool paused)
{
   paused = !!paused;
   if (!sd->anim) return EINA_FALSE;
   if (sd->paused == paused) return EINA_TRUE;
   sd->paused = paused;
   if (sd->edje)
     {
        edje_object_play_set(sd->img, !paused);
        return EINA_TRUE;
     }
   if (!paused)
     {
        sd->anim_timer = ecore_timer_add
            (sd->frame_duration / sd->playback_speed, _efl_ui_image_animate_cb, obj);
     }
   else
     {
        ELM_SAFE_FREE(sd->anim_timer, ecore_timer_del);
     }
   return EINA_TRUE;
}

static Eina_Bool
_efl_ui_image_animated_paused_get_internal(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   if (sd->edje)
     return !edje_object_play_get(sd->img);
   return sd->paused;
}

EAPI void
elm_image_animated_play_set(Elm_Image *obj, Eina_Bool play)
{
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   if (!sd) return;
   _efl_ui_image_animated_paused_set_internal(obj, sd, !play);
}

EAPI Eina_Bool
elm_image_animated_play_get(const Elm_Image *obj)
{
   Efl_Ui_Image_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   if (!sd) return EINA_FALSE;
   return !_efl_ui_image_animated_paused_get_internal(obj, sd);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_paused_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool paused)
{
   return _efl_ui_image_animated_paused_set_internal(obj, sd, paused);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_player_paused_get(const Eo *obj, Efl_Ui_Image_Data *sd)
{
   return _efl_ui_image_animated_paused_get_internal(obj, sd);
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_scale_method_set(Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd EINA_UNUSED, Efl_Gfx_Image_Scale_Method scale_type)
{
   if (scale_type == sd->scale_type) return;

   sd->scale_type = scale_type;

   efl_canvas_group_change(obj);
}

EOLIAN static Efl_Gfx_Image_Scale_Method
_efl_ui_image_efl_gfx_image_scale_method_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->scale_type;
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_can_upscale_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool can_upscale)
{
   if (can_upscale == sd->scale_up) return;

   sd->scale_up = !!can_upscale;

   efl_canvas_group_change(obj);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_can_upscale_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->scale_up;
}

EOLIAN static void
_efl_ui_image_efl_gfx_image_can_downscale_set(Eo *obj, Efl_Ui_Image_Data *sd, Eina_Bool can_downscale)
{
   if (can_downscale == sd->scale_down) return;

   sd->scale_down = !!can_downscale;

   efl_canvas_group_change(obj);
}

EOLIAN static Eina_Bool
_efl_ui_image_efl_gfx_image_can_downscale_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->scale_down;
}

EOLIAN static void
_efl_ui_image_efl_gfx_arrangement_content_align_set(Eo *obj, Efl_Ui_Image_Data *sd, double align_x, double align_y)
{
   if (align_x > 1.0)
     align_x = 1.0;
   else if (align_x < 0.0)
     align_x = 0.0;

   if (align_y > 1.0)
     align_y = 1.0;
   else if (align_y < 0.0)
     align_y = 0.0;

   if ((EINA_DBL_EQ(align_x, sd->align_x)) && (EINA_DBL_EQ(align_y, sd->align_y))) return;

   sd->align_x = align_x;
   sd->align_y = align_y;

   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_image_efl_gfx_arrangement_content_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd, double *align_x, double *align_y)
{
   if (align_x) *align_x = sd->align_x;
   if (align_y) *align_y = sd->align_y;
}

// A11Y

EOLIAN static Eina_Rect
_efl_ui_image_efl_access_component_extents_get(const Eo *obj, Efl_Ui_Image_Data *sd EINA_UNUSED, Eina_Bool screen_coords)
{
   Eina_Rect r;
   Evas_Object *image = elm_image_object_get(obj);

   r.x = r.y = r.w = r.h = -1;
   if (!image) return r;

   evas_object_geometry_get(image, &r.x, &r.y, NULL, NULL);
   if (screen_coords)
     {
        r = _efl_access_component_screen_coords_extents_get(obj, r);
     }
   elm_image_object_size_get(obj, &r.w, &r.h);
   return r;
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_image_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
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
        efl_file_simple_load(obj, path, NULL);
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
        if (!ret)
          {
             ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
             if (ret && fdo) *fdo = EINA_TRUE;
          }
     }
   else
     {
        ret = _icon_freedesktop_set(obj, name, _icon_size_min_get(obj));
        if (ret && fdo) *fdo = EINA_TRUE;
     }

   if (ret)
     {
        eina_stringshare_replace(&sd->stdicon, name);
        efl_canvas_group_change(obj);
        return EINA_TRUE;
     }
   else
     eina_stringshare_replace(&sd->stdicon, NULL);

   if (!eina_file_path_relative(name))
     {
        if (fdo)
          *fdo = EINA_FALSE;
        return efl_file_simple_load(obj, name, NULL);
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
_efl_ui_image_icon_get(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Data *sd)
{
   return sd->stdicon;
}

void
_update_viewmodel(Eo *obj, Efl_Ui_Image_Data *pd)
{
   Eina_Value *vfile = NULL;
   Eina_Value *vkey = NULL;
   Eina_File *f = NULL;
   char *file = NULL;
   char *key = NULL;
   Efl_Model *model;

   if (!pd->property.file) return ;

   if (!pd->property_watch)
     {
        efl_event_callback_add(obj, EFL_UI_VIEW_EVENT_MODEL_CHANGED,
                               _efl_ui_image_model_changed, pd);
        pd->property_watch = EINA_TRUE;
     }

   model = efl_ui_view_model_get(obj);
   if (!model) return ;

   vfile = efl_model_property_get(model, pd->property.file);
   if (!vfile) return;
   vkey = efl_model_property_get(model, pd->property.key);

   if (eina_value_type_get(vfile) == EINA_VALUE_TYPE_ERROR)
     goto err;

   if (pd->property.icon)
     {
        file = eina_value_to_string(vfile);

        efl_ui_image_icon_set(obj, file);
     }
   else
     {
        if (vkey && eina_value_type_get(vkey) != EINA_VALUE_TYPE_ERROR)
          key = eina_value_to_string(vkey);
        if (eina_value_type_get(vfile) == EINA_VALUE_TYPE_FILE)
          {
             eina_value_get(vfile, &f);

             efl_file_simple_mmap_load(obj, f, key);
          }
        else
          {
             file = eina_value_to_string(vfile);

             efl_file_simple_load(obj, file, key);
          }
     }

   free(file);
   free(key);
err:
   eina_value_free(vfile);
   eina_value_free(vkey);
}

static void
_efl_ui_image_model_properties_changed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *evt = event->info;
   Eina_Array_Iterator it;
   Eo *obj = data;
   const char *prop;
   unsigned int i;
   Eina_Bool refresh = EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, pd);

   if (!evt->changed_properties)
     return;

   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
        if (pd->property.file &&
            (pd->property.file == prop || !strcmp(pd->property.file, prop)))
          {
             refresh = EINA_TRUE;
             break ;
          }
        if (pd->property.key &&
            (pd->property.key == prop || !strcmp(pd->property.key, prop)))
          {
             refresh = EINA_TRUE;
             break ;
          }
     }

   if (refresh) _update_viewmodel(obj, pd);
}

EOLIAN static Eina_Error
_efl_ui_image_efl_ui_property_bind_property_bind(Eo *obj, Efl_Ui_Image_Data *pd, const char *key, const char *property)
{
   Eina_Stringshare *sk;

   if (efl_ui_property_bind(efl_super(obj, EFL_UI_IMAGE_CLASS), key, property) == 0)
     return 0;

   if (strcmp(key, "filename") == 0)
     {
        pd->property.icon = EINA_FALSE;
        eina_stringshare_replace(&pd->property.file, property);
     }
   else if (strcmp(key, "icon") == 0)
     {
        pd->property.icon = EINA_TRUE;
        eina_stringshare_replace(&pd->property.file, property);
        eina_stringshare_replace(&pd->property.key, NULL);
     }
   else if (strcmp(key, "key") == 0)
     {
        eina_stringshare_replace(&pd->property.key, property);
     }
   else
     {
        return EFL_PROPERTY_ERROR_INVALID_KEY;
     }

   _update_viewmodel(obj, pd);

   sk = eina_stringshare_add(key);
   efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) sk);
   eina_stringshare_del(sk);

   return 0;
}

EAPI void
elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, MY_CLASS));
   efl_gfx_image_smooth_scale_set(obj, smooth);
   efl_canvas_group_change(obj);
}

EAPI Eina_Bool
elm_image_smooth_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(obj, MY_CLASS), EINA_FALSE);
   return efl_gfx_image_smooth_scale_get(obj);
}

// A11Y - END

/* Legacy deprecated functions */
EAPI void
elm_image_editable_set(Evas_Object *obj, Eina_Bool edit)
{
   efl_ui_draggable_drag_target_set(obj, edit);
}

EAPI Eina_Bool
elm_image_editable_get(const Evas_Object *obj)
{
   return efl_ui_draggable_drag_target_get(obj);
}

EAPI Eina_Bool
elm_image_file_set(Evas_Object *obj, const char *file, const char *group)
{
   Eina_Bool ret = EINA_FALSE;

   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;

   /* check if previous path is same with new one.
      and return true if they are same */
   const char *cur_file_path = efl_file_get(obj);
   if ((cur_file_path && file) && !strcmp(cur_file_path, file))
     {
        const char *cur_group = efl_file_key_get(obj);
        if (!(cur_group && group && strcmp(cur_group, group)))
          {
             if (efl_file_loaded_get(obj)) return EINA_TRUE;
             if (_efl_ui_image_is_remote(file)) return EINA_TRUE;
          }
     }

   ret = efl_file_simple_load(obj, file, group);
   efl_canvas_group_change(obj);
   return ret;
}

EAPI void
elm_image_file_get(const Eo *obj, const char **file, const char **group)
{
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   efl_file_simple_get(sd->img, file, group);
}

EAPI Eina_Bool
elm_image_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   return efl_file_simple_mmap_load(obj, file, group);
}

EAPI Eina_Bool
elm_image_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(img, EINA_FALSE);

   Evas_Load_Error err;

   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);

   if (efl_file_loaded_get(obj))
     efl_file_unload(obj);
   else
     _efl_ui_image_file_set_do(obj);

   evas_object_image_memfile_set
     (sd->img, (void *)img, size, (char *)format, (char *)key);

   if (sd->preload_status == EFL_UI_IMAGE_PRELOAD_DISABLED)
     {
        _prev_img_del(sd);
        if (sd->autoplay) efl_player_playing_set(sd->self, EINA_TRUE);
     }
   else
     {
        sd->preload_status = EFL_UI_IMAGE_PRELOADING;
        evas_object_image_preload(sd->img, EINA_FALSE);
     }

   err = evas_object_image_load_error_get(sd->img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        if (img)
          ERR("Failed to load image from memory block (" FMT_SIZE_T
              " bytes): %s (%p)", size, evas_load_error_str(err), sd->img);
        else
          ERR("NULL image data passed (%p)", sd->img);

        _prev_img_del(sd);
        return EINA_FALSE;
     }

   efl_canvas_group_change(obj);

   return EINA_TRUE;
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
        if (sd->fill_inside) sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_FIT;
        else sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_EXPAND;
     }
   else
     sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_FILL;

   efl_canvas_group_change(obj);
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

   if (sd->edje) return;
   if (!sd->img)
     {
        if (disable) sd->preload_status = EFL_UI_IMAGE_PRELOAD_DISABLED;
        else sd->preload_status = EFL_UI_IMAGE_PRELOAD_ENABLED;
        return;
     }

   if (disable)
     {
        if (sd->preload_status == EFL_UI_IMAGE_PRELOADING)
          {
             evas_object_image_preload(sd->img, disable);
             if (sd->show) evas_object_show(sd->img);
             _prev_img_del(sd);
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
elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient elm_orient)
{
   // This array takes an Elm_Image_Orient and turns it into an Efl_Gfx_Image_Orientation
   static const Efl_Gfx_Image_Orientation efl_orient[8] = {
      EFL_GFX_IMAGE_ORIENTATION_NONE,
      EFL_GFX_IMAGE_ORIENTATION_RIGHT,
      EFL_GFX_IMAGE_ORIENTATION_DOWN,
      EFL_GFX_IMAGE_ORIENTATION_LEFT,
      EFL_GFX_IMAGE_ORIENTATION_FLIP_HORIZONTAL,
      EFL_GFX_IMAGE_ORIENTATION_FLIP_VERTICAL,
      EFL_GFX_IMAGE_ORIENTATION_LEFT | EFL_GFX_IMAGE_ORIENTATION_FLIP_VERTICAL,
      EFL_GFX_IMAGE_ORIENTATION_RIGHT | EFL_GFX_IMAGE_ORIENTATION_FLIP_VERTICAL
   };

   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   EINA_SAFETY_ON_FALSE_RETURN(elm_orient >= 0 && elm_orient < 8);
   sd->image_orient = elm_orient;
   efl_gfx_image_orientation_set(obj, efl_orient[elm_orient]);

   // to keep behavior compatibility, update inlined image orientation
   if (sd->img) efl_gfx_image_orientation_set(sd->img,  efl_orient[elm_orient]);
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
   if (!sd->img)
     sd->img = _img_new((Evas_Object *)obj);

   return sd->img;
}

EAPI void
elm_image_object_size_get(const Evas_Object *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_view_size_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI void
elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   sd->no_scale = no_scale;

   efl_canvas_group_change(obj);
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

   efl_canvas_group_change(obj);
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
elm_image_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed)
{
   EFL_UI_IMAGE_CHECK(obj);
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   fixed = !!fixed;
   if (sd->aspect_fixed == fixed) return;

   sd->aspect_fixed = fixed;

   if (sd->aspect_fixed)
     {
        if (sd->fill_inside) sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_FIT;
        else sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_EXPAND;
     }
   else
     sd->scale_type = EFL_GFX_IMAGE_SCALE_METHOD_FILL;

   efl_canvas_group_change(obj);
}

EAPI Eina_Bool
elm_image_aspect_fixed_get(const Evas_Object *obj)
{
   EFL_UI_IMAGE_CHECK(obj) EINA_FALSE;
   EFL_UI_IMAGE_DATA_GET(obj, sd);
   return sd->aspect_fixed;
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_image, Efl_Ui_Image_Data)

/* Internal EO APIs and hidden overrides */

#define EFL_UI_IMAGE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_image), \
   EFL_CANVAS_GROUP_CALC_OPS(efl_ui_image)

#include "efl_ui_image.eo.c"

#include "efl_ui_image_legacy_eo.h"

#define MY_CLASS_NAME_LEGACY "elm_image"

static void
_efl_ui_image_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_image_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_IMAGE_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

EAPI Evas_Object *
elm_image_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = elm_legacy_add(EFL_UI_IMAGE_LEGACY_CLASS, parent);
   EFL_UI_IMAGE_DATA_GET(obj, priv);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _on_size_hints_changed, priv);

   return obj;
}

#include "efl_ui_image_legacy_eo.c"
