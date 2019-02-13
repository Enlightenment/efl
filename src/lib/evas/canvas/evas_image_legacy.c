
#include "evas_image_private.h"
#include "evas_image.eo.h"

#define EVAS_IMAGE_API(_o, ...) do { \
   if (EINA_UNLIKELY(!efl_isa(_o, EFL_CANVAS_IMAGE_INTERNAL_CLASS))) { \
      EINA_SAFETY_ERROR("object is not an image!"); \
      return __VA_ARGS__; \
   } } while (0)

#define EVAS_IMAGE_LEGACY_API(_o, ...) do { \
   EVAS_OBJECT_LEGACY_API(_o, __VA_ARGS__); \
   EVAS_IMAGE_API(_o, __VA_ARGS__); \
   } while (0)

typedef struct _Evas_Image_Legacy_Pixels_Entry Evas_Image_Legacy_Pixels_Entry;

struct _Evas_Image_Legacy_Pixels_Entry
{
   Eo    *object;
   void  *image;
};

EAPI Evas_Object *
evas_object_image_add(Evas *eo_e)
{
   eo_e = evas_find(eo_e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(eo_e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EVAS_IMAGE_CLASS, eo_e,
                 efl_gfx_fill_auto_set(efl_added, EINA_FALSE),
                 efl_canvas_object_legacy_ctor(efl_added));
}

EAPI Evas_Object *
evas_object_image_filled_add(Evas *eo_e)
{
   eo_e = evas_find(eo_e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(eo_e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EVAS_IMAGE_CLASS, eo_e,
                 efl_canvas_object_legacy_ctor(efl_added));
}

EAPI void
evas_object_image_memfile_set(Evas_Object *eo_obj, void *data, int size, char *format EINA_UNUSED, char *key)
{
   Eina_File *f;

   EVAS_IMAGE_API(eo_obj);

   f = eina_file_virtualize(NULL, data, size, EINA_TRUE);
   if (!f) return ;
   efl_file_mmap_set(eo_obj, f, key);
   eina_file_close(f);
}

EAPI void
evas_object_image_fill_set(Evas_Object *obj,
                           Evas_Coord x, Evas_Coord y,
                           Evas_Coord w, Evas_Coord h)
{
   EVAS_IMAGE_API(obj);
   _evas_image_fill_set(obj, efl_data_scope_get(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS), x, y, w, h);
}

EAPI void
evas_object_image_preload(Evas_Object *eo_obj, Eina_Bool cancel)
{
   EVAS_IMAGE_API(eo_obj);
   if (cancel) _evas_image_load_async_cancel(eo_obj);
   else _evas_image_load_async_start(eo_obj);
}

EAPI Eina_Bool
evas_object_image_filled_get(const Evas_Object *eo_obj)
{
   EVAS_IMAGE_API(eo_obj, EINA_FALSE);
   return efl_gfx_fill_auto_get(eo_obj);
}

EAPI void
evas_object_image_filled_set(Evas_Object *eo_obj, Eina_Bool value)
{
   EVAS_IMAGE_API(eo_obj);
   efl_gfx_fill_auto_set(eo_obj, value);
}

EAPI void
evas_object_image_fill_get(const Evas_Object *obj,
                           Evas_Coord *x, Evas_Coord *y,
                           Evas_Coord *w, Evas_Coord *h)
{
   Eina_Rect r;

   EVAS_IMAGE_API(obj);
   r = efl_gfx_fill_get(obj);
   if (x) *x = r.x;
   if (y) *y = r.y;
   if (w) *w = r.w;
   if (h) *h = r.h;
}

EAPI void
evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool alpha)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_buffer_alpha_set(obj, alpha);
}

EAPI Eina_Bool
evas_object_image_alpha_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return efl_gfx_buffer_alpha_get(obj);
}

EAPI void
evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_image_border_set(obj, l, r, t, b);
}

EAPI void
evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_image_border_get(obj, l, r, t, b);
}

EAPI void
evas_object_image_border_scale_set(Evas_Object *obj, double scale)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_image_border_scale_set(obj, scale);
}

EAPI double
evas_object_image_border_scale_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 0.0);
   return efl_gfx_image_border_scale_get(obj);
}

EAPI void
evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_image_border_center_fill_set(obj, (Efl_Gfx_Border_Fill_Mode) fill);
}

EAPI Evas_Border_Fill_Mode
evas_object_image_border_center_fill_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_BORDER_FILL_NONE);
   return (Evas_Border_Fill_Mode) efl_gfx_image_border_center_fill_get(obj);
}

EAPI void
evas_object_image_size_get(const Evas_Object *obj, int *w, int *h)
{
   Eina_Size2D sz;
   EVAS_IMAGE_API(obj);
   sz = efl_gfx_view_size_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI Evas_Colorspace
evas_object_image_colorspace_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_COLORSPACE_ARGB8888);
   return (Evas_Colorspace) efl_gfx_buffer_colorspace_get(obj);
}

EAPI int
evas_object_image_stride_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 0);
   Evas_Image_Data *o = efl_data_scope_get(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   return o->cur->image.stride;
}

EAPI void
evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h)
{
   Eina_Rect r;

   EVAS_IMAGE_API(obj);
   r = EINA_RECT(x, y, w, h);
   efl_gfx_buffer_update_add(obj, &r);
}

EAPI void
evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key)
{
   EVAS_IMAGE_API(obj);
   efl_file_set(obj, file, key);
}

EAPI void
evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   EVAS_IMAGE_API(obj);
   efl_file_get(obj, file, key);
}

EAPI void
evas_object_image_mmap_set(Evas_Object *obj, const Eina_File *f, const char *key)
{
   EVAS_IMAGE_API(obj);
   _evas_image_mmap_set(obj, f, key);
}

EAPI void
evas_object_image_mmap_get(const Evas_Object *obj, const Eina_File **f, const char **key)
{
   EVAS_IMAGE_API(obj);
   _evas_image_mmap_get(obj, f, key);
}

EAPI Eina_Bool
evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return efl_file_save(obj, file, key, flags);
}

EAPI Eina_Bool
evas_object_image_animated_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return _evas_image_animated_get(obj);
}

EAPI void
evas_object_image_animated_frame_set(Evas_Object *obj, int frame_index)
{
   EVAS_IMAGE_API(obj);
   _evas_image_animated_frame_set(obj, frame_index);
}

EAPI int
evas_object_image_animated_frame_count_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 0);
   return _evas_image_animated_frame_count_get(obj);
}

EAPI Evas_Image_Animated_Loop_Hint
evas_object_image_animated_loop_type_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_IMAGE_ANIMATED_HINT_NONE);
   return (Evas_Image_Animated_Loop_Hint) _evas_image_animated_loop_type_get(obj);
}

EAPI int
evas_object_image_animated_loop_count_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 0);
   return _evas_image_animated_loop_count_get(obj);
}

EAPI double
evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int frame_num)
{
   EVAS_IMAGE_API(obj, 0.0);
   return _evas_image_animated_frame_duration_get(obj, start_frame, frame_num);
}

EAPI void
evas_object_image_load_size_set(Evas_Object *obj, int w, int h)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_size_set(obj, w, h);
}

EAPI void
evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_size_get(obj, w, h);
}

EAPI void
evas_object_image_load_dpi_set(Evas_Object *obj, double dpi)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_dpi_set(obj, dpi);
}

EAPI double
evas_object_image_load_dpi_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 0.0);
   return _evas_image_load_dpi_get(obj);
}

EAPI void
evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_region_set(obj, x, y, w, h);
}

EAPI void
evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_region_get(obj, x, y, w, h);
}

EAPI Eina_Bool
evas_object_image_region_support_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return _evas_image_load_region_support_get(obj);
}

EAPI void
evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_orientation_set(obj, enable);
}

EAPI Eina_Bool
evas_object_image_load_orientation_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return _evas_image_load_orientation_get(obj);
}

EAPI void
evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_scale_down_set(obj, scale_down);
}

EAPI int
evas_object_image_load_scale_down_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, 1);
   return _evas_image_load_scale_down_get(obj);
}

EAPI void
evas_object_image_load_head_skip_set(Evas_Object *obj, Eina_Bool skip)
{
   EVAS_IMAGE_API(obj);
   _evas_image_load_head_skip_set(obj, skip);
}

EAPI Eina_Bool
evas_object_image_load_head_skip_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return _evas_image_load_head_skip_get(obj);
}

EAPI Evas_Load_Error
evas_object_image_load_error_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_LOAD_ERROR_GENERIC);
   return _evas_image_load_error_get(obj);
}

EAPI void
evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale)
{
   EVAS_IMAGE_API(obj);
   efl_gfx_image_smooth_scale_set(obj, smooth_scale);
}

EAPI Eina_Bool
evas_object_image_smooth_scale_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return efl_gfx_image_smooth_scale_get(obj);
}

EAPI void
evas_object_image_orient_set(Evas_Object *obj, Evas_Image_Orient orient)
{
   EVAS_IMAGE_API(obj);

   Evas_Image_Data *o = efl_data_scope_get(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   _evas_image_orientation_set(obj, o, orient);
}

EAPI Evas_Image_Orient
evas_object_image_orient_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_IMAGE_ORIENT_NONE);

   Evas_Image_Data *o = efl_data_scope_get(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   return o->cur->orient;
}

EAPI void
evas_object_image_snapshot_set(Evas_Object *eo, Eina_Bool s)
{
   EVAS_IMAGE_API(eo);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);

   if (obj->cur->snapshot == s) return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     state_write->snapshot = !!s;
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
}

EAPI Eina_Bool
evas_object_image_snapshot_get(const Evas_Object *eo)
{
   EVAS_IMAGE_API(eo, EINA_FALSE);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);
   return obj->cur->snapshot;
}

EAPI Eina_Bool
evas_object_image_source_set(Evas_Object *eo, Evas_Object *src)
{
   EVAS_IMAGE_API(eo, EINA_FALSE);
   return _evas_image_proxy_source_set(eo, src);
}

EAPI Evas_Object *
evas_object_image_source_get(const Evas_Object *eo)
{
   EVAS_IMAGE_API(eo, NULL);
   return _evas_image_proxy_source_get(eo);
}

EAPI Eina_Bool
evas_object_image_source_unset(Evas_Object *eo_obj)
{
   EVAS_IMAGE_API(eo_obj, EINA_FALSE);
   return _evas_image_proxy_source_set(eo_obj, NULL);
}

EAPI void
evas_object_image_source_clip_set(Evas_Object *eo, Eina_Bool source_clip)
{
   EVAS_IMAGE_API(eo);
   _evas_image_proxy_source_clip_set(eo, source_clip);
}

EAPI Eina_Bool
evas_object_image_source_clip_get(const Evas_Object *eo)
{
   EVAS_IMAGE_API(eo, EINA_FALSE);
   return _evas_image_proxy_source_clip_get(eo);
}

EAPI void
evas_object_image_source_events_set(Evas_Object *eo, Eina_Bool repeat)
{
   EVAS_IMAGE_API(eo);
   _evas_image_proxy_source_events_set(eo, repeat);
}

EAPI Eina_Bool
evas_object_image_source_events_get(const Evas_Object *eo)
{
   EVAS_IMAGE_API(eo, EINA_FALSE);
   return _evas_image_proxy_source_events_get(eo);
}

EAPI void
evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint)
{
   EVAS_IMAGE_API(obj);
   return efl_gfx_image_content_hint_set(obj, hint);
}

EAPI Evas_Image_Content_Hint
evas_object_image_content_hint_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_IMAGE_CONTENT_HINT_NONE);
   return efl_gfx_image_content_hint_get(obj);
}

EAPI void
evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint)
{
   EVAS_IMAGE_API(obj);
   return efl_gfx_image_scale_hint_set(obj, (Efl_Gfx_Image_Scale_Hint) hint);
}

EAPI Evas_Image_Scale_Hint
evas_object_image_scale_hint_get(const Evas_Object *obj)
{
   EVAS_IMAGE_API(obj, EVAS_IMAGE_SCALE_HINT_NONE);
   return (Evas_Image_Scale_Hint) efl_gfx_image_scale_hint_get(obj);
}

EAPI void
evas_object_image_native_surface_set(Evas_Object *eo_obj, Evas_Native_Surface *surf)
{
   EVAS_IMAGE_API(eo_obj);

   Eina_Bool ret;

   ret = _evas_image_native_surface_set(eo_obj, surf);

   if (surf && !ret)
     {
        Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

        o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
}

EAPI Evas_Native_Surface *
evas_object_image_native_surface_get(const Evas_Object *eo_obj)
{
   EVAS_IMAGE_API(eo_obj, NULL);
   return _evas_image_native_surface_get(eo_obj);
}

EAPI void
evas_object_image_pixels_get_callback_set(Eo *eo_obj, Evas_Object_Image_Pixels_Get_Cb func, void *data)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   evas_object_async_block(obj);
   EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
     {
        pixi_write->func.get_pixels = func;
        pixi_write->func.get_pixels_data = data;
     }
   EINA_COW_PIXEL_WRITE_END(o, pixi_write);
}

EAPI void
evas_object_image_pixels_dirty_set(Eo *eo_obj, Eina_Bool dirty)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   evas_object_async_block(obj);
   if (dirty)
     {
        o->dirty_pixels = EINA_TRUE;
        o->changed = EINA_TRUE;
     }
   else o->dirty_pixels = EINA_FALSE;

   evas_object_change(eo_obj, obj);
}

EAPI Eina_Bool
evas_object_image_pixels_dirty_get(const Eo *eo_obj)
{
   EVAS_IMAGE_API(eo_obj, EINA_FALSE);

   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   return (o->dirty_pixels ? 1 : 0);
}

EAPI void
evas_object_image_data_set(Eo *eo_obj, void *data)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   void *p_data, *pixels;
   Eina_Bool resize_call = EINA_FALSE;


   evas_object_async_block(obj);
   evas_render_rendering_wait(obj->layer->evas);

   _evas_image_cleanup(eo_obj, obj, o);
   p_data = o->engine_data;
   if (data)
     {
        // r/o FBO data_get: only free the image, don't update pixels
        if ((pixels = eina_hash_find(o->pixels->images_to_free, data)) != NULL)
          {
             eina_hash_del(o->pixels->images_to_free, data, pixels);
             return;
          }

        if (o->engine_data)
          {
             o->engine_data = ENFN->image_data_put(ENC, o->engine_data, data);
          }
        else
          {
             o->engine_data = ENFN->image_new_from_data(ENC,
                                                        o->cur->image.w,
                                                        o->cur->image.h,
                                                        data,
                                                        o->cur->has_alpha,
                                                        o->cur->cspace);
          }
        if (o->engine_data)
          {
             int stride = 0;

             if (ENFN->image_scale_hint_set)
               ENFN->image_scale_hint_set(ENC, o->engine_data, o->scale_hint);

             if (ENFN->image_content_hint_set)
               ENFN->image_content_hint_set(ENC, o->engine_data, o->content_hint);

             if (ENFN->image_stride_get)
               ENFN->image_stride_get(ENC, o->engine_data, &stride);
             else
               stride = o->cur->image.w * 4;

             if (o->cur->image.stride != stride)
               {
                  EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                    state_write->image.stride = stride;
                  EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
               }
         }
       o->written = EINA_TRUE;
     }
   else
     {
        if (o->engine_data)
          {
             ENFN->image_free(ENC, o->engine_data);
             o->changed = EINA_TRUE;
             evas_object_change(eo_obj, obj);
          }
        o->load_error = EVAS_LOAD_ERROR_NONE;
        if ((o->cur->image.w != 0) || (o->cur->image.h != 0))
          resize_call = EINA_TRUE;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             state_write->image.w = 0;
             state_write->image.h = 0;
             state_write->image.stride = 0;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

        o->engine_data = NULL;
     }
/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
*/
   if (o->pixels_checked_out > 0) o->pixels_checked_out--;
   if (p_data != o->engine_data)
     {
        o->pixels_checked_out = 0;
     }
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
}

static void
_image_to_free_del_cb(void *data)
{
   Evas_Image_Legacy_Pixels_Entry *px_entry = data;
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_safe_get(px_entry->object, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   ENFN->image_free(ENC, px_entry->image);
   free(px_entry);
}

EAPI void*
evas_object_image_data_get(const Eo *eo_obj, Eina_Bool for_writing)
{
   EVAS_IMAGE_API(eo_obj, NULL);

   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   Evas_Image_Legacy_Pixels_Entry *px_entry = NULL;
   Eina_Bool tofree = 0;
   void *pixels = NULL;
   int stride = 0;
   DATA32 *data;

   if (!o->engine_data) return NULL;

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (for_writing) evas_object_async_block(obj);
   if (for_writing) evas_render_rendering_wait(obj->layer->evas);

   data = NULL;
   if (ENFN->image_scale_hint_set)
     ENFN->image_scale_hint_set(ENC, o->engine_data, o->scale_hint);
   if (ENFN->image_content_hint_set)
     ENFN->image_content_hint_set(ENC, o->engine_data, o->content_hint);
   pixels = ENFN->image_data_get(ENC, o->engine_data, for_writing, &data, &o->load_error, &tofree);

   /* if we fail to get engine_data, we have to return NULL */
   if (!pixels || !data) goto error;

   if (!tofree)
     {
        o->engine_data = pixels;
        if (ENFN->image_stride_get)
          ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
           stride = o->cur->image.w * 4;

        if (o->cur->image.stride != stride)
          {
             EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                   state_write->image.stride = stride;
             EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
          }

        o->pixels_checked_out++;
     }
   else
     {
        Eina_Hash *hash = o->pixels->images_to_free;

        if (!hash)
          {
             hash = eina_hash_pointer_new(_image_to_free_del_cb);
             if (!hash) goto error;
             EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
               pixi_write->images_to_free = hash;
             EINA_COW_PIXEL_WRITE_END(o, pixi_write);
          }

        px_entry = calloc(1, sizeof(*px_entry));
        px_entry->object = (Eo *) eo_obj;
        px_entry->image = pixels;
        if (!eina_hash_add(hash, data, px_entry))
          goto error;
     }

   if (for_writing)
     {
        o->written = EINA_TRUE;
     }

   return data;

error:
   free(px_entry);
   if (tofree && pixels)
     ENFN->image_free(ENC, pixels);
   return NULL;
}

EAPI void
evas_object_image_data_copy_set(Eo *eo_obj, void *data)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   if (!data) return;
   evas_object_async_block(obj);
   _evas_image_cleanup(eo_obj, obj, o);
   if ((o->cur->image.w <= 0) ||
       (o->cur->image.h <= 0)) return;
   if (o->engine_data)
     ENFN->image_free(ENC, o->engine_data);
   o->engine_data = ENFN->image_new_from_copied_data(ENC,
                                                     o->cur->image.w,
                                                     o->cur->image.h,
                                                     data,
                                                     o->cur->has_alpha,
                                                     o->cur->cspace);
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data =
          ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
        if (ENFN->image_scale_hint_set)
          ENFN->image_scale_hint_set(ENC, o->engine_data, o->scale_hint);
        if (ENFN->image_content_hint_set)
          ENFN->image_content_hint_set(ENC, o->engine_data, o->content_hint);
        if (ENFN->image_stride_get)
          ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
          stride = o->cur->image.w * 4;

        if (o->cur->image.stride != stride)
          {
             EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
               state_write->image.stride = stride;
             EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
          }
        o->written = EINA_TRUE;
     }
   o->pixels_checked_out = 0;
}

/* Evas_Object equivalent: pixels_set(null, w, h, cspace) to (re)allocate an image */
EAPI void
evas_object_image_size_set(Evas_Object *eo_obj, int w, int h)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   int stride = 0;

   if (o->cur->scene) return;

   evas_object_async_block(obj);
   _evas_image_cleanup(eo_obj, obj, o);
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w >= 32768) return;
   if (h >= 32768) return;
   if ((w == o->cur->image.w) &&
       (h == o->cur->image.h)) return;

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->image.w = w;
        state_write->image.h = h;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->engine_data)
      o->engine_data = ENFN->image_size_set(ENC, o->engine_data, w, h);
   else
      o->engine_data = ENFN->image_new_from_copied_data
        (ENC, w, h, NULL, o->cur->has_alpha, o->cur->cspace);

   if (o->engine_data)
     {
        if (ENFN->image_scale_hint_set)
           ENFN->image_scale_hint_set(ENC, o->engine_data, o->scale_hint);
        if (ENFN->image_content_hint_set)
           ENFN->image_content_hint_set(ENC, o->engine_data, o->content_hint);
        if (ENFN->image_stride_get)
           ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
           stride = w * 4;
     }
   else
      stride = w * 4;
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, cur_write)
     {
        cur_write->image.stride = stride;

/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
*/
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, prev_write)
          EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(cur_write, prev_write);
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, prev_write);
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, cur_write);

   o->written = EINA_TRUE;
   o->changed = EINA_TRUE;
   evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

/* Evas_Object equivalent: pixels_set(null, w, h, cspace) to (re)allocate an image */
EAPI void
evas_object_image_colorspace_set(Evas_Object *eo_obj, Evas_Colorspace cspace)
{
   EVAS_IMAGE_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   evas_object_async_block(obj);
   _evas_image_cleanup(eo_obj, obj, o);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->cspace = cspace;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->engine_data)
     ENFN->image_colorspace_set(ENC, o->engine_data, cspace);
}

/* old video surfaces */

EAPI void
evas_object_image_video_surface_set(Evas_Object *eo_obj, Evas_Video_Surface *surf)
{
   EVAS_IMAGE_LEGACY_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   evas_object_async_block(obj);

   _evas_image_cleanup(eo_obj, obj, o);
   if (o->video_surface)
     {
        o->video_surface = EINA_FALSE;
        obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, eo_obj);
     }

   if (surf)
     {
        if (surf->version != EVAS_VIDEO_SURFACE_VERSION) return;

        if (!surf->update_pixels ||
            !surf->move ||
            !surf->resize ||
            !surf->hide ||
            !surf->show)
          return;

        o->created = EINA_TRUE;
        o->video_surface = EINA_TRUE;

        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
          pixi_write->video = *surf;
        EINA_COW_PIXEL_WRITE_END(o, pixi_write)

        obj->layer->evas->video_objects = eina_list_append(obj->layer->evas->video_objects, eo_obj);
     }
   else
     {
        if (!o->video_surface &&
            !o->pixels->video.update_pixels &&
            !o->pixels->video.move &&
            !o->pixels->video.resize &&
            !o->pixels->video.hide &&
            !o->pixels->video.show &&
            !o->pixels->video.data)
          return;

        o->video_surface = EINA_FALSE;
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
          {
             pixi_write->video.update_pixels = NULL;
             pixi_write->video.move = NULL;
             pixi_write->video.resize = NULL;
             pixi_write->video.hide = NULL;
             pixi_write->video.show = NULL;
             pixi_write->video.data = NULL;
          }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write)
     }
}

EAPI const Evas_Video_Surface*
evas_object_image_video_surface_get(const Evas_Object *eo_obj)
{
   EVAS_IMAGE_LEGACY_API(eo_obj, NULL);

   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   return (!o->video_surface ? NULL : &o->pixels->video);
}

EAPI void
evas_object_image_video_surface_caps_set(Evas_Object *eo_obj, unsigned int caps)
{
   EVAS_IMAGE_LEGACY_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   evas_object_async_block(obj);

   _evas_image_cleanup(eo_obj, obj, o);

   if (caps == o->pixels->video_caps)
     return;

   EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
     pixi_write->video_caps = caps;
   EINA_COW_PIXEL_WRITE_END(o, pixi_write)
}

EAPI unsigned int
evas_object_image_video_surface_caps_get(const Evas_Object *eo_obj)
{
   EVAS_IMAGE_LEGACY_API(eo_obj, 0);

   Evas_Image_Data *o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);

   /* The generic hardware plane code calls this function on
    * non-video surfaces, return stacking check for those to
    * allow them to use common video surface code */
   return (!o->video_surface ? EVAS_VIDEO_SURFACE_STACKING_CHECK : o->pixels->video_caps);
}

/* deprecated */
EAPI void
evas_object_image_fill_spread_set(Evas_Object *obj EINA_UNUSED, Evas_Fill_Spread spread)
{
   /* not implemented! */
   if (spread != EFL_GFX_FILL_REPEAT)
     WRN("Fill spread support is not implemented!");
}

/* deprecated */
EAPI Evas_Fill_Spread
evas_object_image_fill_spread_get(const Evas_Object *obj EINA_UNUSED)
{
   return EFL_GFX_FILL_REPEAT;
}

/* deprecated */
EAPI void
evas_object_image_source_visible_set(Evas_Object *eo, Eina_Bool visible)
{
   /* FIXME: I'd love to remove this feature and replace by no_render.
    * But they are not 100% equivalent: if all proxies are removed, then the
    * source becomes visible again. This has some advantages for some apps but
    * it's complete hell to handle in evas render side.
    * -- jpeg, 2016/03/07
    */

   EVAS_IMAGE_LEGACY_API(eo);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *src_obj;
   Evas_Image_Data *o;

   o = efl_data_scope_get(eo, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   if (!o->cur->source) return;

   visible = !!visible;
   src_obj = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);
   if (src_obj->proxy->src_invisible == !visible) return;

   evas_object_async_block(obj);
   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src_obj->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_invisible = !visible;
   EINA_COW_WRITE_END(evas_object_proxy_cow, src_obj->proxy, proxy_write);

   src_obj->changed_src_visible = EINA_TRUE;
   evas_object_smart_member_cache_invalidate(o->cur->source, EINA_FALSE,
                                             EINA_FALSE, EINA_TRUE);
   evas_object_change(o->cur->source, src_obj);
   if ((!visible) || (!src_obj->proxy->src_events)) return;
   //FIXME: Feed mouse events here.
}

/* deprecated */
EAPI Eina_Bool
evas_object_image_source_visible_get(const Evas_Object *eo)
{
   /* FIXME: see evas_object_image_source_visible_set */

   EVAS_IMAGE_LEGACY_API(eo, EINA_FALSE);

   Evas_Object_Protected_Data *src_obj;
   Evas_Image_Data *o;
   Eina_Bool visible;

   o = efl_data_scope_get(eo, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   if (!o->cur->source) visible = EINA_FALSE;
   src_obj = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);
   if (src_obj) visible = !src_obj->proxy->src_invisible;
   else visible = EINA_FALSE;

   return visible;
}

/* deprecated */
EAPI void*
evas_object_image_data_convert(Evas_Object *eo_obj, Evas_Colorspace to_cspace)
{
   EVAS_IMAGE_LEGACY_API(eo_obj, NULL);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o;
   void *engine_data;
   DATA32 *data;
   void* result = NULL;

   static int warned = 0;
   if (!warned)
     {
        ERR("%s is deprecated and shouldn't be called", __FUNCTION__);
        warned = 1;
     }

   evas_object_async_block(obj);
   o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   if (!o->engine_data) return NULL;
   if (o->video_surface)
     o->pixels->video.update_pixels(o->pixels->video.data, eo_obj, &o->pixels->video);
   if (o->cur->cspace == to_cspace) return NULL;
   if ((o->preload & EVAS_IMAGE_PRELOADING) && (o->engine_data))
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_TRUE);
     }
   data = NULL;
   engine_data = ENFN->image_data_get(ENC, o->engine_data, 0, &data, &o->load_error, NULL);
   result = _evas_image_data_convert_internal(o, data, to_cspace);
   if (engine_data)
     o->engine_data = ENFN->image_data_put(ENC, engine_data, data);

   return result;
}

/* deprecated */
EAPI void
evas_object_image_reload(Evas_Object *eo_obj)
{
   EVAS_IMAGE_LEGACY_API(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o;

   evas_object_async_block(obj);
   o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   if ((!o->cur->f) || (o->pixels_checked_out > 0)) return;
   if ((o->preload & EVAS_IMAGE_PRELOADING) && (o->engine_data))
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_TRUE);
     }
   if (o->engine_data)
     o->engine_data = ENFN->image_dirty_region(ENC, o->engine_data, 0, 0, o->cur->image.w, o->cur->image.h);

   eina_file_refresh(o->cur->f);
   o->written = EINA_FALSE;

   _evas_image_unload(eo_obj, obj, 1);
   evas_object_inform_call_image_unloaded(eo_obj);
   _evas_image_load(eo_obj, obj, o);

   EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, prev_write)
     {
        prev_write->f = NULL;
        prev_write->key = NULL;
     }
   EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, prev_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

/* deprecated */
EAPI Eina_Bool
evas_object_image_pixels_import(Evas_Object *eo_obj, Evas_Pixel_Import_Source *pixels)
{
   EVAS_IMAGE_LEGACY_API(eo_obj, EINA_FALSE);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o;

   static int warned = 0;
   if (!warned)
     {
        ERR("%s is deprecated and shouldn't be called", __FUNCTION__);
        warned = 1;
     }

   evas_object_async_block(obj);
   o = efl_data_scope_get(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   _evas_image_cleanup(eo_obj, obj, o);
   if ((pixels->w != o->cur->image.w) || (pixels->h != o->cur->image.h)) return EINA_FALSE;

   switch (pixels->format)
     {
#if 0
      case EVAS_PIXEL_FORMAT_ARGB32:
          {
             if (o->engine_data)
               {
                  DATA32 *image_pixels = NULL;

                  o->engine_data =
                    ENFN->image_data_get(ENC,
                                         o->engine_data,
                                         1,
                                         &image_pixels,
                                         &o->load_error);
/* FIXME: need to actualyl support this */
/*		  memcpy(image_pixels, pixels->rows, o->cur->image.w * o->cur->image.h * 4);*/
                  if (o->engine_data)
                    o->engine_data =
                    ENFN->image_data_put(ENC, o->engine_data, image_pixels);
                  if (o->engine_data)
                    o->engine_data =
                    ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
                  o->changed = EINA_TRUE;
                  evas_object_change(eo_obj, obj);
               }
          }
        break;
#endif
      case EVAS_PIXEL_FORMAT_YUV420P_601:
          {
             if (o->engine_data)
               {
                  DATA32 *image_pixels = NULL;

                  o->engine_data = ENFN->image_data_get(ENC, o->engine_data, 1, &image_pixels,&o->load_error, NULL);
                  if (image_pixels)
                    evas_common_convert_yuv_422p_601_rgba((DATA8 **) pixels->rows, (DATA8 *) image_pixels, o->cur->image.w, o->cur->image.h);
                  if (o->engine_data)
                    o->engine_data = ENFN->image_data_put(ENC, o->engine_data, image_pixels);
                  if (o->engine_data)
                    o->engine_data = ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
                  o->changed = EINA_TRUE;
                  evas_object_change(eo_obj, obj);
               }
          }
        break;
      default:
        return EINA_FALSE;
        break;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
evas_image_max_size_get(Eo *eo_e, int *w, int *h)
{
   Eina_Size2D size;
   Eina_Bool ret;

   ret = efl_canvas_scene_image_max_size_get(eo_e, &size);
   if (ret)
     {
        if (w) *w = size.w;
        if (h) *h = size.h;
     }
   return ret;
}

/* deprecated */
EAPI void
evas_object_image_alpha_mask_set(Evas_Object *eo_obj EINA_UNUSED, Eina_Bool ismask EINA_UNUSED)
{
   WRN("This function is not implemented, has never been and never will be.");
   EVAS_IMAGE_LEGACY_API(eo_obj);
}

EOLIAN static Eina_Bool
_evas_image_efl_file_mmap_set(Eo *obj, void *pd EINA_UNUSED, const Eina_File *f, const char *key)
{
   EVAS_IMAGE_API(obj, EINA_FALSE);
   return _evas_image_mmap_set(obj, f, key);
}

EOLIAN static void
_evas_image_efl_file_mmap_get(const Eo *obj, void *pd EINA_UNUSED, const Eina_File **f, const char **key)
{
   if (f) *f = NULL;
   if (key) *key = NULL;
   EVAS_IMAGE_API(obj);
   _evas_image_mmap_get(obj, f, key);
}

#include "canvas/evas_image.eo.c"
