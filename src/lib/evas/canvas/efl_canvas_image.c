#include "evas_image_private.h"

static void _image_preload_internal(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool cancel);

Eina_Bool
_evas_image_mmap_set(Eo *eo_obj, const Eina_File *f, const char *key)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   Evas_Image_Load_Opts lo;

   if (o->cur->u.f == f)
     {
        if ((!o->cur->key) && (!key))
          return EINA_FALSE;
        if ((o->cur->key) && (key) && (!strcmp(o->cur->key, key)))
          return EINA_FALSE;
     }
   evas_object_async_block(obj);
   _evas_image_init_set(f, NULL, key, eo_obj, obj, o, &lo);
   o->engine_data = ENFN->image_mmap(ENDT, o->cur->u.f, o->cur->key, &o->load_error, &lo);
   _evas_image_done_set(eo_obj, obj, o);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_file_mmap_set(Eo *eo_obj, void *_pd EINA_UNUSED,
                                    const Eina_File *f, const char *key)
{
   return _evas_image_mmap_set(eo_obj, f, key);
}

void
_evas_image_mmap_get(const Eo *eo_obj, const Eina_File **f, const char **key)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (f)
     *f = o->cur->mmaped_source ? o->cur->u.f : NULL;
   if (key)
     *key = o->cur->key;
}

EOLIAN static void
_efl_canvas_image_efl_file_mmap_get(Eo *eo_obj, void *_pd EINA_UNUSED,
                                    const Eina_File **f, const char **key)
{
   _evas_image_mmap_get(eo_obj, f, key);
}

Eina_Bool
_evas_image_file_set(Eo *eo_obj, const char *file, const char *key)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   Evas_Image_Load_Opts lo;

   if ((o->cur->u.file) && (file) && (!strcmp(o->cur->u.file, file)))
     {
        if ((!o->cur->key) && (!key))
          return EINA_FALSE;
        if ((o->cur->key) && (key) && (!strcmp(o->cur->key, key)))
          return EINA_FALSE;
     }

   evas_object_async_block(obj);
   _evas_image_init_set(NULL, file, key, eo_obj, obj, o, &lo);
   o->engine_data = ENFN->image_load(ENDT, o->cur->u.file, o->cur->key, &o->load_error, &lo);
   _evas_image_done_set(eo_obj, obj, o);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_file_file_set(Eo *eo_obj, void *_pd EINA_UNUSED,
                                    const char *file, const char *key)
{
   return _evas_image_file_set(eo_obj, file, key);
}

void
_evas_image_file_get(const Eo *eo_obj, const char **file, const char **key)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (file)
     {
        if (o->cur->mmaped_source)
          *file = eina_file_filename_get(o->cur->u.f);
        else
           *file = o->cur->u.file;
     }
   if (key) *key = o->cur->key;
}

EOLIAN static void
_efl_canvas_image_efl_file_file_get(Eo *eo_obj, void *_pd EINA_UNUSED,
                                    const char **file, const char **key)
{
   _evas_image_file_get(eo_obj, file, key);
}

Efl_Image_Load_Error
_evas_image_load_error_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   return (Efl_Image_Load_Error) o->load_error;
}

EOLIAN static Efl_Image_Load_Error
_efl_canvas_image_efl_image_load_load_error_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_load_error_get(eo_obj);
}

void
_evas_image_load_async_start(Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   evas_object_async_block(obj);
   _image_preload_internal(eo_obj, o, EINA_FALSE);
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_async_start(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   _evas_image_load_async_start(eo_obj);
}

void
_evas_image_load_async_cancel(Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   evas_object_async_block(obj);
   _image_preload_internal(eo_obj, o, EINA_TRUE);
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_async_cancel(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   _evas_image_load_async_cancel(eo_obj);
}

void
_evas_image_load_dpi_set(Eo *eo_obj, double dpi)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (dpi == o->load_opts->dpi) return;
   evas_object_async_block(obj);
   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     low->dpi = dpi;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        _evas_image_unload(eo_obj, obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        _evas_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
        evas_object_change(eo_obj, obj);
     }
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_dpi_set(Eo *eo_obj, void *_pd EINA_UNUSED, double dpi)
{
   _evas_image_load_dpi_set(eo_obj, dpi);
}

double
_evas_image_load_dpi_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   return o->load_opts->dpi;
}

EOLIAN static double
_efl_canvas_image_efl_image_load_load_dpi_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_load_dpi_get(eo_obj);
}

void
_evas_image_load_size_set(Eo *eo_obj, int w, int h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if ((o->load_opts->w == w) && (o->load_opts->h == h)) return;
   evas_object_async_block(obj);
   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
   {
      low->w = w;
      low->h = h;
   }
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        _evas_image_unload(eo_obj, obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        _evas_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
        evas_object_change(eo_obj, obj);
     }
   o->proxyerror = 0;
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_size_set(Eo *eo_obj, void *_pd EINA_UNUSED, int w, int h)
{
   _evas_image_load_size_set(eo_obj, w, h);
}

void
_evas_image_load_size_get(const Eo *eo_obj, int *w, int *h)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (w) *w = o->load_opts->w;
   if (h) *h = o->load_opts->h;
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_size_get(Eo *eo_obj, void *_pd EINA_UNUSED, int *w, int *h)
{
   _evas_image_load_size_get(eo_obj, w, h);
}

void
_evas_image_load_scale_down_set(Eo *eo_obj, int scale_down)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (o->load_opts->scale_down_by == scale_down) return;
   evas_object_async_block(obj);
   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     low->scale_down_by = scale_down;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        _evas_image_unload(eo_obj, obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        _evas_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
        evas_object_change(eo_obj, obj);
     }
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_scale_down_set(Eo *eo_obj, void *_pd EINA_UNUSED, int scale_down)
{
   _evas_image_load_scale_down_set(eo_obj, scale_down);
}

int
_evas_image_load_scale_down_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   return o->load_opts->scale_down_by;
}

EOLIAN static int
_efl_canvas_image_efl_image_load_load_scale_down_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_load_scale_down_get(eo_obj);
}

void
_evas_image_load_region_set(Eo *eo_obj, int x, int y, int w, int h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if ((o->load_opts->region.x == x) && (o->load_opts->region.y == y) &&
       (o->load_opts->region.w == w) && (o->load_opts->region.h == h)) return;
   evas_object_async_block(obj);
   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
   {
      low->region.x = x;
      low->region.y = y;
      low->region.w = w;
      low->region.h = h;
   }
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        _evas_image_unload(eo_obj, obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        _evas_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
        evas_object_change(eo_obj, obj);
     }
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_region_set(Eo *eo_obj, void *_pd EINA_UNUSED, int x, int y, int w, int h)
{
   _evas_image_load_region_set(eo_obj, x, y, w, h);
}

void
_evas_image_load_region_get(const Eo *eo_obj, int *x, int *y, int *w, int *h)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (x) *x = o->load_opts->region.x;
   if (y) *y = o->load_opts->region.y;
   if (w) *w = o->load_opts->region.w;
   if (h) *h = o->load_opts->region.h;
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_region_get(Eo *eo_obj, void *_pd EINA_UNUSED, int *x, int *y, int *w, int *h)
{
   _evas_image_load_region_get(eo_obj, x, y, w, h);
}

void
_evas_image_load_orientation_set(Eo *eo_obj, Eina_Bool enable)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (o->load_opts->orientation == !!enable) return;
   evas_object_async_block(obj);

   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
         low->orientation = !!enable;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);
}

EOLIAN static void
_efl_canvas_image_efl_image_load_load_orientation_set(Eo *eo_obj, void *_pd EINA_UNUSED, Eina_Bool enable)
{
   _evas_image_load_orientation_set(eo_obj, enable);
}

Eina_Bool
_evas_image_load_orientation_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   return o->load_opts->orientation;;
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_image_load_load_orientation_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_load_orientation_get(eo_obj);
}

Eina_Bool
_evas_image_load_region_support_get(const Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   return ENFN->image_can_region_get(ENDT, o->engine_data);
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_image_load_load_region_support_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_load_region_support_get(eo_obj);
}

/* animated feature */
Eina_Bool
_evas_image_animated_get(const Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (!ENFN->image_animated_get)
     return EINA_FALSE;

   return ENFN->image_animated_get(ENDT, o->engine_data);
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_image_animated_animated_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_animated_get(eo_obj);
}

int
_evas_image_animated_frame_count_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   Evas_Object_Protected_Data *obj;
   int frame_count = -1;

   if (!evas_object_image_animated_get(eo_obj)) return frame_count;

   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (ENFN->image_animated_frame_count_get)
     frame_count = ENFN->image_animated_frame_count_get(ENDT, o->engine_data);

   return frame_count;
}

EOLIAN static int
_efl_canvas_image_efl_image_animated_animated_frame_count_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_animated_frame_count_get(eo_obj);
}

Efl_Image_Animated_Loop_Hint
_evas_image_animated_loop_type_get(const Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Efl_Image_Animated_Loop_Hint hint = EFL_IMAGE_ANIMATED_LOOP_HINT_NONE;
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (!evas_object_image_animated_get(eo_obj)) return hint;

   if (ENFN->image_animated_loop_type_get)
     hint = (Efl_Image_Animated_Loop_Hint) ENFN->image_animated_loop_type_get(ENDT, o->engine_data);

   return hint;
}

EOLIAN static Efl_Image_Animated_Loop_Hint
_efl_canvas_image_efl_image_animated_animated_loop_type_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_animated_loop_type_get(eo_obj);
}

int
_evas_image_animated_loop_count_get(const Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   int loop_count;
   loop_count = -1;
   if (!evas_object_image_animated_get(eo_obj)) return loop_count;

   loop_count =
         ENFN->image_animated_loop_count_get ?
            ENFN->image_animated_loop_count_get(ENDT, o->engine_data) :
            -1;

   return loop_count;
}

EOLIAN static int
_efl_canvas_image_efl_image_animated_animated_loop_count_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_animated_loop_count_get(eo_obj);
}

double
_evas_image_animated_frame_duration_get(const Eo *eo_obj, int start_frame, int frame_num)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   double frame_duration = -1;
   int frame_count = 0;

   if (!ENFN->image_animated_frame_count_get) return frame_duration;

   frame_count = ENFN->image_animated_frame_count_get(ENDT, o->engine_data);

   if ((start_frame + frame_num) > frame_count) return frame_duration;
   if (ENFN->image_animated_frame_duration_get)
     frame_duration = ENFN->image_animated_frame_duration_get(ENDT, o->engine_data, start_frame, frame_num);

   return frame_duration;
}

EOLIAN static double
_efl_canvas_image_efl_image_animated_animated_frame_duration_get(Eo *eo_obj, void *_pd EINA_UNUSED, int start_frame, int frame_num)
{
   return _evas_image_animated_frame_duration_get(eo_obj, start_frame, frame_num);
}

Eina_Bool
_evas_image_animated_frame_set(Eo *eo_obj, int frame_index)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   int frame_count = 0;

   if (!o->cur->u.file) return EINA_FALSE;
   if (o->cur->frame == frame_index) return EINA_TRUE;

   if (!evas_object_image_animated_get(eo_obj)) return EINA_FALSE;
   evas_object_async_block(obj);
   frame_count = evas_object_image_animated_frame_count_get(eo_obj);

   /* limit the size of frame to FRAME_MAX */
   if ((frame_count > FRAME_MAX) || (frame_count < 0) || (frame_index > frame_count))
     return EINA_FALSE;

   if (!ENFN->image_animated_frame_set) return EINA_FALSE;
   ENFN->image_animated_frame_set(ENDT, o->engine_data, frame_index);
   //   if (!ENFN->image_animated_frame_set(ENDT, o->engine_data, frame_index)) return;

   EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, prev_write)
     prev_write->frame = o->cur->frame;
   EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, prev_write);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
         state_write->frame = frame_index;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_canvas_image_efl_image_animated_animated_frame_set(Eo *eo_obj, void *_pd EINA_UNUSED, int frame_index)
{
   return _evas_image_animated_frame_set(eo_obj, frame_index);
}

int
_evas_image_animated_frame_get(const Eo *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (!o->cur->u.file) return EINA_FALSE;
   if (!evas_object_image_animated_get(eo_obj)) return EINA_FALSE;
   return o->cur->frame;
}

EOLIAN static int
_efl_canvas_image_efl_image_animated_animated_frame_get(Eo *eo_obj, void *_pd EINA_UNUSED)
{
   return _evas_image_animated_frame_get(eo_obj);
}

static void
_image_preload_internal(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool cancel)
{
   if (!o->engine_data)
     {
        o->preloading = EINA_TRUE;
        evas_object_inform_call_image_preloaded(eo_obj);
        return;
     }
   // FIXME: if already busy preloading, then dont request again until
   // preload done
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (cancel)
     {
        if (o->preloading)
          {
             o->preloading = EINA_FALSE;
             ENFN->image_data_preload_cancel(ENDT, o->engine_data, eo_obj);
          }
     }
   else
     {
        if (!o->preloading)
          {
             o->preloading = EINA_TRUE;
             ENFN->image_data_preload_request(ENDT, o->engine_data, eo_obj);
          }
     }
}

#include "efl_canvas_image.eo.c"
