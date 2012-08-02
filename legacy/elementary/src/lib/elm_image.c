#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_image.h"

#ifdef _WIN32
# define FMT_SIZE_T "%Iu"
#else
# define FMT_SIZE_T "%zu"
#endif

EAPI const char ELM_IMAGE_SMART_NAME[] = "elm_image";

static const char SIG_DND[] = "drop";
static const char SIG_CLICKED[] = "clicked";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DND, ""},
   {SIG_CLICKED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_IMAGE_SMART_NAME, _elm_image, Elm_Image_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

static void
_on_image_preloaded(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj,
                    void *event __UNUSED__)
{
   Elm_Image_Smart_Data *sd = data;

   sd->preloading = EINA_FALSE;

   if (sd->show) evas_object_show(obj);
   if (sd->prev_img) evas_object_del(sd->prev_img);

   sd->prev_img = NULL;
}

static void
_on_mouse_up(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static Eina_Bool
_elm_image_animate_cb(void *data)
{
   Elm_Image_Smart_Data *sd = data;

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
   evas_object_repeat_events_set(img, EINA_TRUE);
   evas_object_event_callback_add
     (img, EVAS_CALLBACK_IMAGE_PRELOADED, _on_image_preloaded, sd);
   evas_object_event_callback_add
     (img, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, obj);

   evas_object_smart_member_add(img, obj);
   elm_widget_sub_object_add(obj, img);

   return img;
}

/* fixme: testar drag and drop depois! */

static void
_elm_image_internal_sizing_eval(Elm_Image_Smart_Data *sd)
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
        int iw = 0, ih = 0;

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
          (ELM_WIDGET_DATA(sd)->obj, &alignh, &alignv);

        if (alignh == EVAS_HINT_FILL) alignh = 0.5;
        if (alignv == EVAS_HINT_FILL) alignv = 0.5;

        x = sd->img_x + ((sd->img_w - w) * alignh);
        y = sd->img_y + ((sd->img_h - h) * alignv);

        evas_object_move(sd->img, x, y);
        evas_object_image_fill_set(sd->img, 0, 0, w, h);
        evas_object_resize(sd->img, w, h);
     }
}

static void
_elm_image_file_set_do(Evas_Object *obj)
{
   Evas_Object *pclip = NULL;

   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->prev_img) evas_object_del(sd->prev_img);
   if (sd->img)
     {
        pclip = evas_object_clip_get(sd->img);
        sd->prev_img = sd->img;
     }

   sd->img = _img_new(obj);

   evas_object_image_load_orientation_set(sd->img, EINA_TRUE);

   evas_object_clip_set(sd->img, pclip);

   sd->edje = EINA_FALSE;

   if (!sd->load_size)
     evas_object_image_load_size_set(sd->img, sd->load_size, sd->load_size);
}

/* WARNING: whenever you patch this function, remember to do the same
 * on elm_icon.c:_elm_icon_smart_file_set()'s 2nd half.
 */
static Eina_Bool
_elm_image_edje_file_set(Evas_Object *obj,
                         const char *file,
                         const char *group)
{
   Evas_Object *pclip;

   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->prev_img) evas_object_del(sd->prev_img);
   sd->prev_img = NULL;

   if (!sd->edje)
     {
        pclip = evas_object_clip_get(sd->img);
        if (sd->img) evas_object_del(sd->img);

        /* Edje object instead */
        sd->img = edje_object_add(evas_object_evas_get(obj));
        evas_object_smart_member_add(sd->img, obj);
        if (sd->show) evas_object_show(sd->img);
        evas_object_clip_set(sd->img, pclip);
     }

   sd->edje = EINA_TRUE;
   if (!edje_object_file_set(sd->img, file, group))
     {
        ERR("failed to set edje file '%s', group '%s': %s", file, group,
            edje_load_error_str(edje_object_load_error_get(sd->img)));
        return EINA_FALSE;
     }

   /* FIXME: do i want to update icon on file change ? */
   _elm_image_internal_sizing_eval(sd);

   return EINA_TRUE;
}

static void
_elm_image_smart_size_get(const Evas_Object *obj,
                          int *w,
                          int *h)
{
   int tw, th;
   int cw = 0, ch = 0;
   const char *type;

   ELM_IMAGE_DATA_GET(obj, sd);

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

static void
_elm_image_smart_smooth_scale_set(Evas_Object *obj,
                                  Eina_Bool smooth)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje) return;

   evas_object_image_smooth_scale_set(sd->img, smooth);
}

static Eina_Bool
_elm_image_smart_smooth_scale_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje) return EINA_FALSE;

   return evas_object_image_smooth_scale_get(sd->img);
}

static Evas_Object *
_elm_image_smart_object_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->img;
}

static void
_elm_image_smart_fill_inside_set(Evas_Object *obj,
                                 Eina_Bool fill_inside)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   fill_inside = !!fill_inside;

   if (sd->fill_inside == fill_inside) return;

   sd->fill_inside = fill_inside;

   _elm_image_internal_sizing_eval(sd);
}

static Eina_Bool
_elm_image_smart_fill_inside_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->fill_inside;
}

static void
_elm_image_smart_resize_up_set(Evas_Object *obj,
                              Eina_Bool resize_up)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   resize_up = !!resize_up;

   if (sd->resize_up == resize_up) return;

   sd->resize_up = resize_up;

   _elm_image_internal_sizing_eval(sd);
}

static Eina_Bool
_elm_image_smart_resize_up_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->resize_up;
}

static void
_elm_image_smart_resize_down_set(Evas_Object *obj,
                                Eina_Bool resize_down)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   resize_down = !!resize_down;

   if (sd->resize_down == resize_down) return;

   sd->resize_down = resize_down;

   _elm_image_internal_sizing_eval(sd);
}

static Eina_Bool
_elm_image_smart_resize_down_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->resize_up;
}

static void
_elm_image_flip_horizontal(Elm_Image_Smart_Data *sd)
{
   unsigned int *p1, *p2, tmp;
   unsigned int *data;
   int x, y, iw, ih;

   evas_object_image_size_get(sd->img, &iw, &ih);
   data = evas_object_image_data_get(sd->img, EINA_TRUE);

   for (y = 0; y < ih; y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((y + 1) * iw) - 1;
        for (x = 0; x < (iw >> 1); x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2--;
          }
     }

   evas_object_image_data_set(sd->img, data);
   evas_object_image_data_update_add(sd->img, 0, 0, iw, ih);

   _elm_image_internal_sizing_eval(sd);
}

static void
_elm_image_flip_vertical(Elm_Image_Smart_Data *sd)
{
   unsigned int *p1, *p2, tmp;
   unsigned int *data;
   int x, y, iw, ih;

   evas_object_image_size_get(sd->img, &iw, &ih);
   data = evas_object_image_data_get(sd->img, EINA_TRUE);

   for (y = 0; y < (ih >> 1); y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((ih - 1 - y) * iw);
        for (x = 0; x < iw; x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2++;
          }
     }

   evas_object_image_data_set(sd->img, data);
   evas_object_image_data_update_add(sd->img, 0, 0, iw, ih);

   _elm_image_internal_sizing_eval(sd);
}

static void
_elm_image_smart_rotate_180(Elm_Image_Smart_Data *sd)
{
   unsigned int *p1, *p2, tmp;
   unsigned int *data;
   int x, hw, iw, ih;

   evas_object_image_size_get(sd->img, &iw, &ih);
   data = evas_object_image_data_get(sd->img, 1);

   hw = iw * ih;
   x = (hw / 2);
   p1 = data;
   p2 = data + hw - 1;

   for (; --x > 0; )
     {
        tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
     }

   evas_object_image_data_set(sd->img, data);
   evas_object_image_data_update_add(sd->img, 0, 0, iw, ih);

   _elm_image_internal_sizing_eval(sd);
}

static void
_elm_image_smart_orient_set(Evas_Object *obj,
                            Elm_Image_Orient orient)
{
   unsigned int *data, *data2 = NULL, *to, *from;
   int x, y, w, hw, iw, ih;

   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje)
     return;

   switch (orient)
     {
      case ELM_IMAGE_FLIP_HORIZONTAL:
        _elm_image_flip_horizontal(sd);
	sd->orient = orient;
        return;

      case ELM_IMAGE_FLIP_VERTICAL:
        _elm_image_flip_vertical(sd);
	sd->orient = orient;
        return;

      case ELM_IMAGE_ROTATE_180:
        _elm_image_smart_rotate_180(sd);
	sd->orient = orient;
        return;

     case ELM_IMAGE_ORIENT_NONE:
        sd->orient = orient;
        return;

      default:
        break;
     }

   evas_object_image_size_get(sd->img, &iw, &ih);

   /* we need separate destination memory if we want to rotate 90 or
    * 270 degree */
   data = evas_object_image_data_get(sd->img, EINA_FALSE);
   if (!data) return;
   data2 = malloc(sizeof(unsigned char) * (iw * ih * 4));
   if (!data2) return;
   memcpy(data2, data, sizeof (unsigned char) * (iw * ih * 4));

   w = ih;
   ih = iw;
   iw = w;
   hw = w * ih;

   evas_object_image_size_set(sd->img, iw, ih);
   data = evas_object_image_data_get(sd->img, EINA_TRUE);

   switch (orient)
     {
      case ELM_IMAGE_FLIP_TRANSPOSE:
        to = data;
        hw = -hw + 1;
	sd->orient = orient;
        break;

      case ELM_IMAGE_FLIP_TRANSVERSE:
        to = data + hw - 1;
        w = -w;
        hw = hw - 1;
	sd->orient = orient;
        break;

      case ELM_IMAGE_ROTATE_90:
        to = data + w - 1;
        hw = -hw - 1;
	sd->orient = orient;
        break;

      case ELM_IMAGE_ROTATE_270:
        to = data + hw - w;
        w = -w;
        hw = hw + 1;
	sd->orient = orient;
        break;

      default:
        ERR("unknown orient %d", orient);
        evas_object_image_data_set(sd->img, data);  // give it back
        if (data2) free(data2);

        return;
     }

   from = data2;
   for (x = iw; --x >= 0; )
     {
        for (y = ih; --y >= 0; )
          {
             *to = *from;
             from++;
             to += w;
          }
        to += hw;
     }
   if (data2) free(data2);

   evas_object_image_data_set(sd->img, data);
   evas_object_image_data_update_add(sd->img, 0, 0, iw, ih);

   _elm_image_internal_sizing_eval(sd);
}

static Elm_Image_Orient
_elm_image_smart_orient_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->orient;
}

static Eina_Bool
_elm_image_drag_n_drop_cb(void *elm_obj,
                          Evas_Object *obj,
                          Elm_Selection_Data *drop)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->file_set
         (obj, drop->data, NULL))
     {
        printf("dnd: %s, %s, %s", elm_widget_type_get(elm_obj),
               SIG_DND, (char *)drop->data);

        evas_object_smart_callback_call(elm_obj, SIG_DND, drop->data);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_elm_image_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Image_Smart_Data);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.add(obj);

   /* starts as an Evas image. may switch to an Edje object */
   priv->img = _img_new(obj);
   priv->prev_img = NULL;

   priv->smooth = EINA_TRUE;
   priv->fill_inside = EINA_TRUE;
   priv->resize_up = EINA_TRUE;
   priv->resize_down = EINA_TRUE;
   priv->aspect_fixed = EINA_TRUE;

   priv->load_size = 64;
   priv->scale = 1.0;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(priv)->api)->load_size_set(obj, 0);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(priv)->api)->sizing_eval(obj);
}

static void
_elm_image_smart_del(Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->anim_timer)
     ecore_timer_del(sd->anim_timer);

   evas_object_del(sd->img);
   if (sd->prev_img) evas_object_del(sd->prev_img);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.del(obj);
}

static void
_elm_image_smart_move(Evas_Object *obj,
                      Evas_Coord x,
                      Evas_Coord y)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.move(obj, x, y);

   if ((sd->img_x == x) && (sd->img_y == y)) return;
   sd->img_x = x;
   sd->img_y = y;

   /* takes care of moving */
   _elm_image_internal_sizing_eval(sd);
}

static void
_elm_image_smart_resize(Evas_Object *obj,
                        Evas_Coord w,
                        Evas_Coord h)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.resize(obj, w, h);

   if ((sd->img_w == w) && (sd->img_h == h)) return;

   sd->img_w = w;
   sd->img_h = h;

   /* takes care of resizing */
   _elm_image_internal_sizing_eval(sd);
}

static void
_elm_image_smart_show(Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->show = EINA_TRUE;
   if (sd->preloading)
     return;

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.show(obj);

   evas_object_show(sd->img);

   if (sd->prev_img) evas_object_del(sd->prev_img);
   sd->prev_img = NULL;
}

static void
_elm_image_smart_hide(Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.hide(obj);

   sd->show = EINA_FALSE;
   evas_object_hide(sd->img);

   if (sd->prev_img) evas_object_del(sd->prev_img);
   sd->prev_img = NULL;
}

static void
_elm_image_smart_color_set(Evas_Object *obj,
                           int r,
                           int g,
                           int b,
                           int a)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.color_set(obj, r, g, b, a);

   evas_object_color_set(sd->img, r, g, b, a);
   if (sd->prev_img) evas_object_color_set(sd->prev_img, r, g, b, a);
}

static void
_elm_image_smart_clip_set(Evas_Object *obj,
                          Evas_Object *clip)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.clip_set(obj, clip);

   evas_object_clip_set(sd->img, clip);
   if (sd->prev_img) evas_object_clip_set(sd->prev_img, clip);
}

static void
_elm_image_smart_clip_unset(Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_image_parent_sc)->base.clip_unset(obj);

   evas_object_clip_unset(sd->img);
   if (sd->prev_img) evas_object_clip_unset(sd->prev_img);
}

static Eina_Bool
_elm_image_smart_theme(Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_image_parent_sc)->theme(obj))
     return EINA_FALSE;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_image_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   int w, h;
   double ts;

   ELM_IMAGE_DATA_GET(obj, sd);

   _elm_image_internal_sizing_eval(sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->smooth_scale_set
     (obj, sd->smooth);

   if (sd->no_scale)
     ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->scale_set(obj, 1.0);
   else
     ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->scale_set
       (obj, elm_widget_scale_get(obj) * elm_config_scale_get());

   ts = sd->scale;
   sd->scale = 1.0;
   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->size_get(obj, &w, &h);
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

static Eina_Bool
_elm_image_smart_memfile_set(Evas_Object *obj,
                             const void *img,
                             size_t size,
                             const char *format,
                             const char *key)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   _elm_image_file_set_do(obj);

   evas_object_image_memfile_set
     (sd->img, (void *)img, size, (char *)format, (char *)key);

   sd->preloading = EINA_TRUE;
   sd->show = EINA_TRUE;

   evas_object_hide(sd->img);
   evas_object_image_preload(sd->img, EINA_FALSE);
   if (evas_object_image_load_error_get(sd->img) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for some random " FMT_SIZE_T
            " byte chunk of memory (%p)", size, sd->img);
        return EINA_FALSE;
     }

   _elm_image_internal_sizing_eval(sd);

   return EINA_TRUE;
}

static Eina_Bool
_elm_image_smart_file_set(Evas_Object *obj,
                          const char *file,
                          const char *key)
{
   Evas_Coord w, h;

   ELM_IMAGE_DATA_GET(obj, sd);

   if (eina_str_has_extension(file, ".edj"))
     return _elm_image_edje_file_set(obj, file, key);

   _elm_image_file_set_do(obj);

   evas_object_image_file_set(sd->img, file, key);

   sd->preloading = EINA_TRUE;
   sd->show = EINA_TRUE;

   evas_object_hide(sd->img);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->size_get(obj, &w, &h);

   evas_object_image_load_size_set(sd->img, w, h);

   evas_object_image_preload(sd->img, EINA_FALSE);
   if (evas_object_image_load_error_get(sd->img) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Things are going bad for '%s' (%p)", file, sd->img);
        return EINA_FALSE;
     }

   _elm_image_internal_sizing_eval(sd);

   return EINA_TRUE;
}

static void
_elm_image_smart_file_get(const Evas_Object *obj,
                          const char **file,
                          const char **key)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje)
     edje_object_file_get(sd->img, file, key);
   else
     evas_object_image_file_get(sd->img, file, key);
}

static void
_elm_image_smart_preload_set(Evas_Object *obj,
                             Eina_Bool disable)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje) return;

   evas_object_image_preload(sd->img, disable);
   sd->preloading = !disable;
}

static void
_elm_image_smart_load_size_set(Evas_Object *obj,
                               int size)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->load_size = size;
   if (!sd->img || sd->edje) return;

   evas_object_image_load_size_set(sd->img, sd->load_size, sd->load_size);
}

static int
_elm_image_smart_load_size_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->load_size;
}

static void
_elm_image_smart_scale_set(Evas_Object *obj,
                           double scale)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->scale = scale;

   _elm_image_internal_sizing_eval(sd);
}

static double
_elm_image_smart_scale_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->scale;
}

/**
 * Turns on editing through drag and drop and copy and paste.
 */
static void
_elm_image_smart_edit_set(Evas_Object *obj,
                          Eina_Bool edit,
                          Evas_Object *parent)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje)
     {
        printf("No editing edje objects yet (ever)\n");
        return;
     }

   edit = !!edit;

   if (edit == sd->edit) return;

   sd->edit = edit;

   if (sd->edit)
     elm_drop_target_add
       (obj, ELM_SEL_FORMAT_IMAGE, _elm_image_drag_n_drop_cb, parent);
   else
     elm_drop_target_del(obj);
}

static Eina_Bool
_elm_image_smart_edit_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->edit;
}

static void
_elm_image_smart_aspect_fixed_set(Evas_Object *obj,
                                  Eina_Bool fixed)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   fixed = !!fixed;
   if (sd->aspect_fixed == fixed) return;

   sd->aspect_fixed = fixed;

   _elm_image_internal_sizing_eval(sd);
}

static Eina_Bool
_elm_image_smart_aspect_fixed_get(const Evas_Object *obj)
{
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->aspect_fixed;
}

static void
_elm_image_smart_set_user(Elm_Image_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_image_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_image_smart_del,
   ELM_WIDGET_CLASS(sc)->base.move = _elm_image_smart_move,
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_image_smart_resize,
   ELM_WIDGET_CLASS(sc)->base.show = _elm_image_smart_show,
   ELM_WIDGET_CLASS(sc)->base.hide = _elm_image_smart_hide,
   ELM_WIDGET_CLASS(sc)->base.color_set = _elm_image_smart_color_set,
   ELM_WIDGET_CLASS(sc)->base.clip_set = _elm_image_smart_clip_set,
   ELM_WIDGET_CLASS(sc)->base.clip_unset = _elm_image_smart_clip_unset,

   ELM_WIDGET_CLASS(sc)->theme = _elm_image_smart_theme;

   sc->aspect_fixed_get = _elm_image_smart_aspect_fixed_get;
   sc->aspect_fixed_set = _elm_image_smart_aspect_fixed_set;
   sc->edit_get = _elm_image_smart_edit_get;
   sc->edit_set = _elm_image_smart_edit_set;
   sc->file_get = _elm_image_smart_file_get;
   sc->file_set = _elm_image_smart_file_set;
   sc->fill_inside_get = _elm_image_smart_fill_inside_get;
   sc->fill_inside_set = _elm_image_smart_fill_inside_set;
   sc->image_object_get = _elm_image_smart_object_get;
   sc->load_size_get = _elm_image_smart_load_size_get;
   sc->load_size_set = _elm_image_smart_load_size_set;
   sc->memfile_set = _elm_image_smart_memfile_set;
   sc->orient_get = _elm_image_smart_orient_get;
   sc->orient_set = _elm_image_smart_orient_set;
   sc->preload_set = _elm_image_smart_preload_set;
   sc->resize_down_get = _elm_image_smart_resize_down_get;
   sc->resize_down_set = _elm_image_smart_resize_down_set;
   sc->scale_get = _elm_image_smart_scale_get;
   sc->scale_set = _elm_image_smart_scale_set;
   sc->resize_up_get = _elm_image_smart_resize_up_get;
   sc->resize_up_set = _elm_image_smart_resize_up_set;
   sc->size_get = _elm_image_smart_size_get;
   sc->sizing_eval = _elm_image_smart_sizing_eval;
   sc->smooth_scale_get = _elm_image_smart_smooth_scale_get;
   sc->smooth_scale_set = _elm_image_smart_smooth_scale_set;
}

EAPI const Elm_Image_Smart_Class *
elm_image_smart_class_get(void)
{
   static Elm_Image_Smart_Class _sc =
     ELM_IMAGE_SMART_CLASS_INIT_NAME_VERSION(ELM_IMAGE_SMART_NAME);
   static const Elm_Image_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_image_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_image_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_image_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Eina_Bool
elm_image_memfile_set(Evas_Object *obj,
                      const void *img,
                      size_t size,
                      const char *format,
                      const char *key)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(img, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!size, EINA_FALSE);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->memfile_set
            (obj, img, size, format, key);
}

EAPI Eina_Bool
elm_image_file_set(Evas_Object *obj,
                   const char *file,
                   const char *group)
{
   Eina_Bool ret;

   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   ret = ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->file_set(obj, file, group);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);

   return ret;
}

EAPI void
elm_image_file_get(const Evas_Object *obj,
                   const char **file,
                   const char **group)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->file_get(obj, file, group);
}

EAPI void
elm_image_smooth_set(Evas_Object *obj,
                     Eina_Bool smooth)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->smooth = smooth;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
}

EAPI Eina_Bool
elm_image_smooth_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->smooth;
}

EAPI void
elm_image_object_size_get(const Evas_Object *obj,
                          int *w,
                          int *h)
{
   if (w) *w = 0;
   if (h) *h = 0;

   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->size_get(obj, w, h);
}

EAPI void
elm_image_no_scale_set(Evas_Object *obj,
                       Eina_Bool no_scale)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->no_scale = no_scale;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
}

EAPI Eina_Bool
elm_image_no_scale_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->no_scale;
}

EAPI void
elm_image_resizable_set(Evas_Object *obj,
                        Eina_Bool up,
                        Eina_Bool down)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->resize_up = !!up;
   sd->resize_down = !!down;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
}

EAPI void
elm_image_resizable_get(const Evas_Object *obj,
                        Eina_Bool *size_up,
                        Eina_Bool *size_down)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   if (size_up) *size_up = sd->resize_up;
   if (size_down) *size_down = sd->resize_down;
}

EAPI void
elm_image_fill_outside_set(Evas_Object *obj,
                           Eina_Bool fill_outside)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   sd->fill_inside = !fill_outside;

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->sizing_eval(obj);
}

EAPI Eina_Bool
elm_image_fill_outside_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return !sd->fill_inside;
}

EAPI void
elm_image_preload_disabled_set(Evas_Object *obj,
                               Eina_Bool disabled)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->preload_set(obj, !!disabled);
}

EAPI void
elm_image_prescale_set(Evas_Object *obj,
                       int size)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->load_size_set(obj, size);
}

EAPI int
elm_image_prescale_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) 0;
   ELM_IMAGE_DATA_GET(obj, sd);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->load_size_get(obj);
}

EAPI void
elm_image_orient_set(Evas_Object *obj,
                     Elm_Image_Orient orient)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->orient_set(obj, orient);
}

EAPI Elm_Image_Orient
elm_image_orient_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) ELM_IMAGE_ORIENT_NONE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->orient_get(obj);
}

EAPI void
elm_image_editable_set(Evas_Object *obj,
                       Eina_Bool set)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->edit_set(obj, set, obj);
}

EAPI Eina_Bool
elm_image_editable_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->edit_get(obj);
}

EAPI Evas_Object *
elm_image_object_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) NULL;
   ELM_IMAGE_DATA_GET(obj, sd);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->image_object_get(obj);
}

EAPI void
elm_image_aspect_fixed_set(Evas_Object *obj,
                           Eina_Bool fixed)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->aspect_fixed_set(obj, fixed);
}

EAPI Eina_Bool
elm_image_aspect_fixed_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return ELM_IMAGE_CLASS(ELM_WIDGET_DATA(sd)->api)->aspect_fixed_get(obj);
}

EAPI Eina_Bool
elm_image_animated_available_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   if (sd->edje) return EINA_FALSE;

   return evas_object_image_animated_get(elm_image_object_get(obj));
}

EAPI void
elm_image_animated_set(Evas_Object *obj,
                       Eina_Bool anim)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   anim = !!anim;
   if (sd->anim == anim) return;

   if (sd->edje) return;

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
   sd->anim = anim;

   return;
}

EAPI Eina_Bool
elm_image_animated_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->anim;
}

EAPI void
elm_image_animated_play_set(Evas_Object *obj,
                            Eina_Bool play)
{
   ELM_IMAGE_CHECK(obj);
   ELM_IMAGE_DATA_GET(obj, sd);

   if (!sd->anim) return;
   if (sd->play == play) return;

   if (sd->edje) return;

   if (play)
     {
        sd->anim_timer = ecore_timer_add
            (sd->frame_duration, _elm_image_animate_cb, sd);
     }
   else
     {
        if (sd->anim_timer)
          {
             ecore_timer_del(sd->anim_timer);
             sd->anim_timer = NULL;
          }
     }
   sd->play = play;
}

EAPI Eina_Bool
elm_image_animated_play_get(const Evas_Object *obj)
{
   ELM_IMAGE_CHECK(obj) EINA_FALSE;
   ELM_IMAGE_DATA_GET(obj, sd);

   return sd->play;
}
