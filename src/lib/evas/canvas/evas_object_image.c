
#include "evas_image_private.h"

#define MY_CLASS      EFL_CANVAS_IMAGE_INTERNAL_CLASS
#define MY_CLASS_NAME "Evas_Image"

/* private magic number for image objects */
static const char o_type[] = "image";

const char *o_image_type = o_type;

/* private methods for image objects */
static Evas_Coord   evas_object_image_figure_x_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord   evas_object_image_figure_y_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void         evas_object_image_init(Evas_Object *eo_obj);
static void         evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj,
                                             void *type_private_data,
                                             void *engine, void *output, void *context, void *surface,
                                             int x, int y, Eina_Bool do_async);
static void         _evas_image_render(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                       void *engine, void *output, void *context, void *surface,
                                       int x, int y, int l, int t, int r, int b, Eina_Bool skip_map, Eina_Bool do_async);
static void         evas_object_image_free(Evas_Object *eo_obj,
                                           Evas_Object_Protected_Data *obj);
static void         evas_object_image_render_pre(Evas_Object *eo_obj,
                                                 Evas_Object_Protected_Data *obj,
                                                 void *type_private_data);
static void         evas_object_image_render_post(Evas_Object *eo_obj,
                                                  Evas_Object_Protected_Data *obj,
                                                  void *type_private_data);

static void        *evas_object_image_engine_data_get(Evas_Object *eo_obj);

static int          evas_object_image_is_opaque(Evas_Object *eo_obj,
                                                Evas_Object_Protected_Data *obj,
                                                void *type_private_data);
static int          evas_object_image_was_opaque(Evas_Object *eo_obj,
                                                 Evas_Object_Protected_Data *obj,
                                                 void *type_private_data);
static int          evas_object_image_is_inside(Evas_Object *eo_obj,
                                                Evas_Object_Protected_Data *obj,
                                                void *type_private_data,
                                                Evas_Coord x, Evas_Coord y);
static int          evas_object_image_has_opaque_rect(Evas_Object *eo_obj,
                                                      Evas_Object_Protected_Data *obj,
                                                      void *type_private_data);
static int          evas_object_image_get_opaque_rect(Evas_Object *eo_obj,
                                                      Evas_Object_Protected_Data *obj,
                                                      void *type_private_data,
                                                      Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
static int          evas_object_image_can_map(Evas_Object *eo_obj);
static void         evas_object_image_render_prepare(Evas_Object *obj, Evas_Object_Protected_Data *pd, Eina_Bool do_async);

static void         evas_object_image_filled_resize_listener(void *data, Evas *eo_e, Evas_Object *eo_obj, void *einfo);

static int          evas_object_image_is_on_plane(Evas_Object *obj EINA_UNUSED, Evas_Object_Protected_Data *pd EINA_UNUSED, void *type_private_data);
static int          evas_object_image_plane_changed(Evas_Object *obj EINA_UNUSED, Evas_Object_Protected_Data *pd EINA_UNUSED, void *type_private_data);
static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   evas_object_image_render,
   evas_object_image_render_pre,
   evas_object_image_render_post,
   evas_object_image_engine_data_get,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   evas_object_image_is_opaque,
   evas_object_image_was_opaque,
   evas_object_image_is_inside,
   NULL,
   NULL,
   evas_object_image_has_opaque_rect,
   evas_object_image_get_opaque_rect,
   evas_object_image_can_map,
   evas_object_image_render_prepare,   // render_prepare
   evas_object_image_is_on_plane,
   evas_object_image_plane_changed
};

static const Evas_Object_Image_Load_Opts default_load_opts = {
   0, 0.0, 0, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0
};

static const Evas_Object_Image_Pixels default_pixels = {
   NULL, { NULL, NULL }, NULL, { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }, ~0x0
};

static const Evas_Object_Image_State default_state = {
   { 0, 0, 0, 0 }, // fill
   { 0, 0, 0 }, // image
   { 1.0, 0, 0, 0, 0, 1 }, // border
   NULL, NULL, NULL, //source, defmap, scene
   NULL, //f
   NULL, //key
   0, //frame
   EVAS_COLORSPACE_ARGB8888,
   EVAS_IMAGE_ORIENT_NONE,

   EINA_TRUE, // smooth
   EINA_FALSE, // has_alpha
   EINA_FALSE, // opaque_valid
   EINA_FALSE // opaque
};

Eina_Cow *evas_object_image_load_opts_cow = NULL;
Eina_Cow *evas_object_image_pixels_cow = NULL;
Eina_Cow *evas_object_image_state_cow = NULL;

static void
evas_object_image_render_prepare(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, Eina_Bool do_async EINA_UNUSED)
{
#if 0
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   // if image data not loaded or in texture then upload
   if ((o->cur->f) || (o->written) || (o->cur->frame != 0))
     {
        if (o->engine_data) ENFN->image_prepare(ENC, o->engine_data);
     }
#endif
   // XXX: if image is a proxy, PREPEND to prerender list in evas canvas
}

static void *
_evas_object_image_output_find(Evas_Object_Protected_Data *obj)
{
   Efl_Canvas_Output *output;
   Eina_List *l;
   const Eina_Rectangle geometry = {
     obj->cur->geometry.x,
     obj->cur->geometry.y,
     obj->cur->geometry.w,
     obj->cur->geometry.h
   };

   EINA_LIST_FOREACH(obj->layer->evas->outputs, l, output)
     {
        if (eina_rectangles_intersect(&output->geometry, &geometry))
          return output->output;
     }

   // Always return an output, as evas rely on that even if the object is out of screen.
   output = eina_list_data_get(obj->layer->evas->outputs);
   return output->output;
}

void
_evas_image_cleanup(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   /* Eina_Cow doesn't know if the resulting memory has changed, better check
      before we change it */
   if (o->cur->opaque_valid)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->opaque_valid = 0;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   if ((o->preload & EVAS_IMAGE_PRELOADING) && (o->engine_data))
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_FALSE);
     }
   if (o->cur->source) _evas_image_proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _evas_image_3d_unset(eo_obj, obj, o);
}

static Eina_Bool
_init_cow(void)
{
   if (!evas_object_image_load_opts_cow ||
       !evas_object_image_pixels_cow ||
       !evas_object_image_state_cow)
     {
        evas_object_image_load_opts_cow = eina_cow_add("Evas_Object_Image load opts",
                                                       sizeof (Evas_Object_Image_Load_Opts),
                                                       8,
                                                       &default_load_opts,
                                                       EINA_TRUE);
        evas_object_image_pixels_cow = eina_cow_add("Evas_Object_Image pixels",
                                                    sizeof (Evas_Object_Image_Pixels),
                                                    8,
                                                    &default_pixels,
                                                    EINA_TRUE);
        evas_object_image_state_cow = eina_cow_add("Evas_Object_Image states",
                                                   sizeof (Evas_Object_Image_State),
                                                   8,
                                                   &default_state,
                                                   EINA_TRUE);
     }
   if (!evas_object_image_load_opts_cow ||
       !evas_object_image_pixels_cow ||
       !evas_object_image_state_cow)
     {
        ERR("Failed to init cow.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eo *
_efl_canvas_image_internal_efl_object_constructor(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Colorspace cspace;

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   evas_object_image_init(eo_obj);

   if (!_init_cow())
     return NULL;

   o->load_opts = eina_cow_alloc(evas_object_image_load_opts_cow);
   o->pixels = eina_cow_alloc(evas_object_image_pixels_cow);
   o->cur = eina_cow_alloc(evas_object_image_state_cow);
   o->prev = eina_cow_alloc(evas_object_image_state_cow);
   o->proxy_src_clip = EINA_TRUE;

   cspace = ENFN->image_colorspace_get(ENC, o->engine_data);
   if (cspace != o->cur->cspace)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        state_write->cspace = cspace;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   return eo_obj;
}

EOLIAN static Eo *
_efl_canvas_image_internal_efl_object_finalize(Eo *eo_obj, Evas_Image_Data *o)
{
   if (!o->filled_set)
     efl_gfx_fill_auto_set(eo_obj, EINA_TRUE);
   return efl_finalize(efl_super(eo_obj, MY_CLASS));
}

void
_evas_image_init_set(const Eina_File *f, const char *key,
                     Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o,
                     Evas_Image_Load_Opts *lo)
{
   if (o->cur->source) _evas_image_proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _evas_image_3d_unset(eo_obj, obj, o);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   {
      Eina_File *tmp =  state_write->f;

      state_write->f = NULL;

      if (f) state_write->f = eina_file_dup(f);
      eina_file_close(tmp);

      eina_stringshare_replace(&state_write->key, key);
      state_write->opaque_valid = 0;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->prev->f != NULL || o->prev->key != NULL)
     {
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->f = NULL;
           state_write->key = NULL;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);
     }

   if (o->engine_data)
     {
        if (o->preload & EVAS_IMAGE_PRELOADING)
          {
             o->preload = EVAS_IMAGE_PRELOAD_NONE;
             ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_FALSE);
          }
        ENFN->image_free(ENC, o->engine_data);
     }
   o->load_error = EFL_GFX_IMAGE_LOAD_ERROR_NONE;
   lo->emile.scale_down_by = o->load_opts->scale_down_by;
   lo->emile.dpi = o->load_opts->dpi;
   lo->emile.w = o->load_opts->w;
   lo->emile.h = o->load_opts->h;
   lo->emile.region.x = o->load_opts->region.x;
   lo->emile.region.y = o->load_opts->region.y;
   lo->emile.region.w = o->load_opts->region.w;
   lo->emile.region.h = o->load_opts->region.h;
   lo->emile.scale_load.src_x = o->load_opts->scale_load.src_x;
   lo->emile.scale_load.src_y = o->load_opts->scale_load.src_y;
   lo->emile.scale_load.src_w = o->load_opts->scale_load.src_w;
   lo->emile.scale_load.src_h = o->load_opts->scale_load.src_h;
   lo->emile.scale_load.dst_w = o->load_opts->scale_load.dst_w;
   lo->emile.scale_load.dst_h = o->load_opts->scale_load.dst_h;
   lo->emile.scale_load.smooth = o->load_opts->scale_load.smooth;
   lo->emile.scale_load.scale_hint = o->load_opts->scale_load.scale_hint;
   lo->emile.orientation = o->load_opts->orientation;
   lo->emile.degree = 0;
   lo->skip_head = o->skip_head;
}

void
_evas_image_done_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   Eina_Bool resize_call = EINA_FALSE;

   if (o->engine_data)
     {
        int w, h;
        int stride;
        Evas_Image_Orient orient;

        ENFN->image_size_get(ENC, o->engine_data, &w, &h);
        if (ENFN->image_stride_get)
          ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
          stride = w * 4;
        orient = ENFN->image_orient_get(ENC, o->engine_data);

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->has_alpha = ENFN->image_alpha_get(ENC, o->engine_data);
           state_write->cspace = ENFN->image_colorspace_get(ENC, o->engine_data);

           if ((o->cur->image.w != w) || (o->cur->image.h != h))
             resize_call = EINA_TRUE;

           state_write->image.w = w;
           state_write->image.h = h;
           state_write->image.stride = stride;
           state_write->orient = orient;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
   else
     {
        if (o->load_error == EFL_GFX_IMAGE_LOAD_ERROR_NONE)
          o->load_error = EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->has_alpha = EINA_TRUE;
           state_write->cspace = EVAS_COLORSPACE_ARGB8888;

           if ((state_write->image.w != 0) || (state_write->image.h != 0))
             resize_call = EINA_TRUE;

           state_write->image.w = 0;
           state_write->image.h = 0;
           state_write->image.stride = 0;
           state_write->orient = EVAS_IMAGE_ORIENT_NONE;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
   o->file_size.w = 0;
   o->file_size.h = 0;
   o->written = EINA_FALSE;
   o->changed = EINA_TRUE;
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

void
_evas_image_orientation_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Image_Orient orient)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int iw, ih;

   if (o->cur->orient == orient) return;

   if ((o->preload & EVAS_IMAGE_PRELOADING) && (o->engine_data))
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_TRUE);
     }

   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data = ENFN->image_orient_set(ENC, o->engine_data, orient);
        if (o->engine_data)
          {
             ENFN->image_size_get(ENC, o->engine_data, &iw, &ih);

             if (ENFN->image_stride_get)
               ENFN->image_stride_get(ENC, o->engine_data, &stride);
             else
               stride = iw * 4;

             EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
             {
                state_write->image.w = iw;
                state_write->image.h = ih;
                state_write->orient = orient;
                state_write->image.stride = stride;
             }
             EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
             o->written = EINA_TRUE;
          }
     }
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

static Evas_Image_Orient
_get_image_orient_from_orient_flip(Efl_Orient orient, Efl_Flip flip)
{
   switch (orient)
     {
      case EFL_ORIENT_0:
        if (flip == EFL_FLIP_HORIZONTAL)
          return EVAS_IMAGE_FLIP_HORIZONTAL;
        else if (flip == EFL_FLIP_VERTICAL)
          return EVAS_IMAGE_FLIP_VERTICAL;
        else
          return EVAS_IMAGE_ORIENT_0;

      case EFL_ORIENT_90:
        if (flip == EFL_FLIP_HORIZONTAL)
          return EVAS_IMAGE_FLIP_TRANSPOSE;
        else if (flip == EFL_FLIP_VERTICAL)
          return EVAS_IMAGE_FLIP_TRANSVERSE;
        else
          return EVAS_IMAGE_ORIENT_90;

      case EFL_ORIENT_180:
        if (flip == EFL_FLIP_HORIZONTAL)
          return EVAS_IMAGE_FLIP_VERTICAL;
        else if (flip == EFL_FLIP_VERTICAL)
          return EVAS_IMAGE_FLIP_HORIZONTAL;
        else
          return EVAS_IMAGE_ORIENT_180;

      case EFL_ORIENT_270:
        if (flip == EFL_FLIP_HORIZONTAL)
          return EVAS_IMAGE_FLIP_TRANSVERSE;
        else if (flip == EFL_FLIP_VERTICAL)
          return EVAS_IMAGE_FLIP_TRANSPOSE;
        else
          return EVAS_IMAGE_ORIENT_270;

      default:
        return EVAS_IMAGE_ORIENT_NONE;
     }
}

EOLIAN static void
_efl_canvas_image_internal_efl_orientation_orientation_set(Eo *obj, Evas_Image_Data *o, Efl_Orient dir)
{
   Evas_Image_Orient orient;

   o->orient_value = dir;
   orient = _get_image_orient_from_orient_flip(dir, o->flip_value);

   _evas_image_orientation_set(obj, o, orient);
}

EOLIAN static Efl_Orient
_efl_canvas_image_internal_efl_orientation_orientation_get(const Eo *obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->orient_value;
}

EOLIAN static void
_efl_canvas_image_internal_efl_orientation_flip_set(Eo *obj, Evas_Image_Data *o, Efl_Flip flip)
{
   Evas_Image_Orient orient;

   o->flip_value = flip;
   orient = _get_image_orient_from_orient_flip(o->orient_value, flip);

   _evas_image_orientation_set(obj, o, orient);
}

EOLIAN static Efl_Flip
_efl_canvas_image_internal_efl_orientation_flip_get(const Eo *obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->flip_value;
}

EOLIAN static void
_efl_canvas_image_internal_efl_object_dbg_info_get(Eo *eo_obj, Evas_Image_Data *o, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *file, *key;
   file = eina_file_filename_get(o->cur->f);
   key = o->cur->key;

   EFL_DBG_INFO_APPEND(group, "Image File", EINA_VALUE_TYPE_STRING, file);
   EFL_DBG_INFO_APPEND(group, "Key", EINA_VALUE_TYPE_STRING, key);
   EFL_DBG_INFO_APPEND(group, "Source", EINA_VALUE_TYPE_UINT64,
                       (uint64_t)(uintptr_t)evas_object_image_source_get(eo_obj));
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_border_set(Eo *eo_obj, Evas_Image_Data *o, int l, int r, int t, int b)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur->border.l == l) &&
       (o->cur->border.r == r) &&
       (o->cur->border.t == t) &&
       (o->cur->border.b == b)) return;

   evas_object_async_block(obj);
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   {
      state_write->border.l = l;
      state_write->border.r = r;
      state_write->border.t = t;
      state_write->border.b = b;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_border_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, int *l, int *r, int *t, int *b)
{
   if (l) *l = o->cur->border.l;
   if (r) *r = o->cur->border.r;
   if (t) *t = o->cur->border.t;
   if (b) *b = o->cur->border.b;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_border_center_fill_set(Eo *eo_obj, Evas_Image_Data *o, Efl_Gfx_Border_Fill_Mode _fill)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Border_Fill_Mode fill = (Evas_Border_Fill_Mode)_fill;

   if (fill == o->cur->border.fill) return;
   evas_object_async_block(obj);
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   state_write->border.fill = fill;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static Efl_Gfx_Border_Fill_Mode
_efl_canvas_image_internal_efl_gfx_image_border_center_fill_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (Efl_Gfx_Border_Fill_Mode)o->cur->border.fill;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_fill_fill_auto_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool setting)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   setting = !!setting;
   o->filled_set = 1;
   if (o->filled == setting) return;

   evas_object_async_block(obj);
   o->filled = setting;
   if (!o->filled)
     evas_object_event_callback_del(eo_obj, EVAS_CALLBACK_RESIZE,
                                    evas_object_image_filled_resize_listener);
   else
     {
        Eina_Size2D sz;

        sz = efl_gfx_entity_size_get(eo_obj);
        _evas_image_fill_set(eo_obj, o, 0, 0, sz.w, sz.h);

        evas_object_event_callback_add(eo_obj, EVAS_CALLBACK_RESIZE,
                                       evas_object_image_filled_resize_listener,
                                       NULL);
     }
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_gfx_fill_fill_auto_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->filled;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_border_scale_set(Eo *eo_obj, Evas_Image_Data *o, double scale)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (EINA_DBL_EQ(scale, o->cur->border.scale)) return;
   evas_object_async_block(obj);
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   state_write->border.scale = scale;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static double
_efl_canvas_image_internal_efl_gfx_image_border_scale_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->border.scale;
}

void
_evas_image_fill_set(Eo *eo_obj, Evas_Image_Data *o, int x, int y, int w, int h)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (w == 0) return;
   if (h == 0) return;
   if (w < 0) w = -w;
   if (h < 0) h = -h;

   if ((o->cur->fill.x == x) &&
       (o->cur->fill.y == y) &&
       (o->cur->fill.w == w) &&
       (o->cur->fill.h == h)) return;

   evas_object_async_block(obj);
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   {
      state_write->fill.x = x;
      state_write->fill.y = y;
      state_write->fill.w = w;
      state_write->fill.h = h;
      state_write->opaque_valid = 0;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_fill_fill_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Rect fill)
{
   // Should (0,0,0,0) reset the filled flag to true?
   o->filled = EINA_FALSE;
   o->filled_set = EINA_TRUE;
   _evas_image_fill_set(eo_obj, o, fill.x, fill.y, fill.w, fill.h);
}

EOLIAN static Eina_Rect
_efl_canvas_image_internal_efl_gfx_fill_fill_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (Eina_Rect) o->cur->fill;
}

EOLIAN static Eina_Size2D
_efl_canvas_image_internal_efl_gfx_image_image_size_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return EINA_SIZE2D(o->file_size.w, o->file_size.h);
}

EOLIAN static Eina_Size2D
_efl_canvas_image_internal_efl_gfx_view_view_size_get(const Eo *eo_obj, Evas_Image_Data *o)
{
   int uvw, uvh;
   Evas_Object_Protected_Data *source = NULL;
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (o->cur->source)
     source = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);

   if (o->cur->scene)
     {
        uvw = obj->data_3d->w;
        uvh = obj->data_3d->h;
     }
   else if (!o->cur->source)
     {
        uvw = o->cur->image.w;
        uvh = o->cur->image.h;
     }
   else if (source->proxy->surface && !source->proxy->redraw)
     {
        uvw = source->proxy->w;
        uvh = source->proxy->h;
     }
   else if (source->type == o_type &&
            ((Evas_Image_Data *)efl_data_scope_get(o->cur->source, MY_CLASS))->engine_data)
     {
        uvw = source->cur->geometry.w;
        uvh = source->cur->geometry.h;
     }
   else
     {
        uvw = source->proxy->w;
        uvh = source->proxy->h;
     }

   return EINA_SIZE2D(uvw, uvh);
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_buffer_buffer_update_add(Eo *eo_obj, Evas_Image_Data *o, const Eina_Rect *region)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Eina_Rectangle *r;
   Eo *eo_obj2;
   Eina_List *l;
   int x, y, w, h;
   int cnt;

   if (region)
     {
        x = region->x;
        y = region->y;
        w = region->w;
        h = region->h;
     }
   else
     {
        x = y = 0;
        w = o->cur->image.w;
        h = o->cur->image.h;
     }

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur->image.w, o->cur->image.h);
   if ((w <= 0) || (h <= 0)) return;
   if (obj->cur->snapshot)
     {
        obj->snapshot_no_obscure = EINA_TRUE;
        evas_object_change(eo_obj, obj);
        return;
     }
   if (!o->written) return;
   evas_object_async_block(obj);
   cnt = eina_list_count(o->pixels->pixel_updates);
   if (cnt == 1) // detect single blob case already there to do a nop
     {
        if ((r = o->pixels->pixel_updates->data)) // already a single full rect there.
          {
             if ((r->x == 0) && (r->y == 0) && (r->w == o->cur->image.w) && (r->h == o->cur->image.h))
               return;
          }
     }
   if ((cnt >= 512) ||
       (((x == 0) && (y == 0) && (w == o->cur->image.w) && (h == o->cur->image.h)))) // too many update rects - just make a single blob update
     {
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        {
           EINA_LIST_FREE(pixi_write->pixel_updates, r)
             eina_rectangle_free(r);
           NEW_RECT(r, 0, 0, o->cur->image.w, o->cur->image.h);
           if (r) pixi_write->pixel_updates = eina_list_append(pixi_write->pixel_updates, r);
        }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }
   else
     {
        NEW_RECT(r, x, y, w, h);
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        if (r) pixi_write->pixel_updates = eina_list_append(pixi_write->pixel_updates, r);
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);

   EINA_LIST_FOREACH(obj->proxy->proxies, l, eo_obj2)
     {
        Evas_Object_Protected_Data *obj2 = efl_data_scope_get(eo_obj2, EFL_CANVAS_OBJECT_CLASS);
        evas_object_change(eo_obj2, obj2);
     }
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_buffer_alpha_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool has_alpha)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);
   if ((o->preload & EVAS_IMAGE_PRELOADING) && (o->engine_data))
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_TRUE);
     }

   has_alpha = !!has_alpha;
   if (has_alpha == o->cur->has_alpha)
     return;

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   {
      state_write->has_alpha = has_alpha;
      state_write->opaque_valid = 0;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data = ENFN->image_alpha_set(ENC, o->engine_data, o->cur->has_alpha);
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
   efl_gfx_buffer_update_add(eo_obj, NULL);
   EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(o);
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_gfx_buffer_alpha_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->has_alpha;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_smooth_scale_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool smooth_scale)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_async_block(obj);
   if (((smooth_scale) && (o->cur->smooth_scale)) ||
       ((!smooth_scale) && (!o->cur->smooth_scale)))
     return;
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   state_write->smooth_scale = smooth_scale;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_gfx_image_smooth_scale_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->smooth_scale;
}

EOLIAN static double
_efl_canvas_image_internal_efl_gfx_image_ratio_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   if (!o->cur->image.h) return 1.0;
   return (double)o->cur->image.w / (double)o->cur->image.h;
}

EOLIAN static Eina_Error
_efl_canvas_image_internal_efl_gfx_image_image_load_error_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->load_error;
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_file_save_save(const Eo *eo_obj, Evas_Image_Data *o, const char *file, const char *key, const Efl_File_Save_Info *info)
{
   int quality = 80, compress = 9, ok = 0;
   const char *encoding = NULL;
   Image_Entry *ie;
   Evas_Colorspace cspace = EVAS_COLORSPACE_ARGB8888;
   Evas_Colorspace want_cspace = EVAS_COLORSPACE_ARGB8888;
   Evas_Object_Protected_Data *obj;
   Eina_Bool unmap_it = EINA_FALSE;
   int imagew, imageh, uvw, uvh;
   Eina_Rw_Slice slice = {};
   DATA32 *data = NULL;
   void *pixels;
   void *output;

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   evas_object_async_block(obj);

   output = _evas_object_image_output_find(obj);
   pixels = _evas_image_pixels_get((Eo *)eo_obj, obj, ENC, output, NULL, NULL,
                                   0, 0,
                                   &imagew, &imageh, &uvw, &uvh, EINA_TRUE, EINA_TRUE);
   if (!pixels) goto no_pixels;

   cspace = ENFN->image_file_colorspace_get(ENC, pixels);
   want_cspace = cspace;
   if (info)
     {
        encoding = info->encoding;
        quality = info->quality;
        compress = info->compression;
     }

   if (encoding)
     {
        const char *ext = strrchr(file, '.');
        if (ext && !strcasecmp(ext, ".tgv"))
          {
             if (!strcmp(encoding, "auto"))
               want_cspace = cspace;
             else if (!strcmp(encoding, "etc1"))
               want_cspace = EVAS_COLORSPACE_ETC1;
             else if (!strcmp(encoding, "etc2"))
               {
                  if (!ENFN->image_alpha_get(ENC, pixels))
                    want_cspace = EVAS_COLORSPACE_RGB8_ETC2;
                  else
                    want_cspace = EVAS_COLORSPACE_RGBA8_ETC2_EAC;
               }
             else if (!strcmp(encoding, "etc1+alpha"))
               want_cspace = EVAS_COLORSPACE_ETC1_ALPHA;
          }
     }

   if (ENFN->image_data_direct_get && (o->cur->orient == EVAS_IMAGE_ORIENT_NONE))
     {
        Evas_Colorspace cs;
        Eina_Slice sl;

        ok = ENFN->image_data_direct_get(ENC, pixels, 0, &sl, &cs, EINA_TRUE);
        if (ok && (cs == want_cspace))
          data = (DATA32 *)sl.mem;
     }

   if (!data)
     {
        int stride;

        cspace = EVAS_COLORSPACE_ARGB8888;
        ok = ENFN->image_data_map(ENC, &pixels, &slice, &stride, 0, 0, imagew, imageh,
                                  cspace, EFL_GFX_BUFFER_ACCESS_MODE_READ, 0);
        if (!ok || !slice.mem) goto no_pixels;
        unmap_it = EINA_TRUE;
        data = slice.mem;

        if (stride != (imagew * 4))
          WRN("Invalid stride: saved image may look wrong!");
     }

   ie = evas_cache_image_data(evas_common_image_cache_get(),
                              imagew, imageh, data, o->cur->has_alpha, cspace);
   if (ie)
     {
        RGBA_Image *im = (RGBA_Image *)ie;

        ok = evas_common_save_image_to_file(im, file, key, quality, compress, encoding);
        evas_cache_image_drop(ie);
     }
   else ok = EINA_FALSE;

   if (unmap_it)
     ENFN->image_data_unmap(ENC, pixels, &slice);

   if (!ok) ERR("Image save failed.");
   return ok;

no_pixels:
   ERR("Could not get image pixels for saving.");
   return EINA_FALSE;
}

EOLIAN static Efl_Gfx_Colorspace
_efl_canvas_image_internal_efl_gfx_buffer_colorspace_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (Efl_Gfx_Colorspace)o->cur->cspace;
}

static void
_on_image_native_surface_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *einfo EINA_UNUSED)
{
   evas_object_image_native_surface_set(obj, NULL);
}

Eina_Bool
_evas_image_native_surface_set(Eo *eo_obj, Evas_Native_Surface *surf)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   evas_object_event_callback_del_full
     (eo_obj, EVAS_CALLBACK_DEL, _on_image_native_surface_del, NULL);
   if (surf) // We need to unset native surf on del to remove shared hash refs
     evas_object_event_callback_add
       (eo_obj, EVAS_CALLBACK_DEL, _on_image_native_surface_del, NULL);

   o->can_scanout = EINA_FALSE;

   evas_render_rendering_wait(obj->layer->evas);
   _evas_image_cleanup(eo_obj, obj, o);
   if (!ENFN->image_native_set) return EINA_FALSE;
   if ((surf) &&
       ((surf->version < 2) ||
        (surf->version > EVAS_NATIVE_SURFACE_VERSION))) return EINA_FALSE;
   o->engine_data = ENFN->image_native_set(ENC, o->engine_data, surf);

   if (surf && surf->version > 4)
     {
        switch (surf->type)
          {
           case EVAS_NATIVE_SURFACE_WL_DMABUF:
             if (surf->data.wl_dmabuf.scanout.handler)
               o->can_scanout = EINA_TRUE;
             break;

           default:
             break;
          }
     }
   return o->engine_data != NULL;
}

Evas_Native_Surface *
_evas_image_native_surface_get(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Evas_Native_Surface *surf = NULL;

   if (ENFN->image_native_get)
     surf = ENFN->image_native_get(ENC, o->engine_data);

   return surf;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_scale_hint_set(Eo *eo_obj, Evas_Image_Data *o, Efl_Gfx_Image_Scale_Hint hint)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   if (o->scale_hint == hint) return;
   o->scale_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;

        if (ENFN->image_scale_hint_set)
          ENFN->image_scale_hint_set(ENC, o->engine_data, o->scale_hint);
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
}

EOLIAN static Efl_Gfx_Image_Scale_Hint
_efl_canvas_image_internal_efl_gfx_image_scale_hint_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->scale_hint;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_image_content_hint_set(Eo *eo_obj, Evas_Image_Data *o, Efl_Gfx_Image_Content_Hint hint)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   if (o->content_hint == hint) return;
   o->content_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;

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
}

EOLIAN static Efl_Gfx_Image_Content_Hint
_efl_canvas_image_internal_efl_gfx_image_content_hint_get(const Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->content_hint;
}

void
_evas_image_unload(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool dirty)
{
   Evas_Image_Data *o;
   Eina_Bool resize_call = EINA_FALSE;

   o = efl_data_scope_get(eo_obj, MY_CLASS);
   if ((!o->cur->f) ||
       (o->pixels_checked_out > 0)) return;

   evas_object_async_block(obj);
   if (dirty)
     {
        if (o->engine_data)
          o->engine_data = ENFN->image_dirty_region(ENC, o->engine_data,
                                                    0, 0,
                                                    o->cur->image.w, o->cur->image.h);
     }
   if (o->engine_data)
     {
        if (o->preload & EVAS_IMAGE_PRELOADING)
          {
             o->preload = EVAS_IMAGE_PRELOAD_NONE;
             ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_FALSE);
          }
        ENFN->image_free(ENC, o->engine_data);
     }
   o->engine_data = NULL;
   o->load_error = EFL_GFX_IMAGE_LOAD_ERROR_NONE;

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
   {
      state_write->has_alpha = EINA_TRUE;
      state_write->cspace = EVAS_COLORSPACE_ARGB8888;
      if ((state_write->image.w != 0) || (state_write->image.h != 0)) resize_call = EINA_TRUE;
      state_write->image.w = 0;
      state_write->image.h = 0;
      state_write->image.stride = 0;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
}

void
_evas_image_load(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   Evas_Image_Load_Opts lo;
   int load_error = 0;

   if (o->engine_data) return;

   lo.emile.scale_down_by = o->load_opts->scale_down_by;
   lo.emile.dpi = o->load_opts->dpi;
   lo.emile.w = o->load_opts->w;
   lo.emile.h = o->load_opts->h;
   lo.emile.region.x = o->load_opts->region.x;
   lo.emile.region.y = o->load_opts->region.y;
   lo.emile.region.w = o->load_opts->region.w;
   lo.emile.region.h = o->load_opts->region.h;
   lo.emile.scale_load.src_x = o->load_opts->scale_load.src_x;
   lo.emile.scale_load.src_y = o->load_opts->scale_load.src_y;
   lo.emile.scale_load.src_w = o->load_opts->scale_load.src_w;
   lo.emile.scale_load.src_h = o->load_opts->scale_load.src_h;
   lo.emile.scale_load.dst_w = o->load_opts->scale_load.dst_w;
   lo.emile.scale_load.dst_h = o->load_opts->scale_load.dst_h;
   lo.emile.scale_load.smooth = o->load_opts->scale_load.smooth;
   lo.emile.scale_load.scale_hint = o->load_opts->scale_load.scale_hint;
   lo.emile.orientation = o->load_opts->orientation;
   lo.emile.degree = 0;
   lo.skip_head = o->skip_head;
   o->engine_data = ENFN->image_mmap(ENC, o->cur->f, o->cur->key, &load_error, &lo);
   o->load_error = _evas_load_error_to_efl_gfx_image_load_error(load_error);

   if (o->engine_data)
     {
        int w, h;
        int stride = 0;
        Eina_Bool resize_call = EINA_FALSE;

        ENFN->image_size_get(ENC, o->engine_data, &w, &h);
        if (ENFN->image_stride_get)
          ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
          stride = w * 4;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->has_alpha = ENFN->image_alpha_get(ENC, o->engine_data);
           state_write->cspace = ENFN->image_colorspace_get(ENC, o->engine_data);
           if ((state_write->image.w != w) || (state_write->image.h != h))
             resize_call = EINA_TRUE;
           state_write->image.w = w;
           state_write->image.h = h;
           state_write->image.stride = stride;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
        if (resize_call) evas_object_inform_call_image_resize(eo_obj);
     }
   else
     {
        o->load_error = EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;
     }
}

void
_evas_image_load_post_update(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   if (o->engine_data)
     {
        int w, h;
        int stride = 0;
        Eina_Bool resize_call = EINA_FALSE;

        ENFN->image_size_get(ENC, o->engine_data, &w, &h);
        if (ENFN->image_stride_get)
          ENFN->image_stride_get(ENC, o->engine_data, &stride);
        else
          stride = w * 4;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->has_alpha = ENFN->image_alpha_get(ENC, o->engine_data);
           state_write->cspace = ENFN->image_colorspace_get(ENC, o->engine_data);
           if ((state_write->image.w != w) || (state_write->image.h != h))
             resize_call = EINA_TRUE;
           state_write->image.w = w;
           state_write->image.h = h;
           state_write->image.stride = stride;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
        o->changed = EINA_TRUE;
        o->preload = EVAS_IMAGE_PRELOADED;
        if (resize_call) evas_object_inform_call_image_resize(eo_obj);
        evas_object_change(eo_obj, obj);

        //preloading error check
        if (ENFN->image_load_error_get)
          o->load_error = _evas_load_error_to_efl_gfx_image_load_error(ENFN->image_load_error_get(ENC, o->engine_data));
     }
   else
     {
        o->preload = EVAS_IMAGE_PRELOAD_NONE;
        o->load_error = EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;
     }
}

static Evas_Coord
evas_object_image_figure_x_fill(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord w;

   w = ((size * obj->layer->evas->output.w) /
        (Evas_Coord)obj->layer->evas->viewport.w);
   if (size <= 0) size = 1;
   if (start > 0)
     {
        while (start - size > 0)
          start -= size;
     }
   else if (start < 0)
     {
        while (start < 0)
          start += size;
     }
   start = ((start * obj->layer->evas->output.w) /
            (Evas_Coord)obj->layer->evas->viewport.w);
   *size_ret = w;
   return start;
}

static Evas_Coord
evas_object_image_figure_y_fill(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
{
   Evas_Coord h;

   h = ((size * obj->layer->evas->output.h) /
        (Evas_Coord)obj->layer->evas->viewport.h);
   if (size <= 0) size = 1;
   if (start > 0)
     {
        while (start - size > 0)
          start -= size;
     }
   else if (start < 0)
     {
        while (start < 0)
          start += size;
     }
   start = ((start * obj->layer->evas->output.h) /
            (Evas_Coord)obj->layer->evas->viewport.h);
   *size_ret = h;
   return start;
}

static void
evas_object_image_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;
   obj->is_image_object = EINA_TRUE;
}

EOLIAN static void
_efl_canvas_image_internal_efl_object_destructor(Eo *eo_obj, Evas_Image_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (obj->legacy.ctor)
     evas_object_image_video_surface_set(eo_obj, NULL);
   evas_object_image_free(eo_obj, obj);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

void
_evas_object_image_free(Evas_Object *obj)
{
   Evas_Image_Data *o;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, MY_CLASS));

   o = efl_data_scope_get(obj, MY_CLASS);

   // eina_cow_free reset the pointer to the default read only state
   eina_cow_free(evas_object_image_load_opts_cow, (const Eina_Cow_Data **)&o->load_opts);
   eina_cow_free(evas_object_image_pixels_cow, (const Eina_Cow_Data **)&o->pixels);
   eina_cow_free(evas_object_image_state_cow, (const Eina_Cow_Data **)&o->cur);
   eina_cow_free(evas_object_image_state_cow, (const Eina_Cow_Data **)&o->prev);
}

static void
evas_object_image_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;

   /* free obj */
   eina_file_close(o->cur->f);
   if (o->cur->key) eina_stringshare_del(o->cur->key);
   if (o->cur->source) _evas_image_proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _evas_image_3d_unset(eo_obj, obj, o);
   if (obj->layer && obj->layer->evas)
     {
        if (o->engine_data && ENC)
          {
             if (o->preload & EVAS_IMAGE_PRELOADING)
               {
                  o->preload = EVAS_IMAGE_PRELOAD_NONE;
                  ENFN->image_data_preload_cancel(ENC, o->engine_data, eo_obj, EINA_FALSE);
               }
             ENFN->image_free(ENC, o->engine_data);
          }
        if (o->engine_data_prep && ENC)
          {
             ENFN->image_free(ENC, o->engine_data_prep);
          }
        if (o->video_surface)
          {
             o->video_surface = EINA_FALSE;
             obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, eo_obj);
          }
     }
   o->engine_data = NULL;
   o->engine_data_prep = NULL;
   if (o->pixels->images_to_free)
     {
        eina_hash_free(o->pixels->images_to_free);
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        pixi_write->images_to_free = NULL;
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }
   if (o->pixels->pixel_updates)
     {
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        {
           EINA_LIST_FREE(pixi_write->pixel_updates, r)
             eina_rectangle_free(r);
        }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }
}

static void
_draw_image(Evas_Object_Protected_Data *obj,
            void *engine, void *data, void *context, void *surface, void *image,
            int src_x, int src_y, int src_w, int src_h, int dst_x,
            int dst_y, int dst_w, int dst_h, int smooth,
            Eina_Bool do_async)
{
   Eina_Bool async_unref;

   async_unref = ENFN->image_draw(engine, data, context, surface,
                                  image, src_x, src_y,
                                  src_w, src_h, dst_x,
                                  dst_y, dst_w, dst_h,
                                  smooth, do_async);
   if (do_async && async_unref)
     {
        evas_cache_image_ref((Image_Entry *)image);

        evas_unref_queue_image_put(obj->layer->evas, image);
     }
}

void
evas_draw_image_map_async_check(Evas_Object_Protected_Data *obj,
                                void *engine, void *data, void *context, void *surface,
                                void *image, RGBA_Map *m, int smooth, int level,
                                Eina_Bool do_async)
{
   Eina_Bool async_unref;
   obj->layer->evas->engine.func->context_anti_alias_set(engine, context,
                                                         obj->cur->anti_alias);
   async_unref = ENFN->image_map_draw(engine, data, context,
                                      surface, image, m,
                                      smooth, level,
                                      do_async);
   if (do_async && async_unref)
     {
        evas_cache_image_ref((Image_Entry *)image);

        evas_unref_queue_image_put(obj->layer->evas, image);
     }
}

void
evas_object_pixels_get_force(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Image_Data *o = obj->private_data;

   o->pixels->func.get_pixels(o->pixels->func.get_pixels_data, eo_obj);
}

static void *
evas_process_dirty_pixels(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o,
                          void *engine, void *output, void *surface, void *pixels)
{
   Eina_Bool direct_override = EINA_FALSE, direct_force_off = EINA_FALSE;

   if (o->dirty_pixels)
     {
        if (o->pixels->func.get_pixels)
          {
             Evas_Coord x, y, w, h;

             if (ENFN->image_native_get)
               {
                  Evas_Native_Surface *ns;

                  ns = ENFN->image_native_get(engine, o->engine_data);
                  if (ns)
                    {
                       Eina_Bool direct_renderable = EINA_FALSE;

                       // Check if we can do direct rendering...
                       if (ENFN->gl_direct_override_get)
                         ENFN->gl_direct_override_get(engine, &direct_override, &direct_force_off);
                       if (ENFN->gl_surface_direct_renderable_get)
                         direct_renderable = ENFN->gl_surface_direct_renderable_get(engine, output, ns, &direct_override, surface);

                       if (((direct_override) ||
                            ((direct_renderable) &&
                             (obj->cur->geometry.w == o->cur->image.w) &&
                             (obj->cur->geometry.h == o->cur->image.h) &&
                             (obj->cur->color.r == 255) &&
                             (obj->cur->color.g == 255) &&
                             (obj->cur->color.b == 255) &&
                             (obj->cur->color.a == 255) &&
                             (obj->cur->cache.clip.r == 255) &&
                             (obj->cur->cache.clip.g == 255) &&
                             (obj->cur->cache.clip.b == 255) &&
                             (obj->cur->cache.clip.a == 255) &&
                             (!obj->map->cur.map))
                            ) && (!direct_force_off))
                         {
                            if (ENFN->gl_get_pixels_set)
                              ENFN->gl_get_pixels_set(engine, o->pixels->func.get_pixels, o->pixels->func.get_pixels_data, eo_obj);
                            if (ENFN->gl_image_direct_set)
                              ENFN->gl_image_direct_set(engine, o->engine_data, EINA_TRUE);
                            o->direct_render = EINA_TRUE;
                         }
                       else
                         o->direct_render = EINA_FALSE;
                    }

                  if ((ns) &&
                      (ns->type == EVAS_NATIVE_SURFACE_X11))
                    {
                       if (ENFN->context_flush)
                         ENFN->context_flush(engine);
                    }
               }

             x = obj->cur->geometry.x;
             y = obj->cur->geometry.y;
             w = obj->cur->geometry.w;
             h = obj->cur->geometry.h;

             if (!o->direct_render)
               {
                  if (ENFN->gl_get_pixels_pre)
                    ENFN->gl_get_pixels_pre(engine, output);
                  o->pixels->func.get_pixels(o->pixels->func.get_pixels_data, eo_obj);
                  if (ENFN->gl_get_pixels_post)
                    ENFN->gl_get_pixels_post(engine, output);
               }

             if (!(obj->cur->geometry.x == x &&
                   obj->cur->geometry.y == y &&
                   obj->cur->geometry.w == w &&
                   obj->cur->geometry.h == h))
               CRI("Evas_Image_Data geometry did change during pixels get callback !");

             o->engine_data = ENFN->image_dirty_region
                 (engine, o->engine_data,
                 0, 0, o->cur->image.w, o->cur->image.h);
             if (o->engine_data != pixels)
               pixels = o->engine_data;
          }
        o->dirty_pixels = EINA_FALSE;
     }
   else
     {
        // Check if the it's not dirty but it has direct rendering
        if (o->direct_render && ENFN->image_native_get)
          {
             Evas_Native_Surface *ns;
             ns = ENFN->image_native_get(engine, o->engine_data);

             if (ENFN->gl_direct_override_get)
               ENFN->gl_direct_override_get(engine, &direct_override, &direct_force_off);
             if (ENFN->gl_surface_direct_renderable_get)
               ENFN->gl_surface_direct_renderable_get(engine, output, ns, &direct_override, surface);

             if (direct_override && !direct_force_off)
               {
                  // always use direct rendering
                  if (ENFN->gl_get_pixels_set)
                    ENFN->gl_get_pixels_set(engine, o->pixels->func.get_pixels, o->pixels->func.get_pixels_data, eo_obj);
                  if (ENFN->gl_image_direct_set)
                    ENFN->gl_image_direct_set(engine, o->engine_data, EINA_TRUE);
               }
             else
               {
                  // Auto-fallback to FBO rendering (for perf & power consumption)
                  if (ENFN->gl_get_pixels_pre)
                    ENFN->gl_get_pixels_pre(engine, output);
                  o->pixels->func.get_pixels(o->pixels->func.get_pixels_data, obj->object);
                  if (ENFN->gl_get_pixels_post)
                    ENFN->gl_get_pixels_post(engine, output);
                  o->direct_render = EINA_FALSE;
               }
          }
     }

   return pixels;
}

EOLIAN static void
_efl_canvas_image_internal_efl_canvas_filter_internal_filter_dirty(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_canvas_filter_internal_filter_input_alpha(
  Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_canvas_image_internal_efl_gfx_filter_filter_program_set(Eo *obj, Evas_Image_Data *pd,
                                                             const char *code, const char *name)
{
   pd->has_filter = (code != NULL);
   efl_gfx_filter_program_set(efl_super(obj, MY_CLASS), code, name);
}

EOLIAN static void
_efl_canvas_image_internal_efl_canvas_filter_internal_filter_state_prepare(
  Eo *eo_obj, Evas_Image_Data *o EINA_UNUSED, Efl_Canvas_Filter_State *state, void *data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   memset(&state->text, 0, sizeof(state->text));
#define STATE_COLOR(dst, src) dst.r = src.r; dst.g = src.g; dst.b = src.b; dst.a = src.a
   STATE_COLOR(state->color, obj->cur->color);
#undef STATE_COLOR

   state->w = obj->cur->geometry.w;
   state->h = obj->cur->geometry.h;
   state->scale = obj->cur->scale;
}

static inline Eina_Bool
_image_has_border(Evas_Object_Protected_Data *obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->border.l || o->cur->border.r || o->cur->border.t ||
          o->cur->border.b || (o->cur->border.fill == 0);
}

static inline Eina_Bool
_image_is_filled(Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   if (o->filled) return EINA_TRUE;
   return !o->cur->fill.x && !o->cur->fill.y &&
          (o->cur->fill.w == obj->cur->geometry.w) &&
          (o->cur->fill.h == obj->cur->geometry.h);
}

static inline Eina_Bool
_image_is_scaled(Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   return (obj->cur->geometry.w != o->cur->image.w) ||
          (obj->cur->geometry.h != o->cur->image.h);
}

EOLIAN static Eina_Bool
_efl_canvas_image_internal_efl_canvas_filter_internal_filter_input_render(
  Eo *eo_obj, Evas_Image_Data *o,
  void *_filter, void *engine, void *output, void *context, void *data EINA_UNUSED,
  int l, int r EINA_UNUSED, int t, int b EINA_UNUSED,
  int x, int y, Eina_Bool do_async)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Filter_Context *filter = _filter;
   void *surface, *ctx;
   Eina_Bool ok;
   int W, H;

   W = obj->cur->geometry.w;
   H = obj->cur->geometry.h;

   // FIXME: In GL we could use the image even if scaled
   if (!_image_has_border(obj, o) && _image_is_filled(obj, o) && !_image_is_scaled(obj, o))
     {
        int imagew, imageh, uvw, uvh;

        surface = _evas_image_pixels_get(eo_obj, obj, engine, output, context, NULL, x, y,
                                         &imagew, &imageh, &uvw, &uvh, EINA_FALSE, EINA_FALSE);

        ok = evas_filter_buffer_backing_set(filter, EVAS_FILTER_BUFFER_INPUT_ID, surface);
        if (ok) return EINA_TRUE;
     }

   surface = evas_filter_buffer_backing_get(filter, EVAS_FILTER_BUFFER_INPUT_ID, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, EINA_FALSE);

   if (!o->filled)
     {
        l = 0;
        t = 0;
        r = 0;
        b = 0;
     }

   ctx = ENFN->context_new(engine);

   if (o->cur->has_alpha && !obj->cur->snapshot)
     {
        ENFN->context_color_set(engine, ctx, 0, 0, 0, 0);
        ENFN->context_render_op_set(engine, ctx, EVAS_RENDER_COPY);
        ENFN->rectangle_draw(engine, output, ctx, surface, 0, 0, W, H, do_async);
        ENFN->context_color_set(engine, ctx, 255, 255, 255, 255);
        ENFN->context_render_op_set(engine, ctx, EVAS_RENDER_BLEND);
     }

   _evas_image_render(eo_obj, obj,
                      engine, output, ctx, surface,
                      x + l - obj->cur->geometry.x,
                      y + t - obj->cur->geometry.y,
                      l, t, r, b, EINA_TRUE, do_async);

   ENFN->context_free(engine, ctx);
   ENFN->image_free(engine, surface);

   return EINA_TRUE;
}

void
_evas_object_image_plane_release(Evas_Object *eo_obj EINA_UNUSED,
                                 Evas_Object_Protected_Data *obj,
                                 Efl_Canvas_Output *output)
{
   Evas_Image_Data *o;

   if (!obj) return;

   o = obj->private_data;

   if (!o->engine_data) return;

   if (!o->plane) return;

   ENFN->image_plane_release(output->output, o->engine_data, o->plane);
   output->planes = eina_list_remove(output->planes, obj);
   o->plane = NULL;
}

Eina_Bool
_evas_object_image_can_use_plane(Evas_Object_Protected_Data *obj,
                                 Efl_Canvas_Output *output)
{
   Evas_Native_Surface *ns;
   Evas_Image_Data *o = obj->private_data;

   /* Let the video surface code handle this one... */
   if (o->video_surface)
     return EINA_TRUE;

   if (!o->can_scanout)
     return EINA_FALSE;

   if (!o->engine_data)
     return EINA_FALSE;

   if (!ENFN->image_plane_assign)
     return EINA_FALSE;

   if (!ENFN->image_native_get)
     return EINA_FALSE;

   ns = ENFN->image_native_get(ENC, o->engine_data);
   if (!ns) return EINA_FALSE;

   // FIXME: adjust position with output offset.
   o->plane = ENFN->image_plane_assign(output->output, o->engine_data,
                                       obj->cur->geometry.x,
                                       obj->cur->geometry.y);

   if (!o->plane)
     return EINA_FALSE;

   output->planes = eina_list_append(output->planes, obj);
   return EINA_TRUE;
}

static int
evas_object_image_is_on_plane(Evas_Object *obj EINA_UNUSED, Evas_Object_Protected_Data *pd EINA_UNUSED, void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;

   if (o->plane) return 1;
   return 0;
}

static int
evas_object_image_plane_changed(Evas_Object *obj EINA_UNUSED, Evas_Object_Protected_Data *pd EINA_UNUSED, void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;

   if (!!o->plane != o->plane_status)
     {
        o->plane_status = !!o->plane;
        return 1;
     }

   return 0;
}

static void
evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *type_private_data,
                         void *engine, void *output, void *context, void *surface, int x, int y, Eina_Bool do_async)
{
   Evas_Image_Data *o = type_private_data;

   /* image is not ready yet, skip rendering. Leave it to next frame */
   if (o->preload == EVAS_IMAGE_PRELOADING) return;

   if ((o->cur->fill.w < 1) || (o->cur->fill.h < 1))
     return;  /* no error message, already printed in pre_render */

   /* Proxy sanity */
   if (o->proxyrendering)
     {
        _evas_image_proxy_error(eo_obj, engine, output, context, surface, x, y, EINA_FALSE);
        return;
     }

   /* Mask sanity */
   if (obj->mask->is_mask && (surface != obj->mask->surface))
     {
        ERR("Drawing a mask to another surface? Something's wrong...");
        return;
     }

   if (o->engine_data_prep)
     {
        ENFN->context_multiplier_unset(engine, context);
        ENFN->context_render_op_set(engine, context, obj->cur->render_op);
        ENFN->image_draw(engine, output, context, surface, o->engine_data_prep,
                         0, 0, obj->cur->geometry.w, obj->cur->geometry.h,
                         obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                         obj->cur->geometry.w, obj->cur->geometry.h,
                         0, do_async);
        return;
     }

   if (o->plane)
     {
        /* We must call pixels get because enlightenment uses it for internal
         * bookkeeping and won't send frame callbacks to wayland clients if we
         * don't
         */
        evas_object_pixels_get_force(eo_obj, obj);
#if 0
        Evas_Native_Surface *ns;

        /* Draw a bright red rectangle where the object replaced by
         * a hardware plane would have been.
         */
        ns = ENFN->image_native_get(ENC, o->engine_data);
        if (ns && ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF)
          {
             ENFN->context_color_set(output, context, 255, 0, 0, 255);
             ENFN->context_multiplier_unset(output, context);
             ENFN->context_render_op_set(output, context, EVAS_RENDER_COPY);
             ENFN->rectangle_draw(engine, output, context, surface,
                                  obj->cur->geometry.x + x,
                                  obj->cur->geometry.y + y,
                                  obj->cur->geometry.w,
                                  obj->cur->geometry.h,
                                  do_async);
          }
#endif
        return;
     }
   /* We are displaying the overlay */
   if (o->video_visible)
     {
        /* Create a transparent rectangle */
        ENFN->context_color_set(engine, context, 0, 0, 0, 0);
        ENFN->context_multiplier_unset(engine, context);
        ENFN->context_render_op_set(engine, context, EVAS_RENDER_COPY);
        ENFN->rectangle_draw(engine, output, context, surface,
                             obj->cur->geometry.x + x, obj->cur->geometry.y + y,
                             obj->cur->geometry.w, obj->cur->geometry.h,
                             do_async);

        return;
     }

   ENFN->context_color_set(engine, context, 255, 255, 255, 255);

   if ((obj->cur->cache.clip.r == 255) &&
       (obj->cur->cache.clip.g == 255) &&
       (obj->cur->cache.clip.b == 255) &&
       (obj->cur->cache.clip.a == 255))
     {
        ENFN->context_multiplier_unset(engine, context);
     }
   else
     ENFN->context_multiplier_set(engine, context,
                                  obj->cur->cache.clip.r,
                                  obj->cur->cache.clip.g,
                                  obj->cur->cache.clip.b,
                                  obj->cur->cache.clip.a);

   ENFN->context_render_op_set(engine, context, obj->cur->render_op);

   // Clear out the pixel get stuff..
   if (ENFN->gl_get_pixels_set)
     ENFN->gl_get_pixels_set(engine, NULL, NULL, NULL);
   if (ENFN->gl_image_direct_set)
     ENFN->gl_image_direct_set(engine, o->engine_data, EINA_FALSE);

   /* Render filter */
   if (o->has_filter)
     {
        if (evas_filter_object_render(eo_obj, obj, engine, output, context, surface, x, y, do_async, EINA_FALSE))
          return;
     }

   _evas_image_render(eo_obj, obj, engine, output, context, surface, x, y, 0, 0, 0, 0, EINA_FALSE, do_async);
}

void *
_evas_image_pixels_get(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                       void *engine, void *output, void *context, void *surface,
                       int x, int y,
                       int *imagew, int *imageh, int *uvw, int *uvh,
                       Eina_Bool filtered, Eina_Bool needs_post_render)
{
   Evas_Image_Data *o = obj->private_data, *oi = NULL;
   Evas_Object_Protected_Data *source = NULL;
   void *pixels = NULL;

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj, NULL);

   if (filtered && o->has_filter)
     pixels = evas_filter_output_buffer_get(eo_obj);

   if (!pixels && o->cur->source)
     {
        source = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);
        if (source && (source->type == o_type))
          oi = efl_data_scope_get(o->cur->source, MY_CLASS);
     }

   if (pixels)
     {
        ENFN->image_size_get(engine, pixels, imagew, imageh);
        *uvw = *imagew;
        *uvh = *imageh;
     }
   else if (o->cur->scene)
     {
        _evas_image_3d_render(obj->layer->evas->evas, eo_obj, obj, o, o->cur->scene, engine, output);
        pixels = obj->data_3d->surface;
        *imagew = obj->data_3d->w;
        *imageh = obj->data_3d->h;
        *uvw = *imagew;
        *uvh = *imageh;
     }
   else if (obj->cur->snapshot)
     {
        pixels = o->engine_data;
        *imagew = o->cur->image.w;
        *imageh = o->cur->image.h;
        *uvw = *imagew;
        *uvh = *imageh;
     }
   else if (!o->cur->source || !source)
     {
        // normal image (from file or user pixel set)
        needs_post_render = EINA_FALSE;
        if (output && surface)
          pixels = evas_process_dirty_pixels(eo_obj, obj, o, engine, output, surface, o->engine_data);
        else
          pixels = o->engine_data;
        *imagew = o->cur->image.w;
        *imageh = o->cur->image.h;
        *uvw = *imagew;
        *uvh = *imageh;
     }
   else if (source->proxy->surface && !source->proxy->redraw)
     {
        pixels = source->proxy->surface;
        *imagew = source->proxy->w;
        *imageh = source->proxy->h;
        *uvw = *imagew;
        *uvh = *imageh;
     }
   else if (oi && oi->engine_data)
     {
        if (oi->has_filter)
          pixels = evas_filter_output_buffer_get(source->object);
        if (!pixels)
          pixels = oi->engine_data;
        *imagew = oi->cur->image.w;
        *imageh = oi->cur->image.h;
        *uvw = source->cur->geometry.w;
        *uvh = source->cur->geometry.h;
        /* check source_clip since we skip proxy_subrender here */
        if (context && o->proxy_src_clip && source->cur->clipper)
          {
             ENFN->context_clip_clip(engine, context,
                                     source->cur->clipper->cur->cache.clip.x + x,
                                     source->cur->clipper->cur->cache.clip.y + y,
                                     source->cur->clipper->cur->cache.clip.w,
                                     source->cur->clipper->cur->cache.clip.h);
          }
     }
   else
     {
        o->proxyrendering = EINA_TRUE;
        evas_render_proxy_subrender(obj->layer->evas->evas, output, o->cur->source,
                                    eo_obj, obj, o->proxy_src_clip, EINA_FALSE);
        pixels = source->proxy->surface;
        *imagew = source->proxy->w;
        *imageh = source->proxy->h;
        *uvw = *imagew;
        *uvh = *imageh;
        o->proxyrendering = EINA_FALSE;
     }

   if (needs_post_render && !obj->layer->evas->inside_post_render)
     {
        ERR("Can not save or map this image now! Proxies, snapshots and "
            "filtered images support those operations only from inside a "
            "post-render event.");
        return NULL;
     }

   return pixels;
}

static void
_evas_image_render(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                   void *engine, void *output, void *context, void *surface, int x, int y,
                   int l, int t, int r, int b, Eina_Bool skip_map, Eina_Bool do_async)
{
   Evas_Image_Data *o = obj->private_data;
   int imagew, imageh, uvw, uvh, cw, ch;
   int ix, iy, iw, ih, offx, offy;
   int idw, idh, idx, idy;
   void *pixels;

   pixels = _evas_image_pixels_get(eo_obj, obj, engine, output, context, surface, x, y,
                                   &imagew, &imageh, &uvw, &uvh, EINA_FALSE, EINA_FALSE);

   if (!pixels) return;
   if (ENFN->context_clip_get(engine, context, NULL, NULL, &cw, &ch) && (!cw || !ch))
     return;

   if (!skip_map && (obj->map->cur.map) && (obj->map->cur.map->count > 3)
       && (obj->map->cur.usemap))
     {
        evas_object_map_update(eo_obj, x, y, imagew, imageh, uvw, uvh);

        evas_draw_image_map_async_check(
          obj, engine, output, context, surface, pixels, obj->map->spans,
          o->cur->smooth_scale | obj->map->cur.map->smooth, 0, do_async);

        return;
     }

   ENFN->image_scale_hint_set(engine, pixels, o->scale_hint);
   idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur->fill.x, o->cur->fill.w, &idw);
   idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur->fill.y, o->cur->fill.h, &idh);
   if (idw < 1) idw = 1;
   if (idh < 1) idh = 1;
   if (idx > 0) idx -= idw;
   if (idy > 0) idy -= idh;

   offx = obj->cur->geometry.x + x;
   offy = obj->cur->geometry.y + y;

   while (idx < obj->cur->geometry.w)
     {
        int ydy, dobreak_w = 0;

        ydy = idy;
        ix = idx;
        if ((o->cur->fill.w == obj->cur->geometry.w) &&
            (o->cur->fill.x == 0))
          {
             dobreak_w = 1;
             iw = obj->cur->geometry.w;
          }
        else
          iw = idx + idw - ix;

        // Filter stuff
        if (o->filled)
          {
             iw -= l + r;
             if (iw <= 0) break;
          }

        while (idy < obj->cur->geometry.h)
          {
             int dobreak_h = 0;

             iy = idy;
             if ((o->cur->fill.h == obj->cur->geometry.h) &&
                 (o->cur->fill.y == 0))
               {
                  ih = obj->cur->geometry.h;
                  dobreak_h = 1;
               }
             else
               ih = idy + idh - iy;

             // Filter stuff
             if (o->filled)
               {
                  ih -= t + b;
                  if (ih <= 0) break;
               }

             if ((o->cur->border.l == 0) && (o->cur->border.r == 0) &&
                 (o->cur->border.t == 0) && (o->cur->border.b == 0) &&
                 (o->cur->border.fill != 0))
               {
                  _draw_image(obj, engine, output, context, surface, pixels,
                              0, 0, imagew, imageh,
                              offx + ix, offy + iy, iw, ih,
                              o->cur->smooth_scale, do_async);
               }
             else
               {
                  int inx, iny, inw, inh, outx, outy, outw, outh;
                  int bl, br, bt, bb, bsl, bsr, bst, bsb;
                  int imw, imh, ox, oy;

                  ox = offx + ix;
                  oy = offy + iy;
                  imw = imagew;
                  imh = imageh;
                  bl = o->cur->border.l;
                  br = o->cur->border.r;
                  bt = o->cur->border.t;
                  bb = o->cur->border.b;
                  // fix impossible border settings if img pixels not enough
                  if ((bl + br) > 0)
                    {
                       if ((bl + br) > imw)
                         {
                            bl = (bl * imw) / (bl + br);
                            br = imw - bl;
                         }
                       if ((bl + br) == imw)
                         {
                            if (bl < br) br--;
                            else bl--;
                         }
                    }
                  if ((bt + bb) > 0)
                    {
                       if ((bt + bb) > imh)
                         {
                            bt = (bt * imh) / (bt + bb);
                            bb = imh - bt;
                         }
                       if ((bt + bb) == imh)
                         {
                            if (bt < bb) bb--;
                            else bt--;
                         }
                    }
                  if (!EINA_DBL_EQ(o->cur->border.scale, 1.0))
                    {
                       bsl = ((double)bl * o->cur->border.scale);
                       bsr = ((double)br * o->cur->border.scale);
                       bst = ((double)bt * o->cur->border.scale);
                       bsb = ((double)bb * o->cur->border.scale);
                    }
                  else
                    {
                       bsl = bl; bsr = br; bst = bt; bsb = bb;
                    }
                  // adjust output border rendering if it doesnt fit
                  if ((bsl + bsr) > iw)
                    {
                       int b0 = bsl, b1 = bsr;

                       if ((bsl + bsr) > 0)
                         {
                            bsl = (bsl * iw) / (bsl + bsr);
                            bsr = iw - bsl;
                         }
                       if (b0 > 0) bl = (bl * bsl) / b0;
                       else bl = 0;
                       if (b1 > 0) br = (br * bsr) / b1;
                       else br = 0;
                    }
                  if ((bst + bsb) > ih)
                    {
                       int b0 = bst, b1 = bsb;

                       if ((bst + bsb) > 0)
                         {
                            bst = (bst * ih) / (bst + bsb);
                            bsb = ih - bst;
                         }
                       if (b0 > 0) bt = (bt * bst) / b0;
                       else bt = 0;
                       if (b1 > 0) bb = (bb * bsb) / b1;
                       else bb = 0;
                    }
                  // #--.
                  // |  |
                  // '--'
                  inx = 0; iny = 0;
                  inw = bl; inh = bt;
                  outx = ox; outy = oy;
                  outw = bsl; outh = bst;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .##.
                  // |  |
                  // '--'
                  inx = bl; iny = 0;
                  inw = imw - bl - br; inh = bt;
                  outx = ox + bsl; outy = oy;
                  outw = iw - bsl - bsr; outh = bst;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--#
                  // |  |
                  // '--'
                  inx = imw - br; iny = 0;
                  inw = br; inh = bt;
                  outx = ox + iw - bsr; outy = oy;
                  outw = bsr; outh = bst;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--.
                  // #  |
                  // '--'
                  inx = 0; iny = bt;
                  inw = bl; inh = imh - bt - bb;
                  outx = ox; outy = oy + bst;
                  outw = bsl; outh = ih - bst - bsb;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--.
                  // |##|
                  // '--'
                  if (o->cur->border.fill > EVAS_BORDER_FILL_NONE)
                    {
                       inx = bl; iny = bt;
                       inw = imw - bl - br; inh = imh - bt - bb;
                       outx = ox + bsl; outy = oy + bst;
                       outw = iw - bsl - bsr; outh = ih - bst - bsb;
                       if ((o->cur->border.fill == EVAS_BORDER_FILL_SOLID) &&
                           (obj->cur->cache.clip.a == 255) &&
                           (!obj->clip.mask) &&
                           (obj->cur->render_op == EVAS_RENDER_BLEND))
                         {
                            ENFN->context_render_op_set(engine, context, EVAS_RENDER_COPY);
                            _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            ENFN->context_render_op_set(engine, context, obj->cur->render_op);
                         }
                       else
                         _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                    }
                  // .--.
                  // |  #
                  // '--'
                  inx = imw - br; iny = bt;
                  inw = br; inh = imh - bt - bb;
                  outx = ox + iw - bsr; outy = oy + bst;
                  outw = bsr; outh = ih - bst - bsb;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--.
                  // |  |
                  // #--'
                  inx = 0; iny = imh - bb;
                  inw = bl; inh = bb;
                  outx = ox; outy = oy + ih - bsb;
                  outw = bsl; outh = bsb;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--.
                  // |  |
                  // '##'
                  inx = bl; iny = imh - bb;
                  inw = imw - bl - br; inh = bb;
                  outx = ox + bsl; outy = oy + ih - bsb;
                  outw = iw - bsl - bsr; outh = bsb;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                  // .--.
                  // |  |
                  // '--#
                  inx = imw - br; iny = imh - bb;
                  inw = br; inh = bb;
                  outx = ox + iw - bsr; outy = oy + ih - bsb;
                  outw = bsr; outh = bsb;
                  _draw_image(obj, engine, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
               }
             idy += idh;
             if (dobreak_h) break;
          }
        idx += idw;
        idy = ydy;
        if (dobreak_w) break;
     }
}

static void
evas_object_image_render_pre(Evas_Object *eo_obj,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;
   int is_v = 0, was_v = 0;
   Eina_Bool changed_prep = EINA_TRUE;

   /* image is not ready yet, skip rendering. Leave it to next frame */
   if (o->preload & EVAS_IMAGE_PRELOADING) return;
   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   Evas_Public_Data *e = obj->layer->evas;

   if ((o->cur->fill.w < 1) || (o->cur->fill.h < 1)) return;

   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          evas_object_clip_recalc(obj->cur->clipper);
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
                                            obj->cur->clipper,
                                            obj->cur->clipper->private_data);
     }
   /* Proxy: Do it early */
   if (o->cur->source)
     {
        Evas_Object_Protected_Data *source = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);
        if (source->proxy->redraw || source->changed)
          {
             /* XXX: Do I need to sort out the map here? */
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
     }
   else if (o->cur->scene)
     {
        Evas_Canvas3D_Scene *scene = o->cur->scene;
        Eina_Bool dirty;

        dirty = evas_canvas3d_object_dirty_get(scene, EVAS_CANVAS3D_STATE_ANY);
        if (dirty)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
     }

   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj, obj);
   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&e->clip_changes, eo_obj, is_v, was_v);
        goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   if (was_v)
     evas_object_render_pre_clipper_change(&e->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a) ||
       (obj->cur->cache.clip.r != obj->prev->cache.clip.r) ||
       (obj->cur->cache.clip.g != obj->prev->cache.clip.g) ||
       (obj->cur->cache.clip.b != obj->prev->cache.clip.b) ||
       (obj->cur->cache.clip.a != obj->prev->cache.clip.a))
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        goto done;
     }
   if ((obj->cur->render_op != obj->prev->render_op) ||  /* if it changed render op */
       (obj->cur->anti_alias != obj->prev->anti_alias))  /* if it changed anti_alias */
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        goto done;
     }
   if (o->changed)
     {
        if (((o->cur->f) && (!o->prev->f)) ||
            ((!o->cur->f) && (o->prev->f)) ||
            ((o->cur->key) && (!o->prev->key)) ||
            ((!o->cur->key) && (o->prev->key))
            )
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
        if ((o->cur->image.w != o->prev->image.w) ||
            (o->cur->image.h != o->prev->image.h) ||
            (o->cur->has_alpha != o->prev->has_alpha) ||
            (o->cur->cspace != o->prev->cspace) ||
            (o->cur->smooth_scale != o->prev->smooth_scale))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
        if ((o->cur->border.l != o->prev->border.l) ||
            (o->cur->border.r != o->prev->border.r) ||
            (o->cur->border.t != o->prev->border.t) ||
            (o->cur->border.b != o->prev->border.b) ||
            (o->cur->border.fill != o->prev->border.fill) ||
            (!EINA_DBL_EQ(o->cur->border.scale, o->prev->border.scale)))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
        if ((o->cur->frame != o->prev->frame) ||
            (o->cur->orient != o->prev->orient) ||
             /* Legacy compatibility (invalid behaviour): dirty_set() used to
              * trigger full image redraw, even though this was not correct. */
            (o->dirty_pixels && !o->pixels->pixel_updates))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
        //pre-loading is finished
        if (o->preload == EVAS_IMAGE_PRELOADED)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             o->preload = EVAS_IMAGE_PRELOAD_NONE;
             goto done;
          }
     }
   if (((obj->cur->geometry.x != obj->prev->geometry.x) ||
        (obj->cur->geometry.y != obj->prev->geometry.y) ||
        (obj->cur->geometry.w != obj->prev->geometry.w) ||
        (obj->cur->geometry.h != obj->prev->geometry.h))
       )
     {
        if ((obj->cur->geometry.w == obj->prev->geometry.w) &&
            (obj->cur->geometry.h == obj->prev->geometry.h))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
          }
        else
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
     }
   if (o->changed)
     {
        if ((o->cur->fill.x != o->prev->fill.x) ||
            (o->cur->fill.y != o->prev->fill.y) ||
            (o->cur->fill.w != o->prev->fill.w) ||
            (o->cur->fill.h != o->prev->fill.h))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
        if (o->pixels->pixel_updates)
          {
             if ((o->cur->border.l == 0) &&
                 (o->cur->border.r == 0) &&
                 (o->cur->border.t == 0) &&
                 (o->cur->border.b == 0) &&
                 (o->cur->image.w > 0) &&
                 (o->cur->image.h > 0) &&
                 (!((obj->map->cur.map) && (obj->map->cur.usemap))))
               {
                  Eina_Rectangle *rr;

                  if ((!o->cur->has_alpha) &&
                      (evas_object_is_opaque(eo_obj, obj)) &&
                      (obj->cur->color.a == 255))
                    {
                       Evas_Coord x, y, w, h;

                       x = obj->cur->cache.clip.x;
                       y = obj->cur->cache.clip.y;
                       w = obj->cur->cache.clip.w;
                       h = obj->cur->cache.clip.h;
                       if (obj->cur->clipper)
                         {
                            if (obj->cur->clipper->cur->cache.clip.a != 255)
                              w = 0;
                            else
                              {
                                 RECTS_CLIP_TO_RECT(x, y, w, h,
                                                    obj->cur->clipper->cur->cache.clip.x,
                                                    obj->cur->clipper->cur->cache.clip.y,
                                                    obj->cur->clipper->cur->cache.clip.w,
                                                    obj->cur->clipper->cur->cache.clip.h);
                              }
                         }
                       if ((w > 0) && (h > 0))
                         evas_render_update_del(e,
                                                x + e->framespace.x,
                                                y + e->framespace.y,
                                                w, h);
                    }
                  EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
                  {
                     EINA_LIST_FREE(pixi_write->pixel_updates, rr)
                       {
                          Evas_Coord idw, idh, idx, idy;
                          int x, y, w, h;
                          e->engine.func->image_dirty_region(ENC, o->engine_data, rr->x, rr->y, rr->w, rr->h);

                          idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur->fill.x, o->cur->fill.w, &idw);
                          idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur->fill.y, o->cur->fill.h, &idh);

                          if (idw < 1) idw = 1;
                          if (idh < 1) idh = 1;
                          if (idx > 0) idx -= idw;
                          if (idy > 0) idy -= idh;
                          while (idx < obj->cur->geometry.w)
                            {
                               Evas_Coord ydy;

                               ydy = idy;
                               x = idx;
                               w = ((int)(idx + idw)) - x;
                               while (idy < obj->cur->geometry.h)
                                 {
                                    Eina_Rectangle r;

                                    y = idy;
                                    h = ((int)(idy + idh)) - y;

                                    r.x = (rr->x * w) / o->cur->image.w;
                                    r.y = (rr->y * h) / o->cur->image.h;
                                    r.w = ((rr->w * w) + (o->cur->image.w * 2) - 1) / o->cur->image.w;
                                    r.h = ((rr->h * h) + (o->cur->image.h * 2) - 1) / o->cur->image.h;
                                    r.x += obj->cur->geometry.x + x;
                                    r.y += obj->cur->geometry.y + y;
                                    RECTS_CLIP_TO_RECT(r.x, r.y, r.w, r.h,
                                                       obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                                                       obj->cur->cache.clip.w, obj->cur->cache.clip.h);
                                    evas_add_rect(&e->clip_changes, r.x, r.y, r.w, r.h);
                                    idy += h;
                                 }
                               idx += idw;
                               idy = ydy;
                            }
                          eina_rectangle_free(rr);
                       }
                  }
                  EINA_COW_PIXEL_WRITE_END(o, pixi_write);
               }
             else
               {
                  if ((o->cur->image.w > 0) &&
                      (o->cur->image.h > 0) &&
                      (o->cur->image.w == obj->cur->geometry.w) &&
                      (o->cur->image.h == obj->cur->geometry.h) &&
                      (o->cur->fill.x == 0) &&
                      (o->cur->fill.y == 0) &&
                      (o->cur->fill.w == o->cur->image.w) &&
                      (o->cur->fill.h == o->cur->image.h) &&
                      (!((obj->map->cur.map) && (obj->map->cur.usemap))))
                    {
                       Eina_Rectangle *rr;

                       if ((!o->cur->has_alpha) &&
                           (evas_object_is_opaque(eo_obj, obj)) &&
                           (obj->cur->color.a == 255))
                         {
                            Evas_Coord x, y, w, h;

                            x = obj->cur->cache.clip.x;
                            y = obj->cur->cache.clip.y;
                            w = obj->cur->cache.clip.w;
                            h = obj->cur->cache.clip.h;
                            if (obj->cur->clipper)
                              {
                                 if (obj->cur->clipper->cur->cache.clip.a != 255)
                                   w = 0;
                                 else
                                   {
                                      RECTS_CLIP_TO_RECT(x, y, w, h,
                                                         obj->cur->clipper->cur->cache.clip.x,
                                                         obj->cur->clipper->cur->cache.clip.y,
                                                         obj->cur->clipper->cur->cache.clip.w,
                                                         obj->cur->clipper->cur->cache.clip.h);
                                   }
                              }
                            if ((w > 0) && (h > 0))
                              evas_render_update_del(e,
                                                     x + e->framespace.x,
                                                     y + e->framespace.y,
                                                     w, h);
                         }
                       else if ((o->cur->border.fill == EVAS_BORDER_FILL_SOLID) &&
                                (obj->cur->color.a == 255))
                         {
                            Evas_Coord x, y, w, h;

                            x = obj->cur->geometry.x + o->cur->border.l;
                            y = obj->cur->geometry.y + o->cur->border.t;
                            w = obj->cur->geometry.w - o->cur->border.l - o->cur->border.r;
                            h = obj->cur->geometry.h - o->cur->border.t - o->cur->border.b;
                            if (obj->cur->clipper)
                              {
                                 if (obj->cur->clipper->cur->cache.clip.a != 255)
                                   w = 0;
                                 else
                                   {
                                      RECTS_CLIP_TO_RECT(x, y, w, h,
                                                         obj->cur->clipper->cur->cache.clip.x,
                                                         obj->cur->clipper->cur->cache.clip.y,
                                                         obj->cur->clipper->cur->cache.clip.w,
                                                         obj->cur->clipper->cur->cache.clip.h);
                                   }
                              }
                            if ((w > 0) && (h > 0))
                              evas_render_update_del(e,
                                                     x + e->framespace.x,
                                                     y + e->framespace.y,
                                                     w, h);
                         }
                       EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
                       {
                          EINA_LIST_FREE(pixi_write->pixel_updates, rr)
                            {
                               Eina_Rectangle r;

                               e->engine.func->image_dirty_region(ENC, o->engine_data, rr->x, rr->y, rr->w, rr->h);
                               r.x = rr->x;
                               r.y = rr->y;
                               r.w = rr->w;
                               r.h = rr->h;
                               r.x += obj->cur->geometry.x;
                               r.y += obj->cur->geometry.y;
                               RECTS_CLIP_TO_RECT(r.x, r.y, r.w, r.h,
                                                  obj->cur->cache.clip.x, obj->cur->cache.clip.y,
                                                  obj->cur->cache.clip.w, obj->cur->cache.clip.h);
                               evas_add_rect(&e->clip_changes, r.x, r.y, r.w, r.h);
                               eina_rectangle_free(rr);
                            }
                       }
                       EINA_COW_PIXEL_WRITE_END(o, pixi_write);
                    }
                  else
                    {
                       Eina_Rectangle *r;

                       EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
                       {
                          EINA_LIST_FREE(pixi_write->pixel_updates, r)
                            eina_rectangle_free(r);
                       }
                       EINA_COW_PIXEL_WRITE_END(o, pixi_write);
                       e->engine.func->image_dirty_region(ENC, o->engine_data, 0, 0, o->cur->image.w, o->cur->image.h);

                       evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj,
                                                           obj);
                    }
               }
             goto done;
          }
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* aren't fully opaque and we are visible */
   if (evas_object_is_opaque(eo_obj, obj))
     {
        Evas_Coord x, y, w, h;

        x = obj->cur->cache.clip.x;
        y = obj->cur->cache.clip.y;
        w = obj->cur->cache.clip.w;
        h = obj->cur->cache.clip.h;
        if (obj->cur->clipper)
          {
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur->clipper->cur->cache.clip.x,
                                obj->cur->clipper->cur->cache.clip.y,
                                obj->cur->clipper->cur->cache.clip.w,
                                obj->cur->clipper->cur->cache.clip.h);
          }
        evas_render_update_del(e,
                               x + e->framespace.x,
                               y + e->framespace.y,
                               w, h);
        changed_prep = EINA_FALSE;
     }
   else
     changed_prep = EINA_FALSE;
done:
   evas_object_render_pre_effect_updates(&e->clip_changes, eo_obj, is_v, was_v);
   if (o->pixels->pixel_updates)
     {
        Eina_Rectangle *rr;

        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        {
           EINA_LIST_FREE(pixi_write->pixel_updates, rr)
             {
                eina_rectangle_free(rr);
             }
        }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }
   if (changed_prep)
     {
        if (o->engine_data_prep)
          {
             ENFN->image_free(ENC, o->engine_data_prep);
             o->engine_data_prep = NULL;
          }
     }
   o->changed = EINA_FALSE;
}

static void
evas_object_image_render_post(Evas_Object *eo_obj EINA_UNUSED,
                              Evas_Object_Protected_Data *obj,
                              void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;
   Eina_Rectangle *r;

   /* image is not ready yet, skip rendering. Leave it to next frame */
   if (o->preload & EVAS_IMAGE_PRELOADING) return;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);

   if (o->pixels->pixel_updates)
     {
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        {
           EINA_LIST_FREE(pixi_write->pixel_updates, r)
             eina_rectangle_free(r);
        }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }

   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
   eina_cow_memcpy(evas_object_image_state_cow, (const Eina_Cow_Data **)&o->prev, o->cur);
   /* FIXME: copy strings across */

   //Somehow(preloading cancelled) image has been changed, need to redraw.
   if (o->changed) evas_object_change(eo_obj, obj);
}

static void *
evas_object_image_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_image_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                            Evas_Object_Protected_Data *obj,
                            void *type_private_data)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
/*  disable caching due tyo maps screwing with this
    o->cur.opaque_valid = 0;*/
   Evas_Image_Data *o = type_private_data;

   if (o->preload == EVAS_IMAGE_PRELOADING) return 0;

   if (o->cur->opaque_valid)
     {
        if (!o->cur->opaque) return 0;
     }
   else
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->opaque = 0;
           state_write->opaque_valid = 1;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

        if ((o->cur->fill.w < 1) || (o->cur->fill.h < 1))
          return o->cur->opaque;
        if (((o->cur->border.l != 0) ||
             (o->cur->border.r != 0) ||
             (o->cur->border.t != 0) ||
             (o->cur->border.b != 0)) &&
            (!o->cur->border.fill))
          return o->cur->opaque;
        if (!o->engine_data)
          return o->cur->opaque;
        if (o->has_filter)
          return o->cur->opaque;

        // FIXME: use proxy
        if (o->cur->source)
          {
             Evas_Object_Protected_Data *cur_source = efl_data_scope_get(o->cur->source, EFL_CANVAS_OBJECT_CLASS);
             EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
             {
                state_write->opaque = evas_object_is_opaque(o->cur->source, cur_source);
             }
             EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
             return o->cur->opaque; /* FIXME: Should go poke at the object */
          }
        if (o->cur->has_alpha)
          return o->cur->opaque;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->opaque = 1;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   if ((obj->map->cur.map) && (obj->map->cur.usemap))
     {
        Evas_Map *m = obj->map->cur.map;

        if ((m->points[0].a == 255) &&
            (m->points[1].a == 255) &&
            (m->points[2].a == 255) &&
            (m->points[3].a == 255))
          {
             if (((EINA_DBL_EQ(m->points[0].x, m->points[3].x)) &&
                  (EINA_DBL_EQ(m->points[1].x, m->points[2].x)) &&
                  (EINA_DBL_EQ(m->points[0].y, m->points[1].y)) &&
                  (EINA_DBL_EQ(m->points[2].y, m->points[3].y))) ||
                 ((EINA_DBL_EQ(m->points[0].x, m->points[1].x)) &&
                  (EINA_DBL_EQ(m->points[2].x, m->points[3].x)) &&
                  (EINA_DBL_EQ(m->points[0].y, m->points[3].y)) &&
                  (EINA_DBL_EQ(m->points[1].y, m->points[2].y))))
               {
                  if ((EINA_DBL_EQ(m->points[0].x, obj->cur->geometry.x)) &&
                      (EINA_DBL_EQ(m->points[0].y, obj->cur->geometry.y)) &&
                      (EINA_DBL_EQ(m->points[2].x,
                                   obj->cur->geometry.x + obj->cur->geometry.w)) &&
                      (EINA_DBL_EQ(m->points[2].y,
                                   obj->cur->geometry.y + obj->cur->geometry.h)))
                    return o->cur->opaque;
               }
          }

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->opaque = 0;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

        return o->cur->opaque;
     }
   if (obj->cur->render_op == EVAS_RENDER_COPY)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
        {
           state_write->opaque = 1;
        }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

        return o->cur->opaque;
     }
   return o->cur->opaque;
}

static int
evas_object_image_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   if (o->prev->opaque_valid)
     {
        if (!o->prev->opaque) return 0;
     }
   else
     {
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->opaque = 0;
           state_write->opaque_valid = 1;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);

        if ((o->prev->fill.w < 1) || (o->prev->fill.h < 1))
          return o->prev->opaque;
        if (((o->prev->border.l != 0) ||
             (o->prev->border.r != 0) ||
             (o->prev->border.t != 0) ||
             (o->prev->border.b != 0)) &&
            (!o->prev->border.fill))
          return o->prev->opaque;
        if (!o->engine_data)
          return o->prev->opaque;

        // FIXME: use proxy
        if (o->prev->source)
          return o->prev->opaque;  /* FIXME: Should go poke at the object */
        if (o->prev->has_alpha)
          return o->prev->opaque;
        if (o->has_filter)
          return o->cur->opaque;

        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->opaque = 1;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);
     }
   if (obj->map->prev.usemap)
     {
        Evas_Map *m = obj->map->prev.map;

        if ((m->points[0].a == 255) &&
            (m->points[1].a == 255) &&
            (m->points[2].a == 255) &&
            (m->points[3].a == 255))
          {
             if (((EINA_DBL_EQ(m->points[0].x, m->points[3].x)) &&
                  (EINA_DBL_EQ(m->points[1].x, m->points[2].x)) &&
                  (EINA_DBL_EQ(m->points[0].y, m->points[1].y)) &&
                  (EINA_DBL_EQ(m->points[2].y, m->points[3].y))) ||
                 ((EINA_DBL_EQ(m->points[0].x, m->points[1].x)) &&
                  (EINA_DBL_EQ(m->points[2].x, m->points[3].x)) &&
                  (EINA_DBL_EQ(m->points[0].y, m->points[3].y)) &&
                  (EINA_DBL_EQ(m->points[1].y, m->points[2].y))))
               {
                  if ((EINA_DBL_EQ(m->points[0].x, obj->prev->geometry.x)) &&
                      (EINA_DBL_EQ(m->points[0].y, obj->prev->geometry.y)) &&
                      (EINA_DBL_EQ(m->points[2].x,
                                   obj->prev->geometry.x + obj->prev->geometry.w)) &&
                      (EINA_DBL_EQ(m->points[2].y,
                                   obj->prev->geometry.y + obj->prev->geometry.h)))
                    return o->prev->opaque;
               }
          }

        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->opaque = 0;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);

        return o->prev->opaque;
     }
   if (obj->prev->render_op == EVAS_RENDER_COPY)
     {
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->opaque = 1;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);

        return o->prev->opaque;
     }
   if (obj->prev->render_op != EVAS_RENDER_BLEND)
     {
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
        {
           state_write->opaque = 0;
        }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);

        return o->prev->opaque;
     }
   return o->prev->opaque;
}

static int
evas_object_image_is_inside(Evas_Object *eo_obj,
                            Evas_Object_Protected_Data *obj,
                            void *type_private_data,
                            Evas_Coord px, Evas_Coord py)
{
   Evas_Image_Data *o = type_private_data;
   int imagew, imageh, uvw, uvh, ix, iy, iw, ih, idw, idh, idx, idy;
   int is_inside = 0;
   void *pixels;
   void *output;

   // FIXME: The below loop is incredibly dubious and probably should be simplified.
   // We're looking for one single pixel, not a random series of positions.
   // Original comment:
   /* the following code is similar to evas_object_image_render(), but doesn't
    * draw, just get the pixels so we can check the transparency.
    */
   output = _evas_object_image_output_find(obj);
   pixels = _evas_image_pixels_get(eo_obj, obj, ENC, output, NULL, NULL, 0, 0,
                                   &imagew, &imageh, &uvw, &uvh, EINA_TRUE, EINA_FALSE);
   if (!pixels) return is_inside;

   if (o->dirty_pixels)
     {
        if (o->pixels->func.get_pixels)
          {
             if (ENFN->gl_get_pixels_pre)
               ENFN->gl_get_pixels_pre(ENC, output);
             o->pixels->func.get_pixels(o->pixels->func.get_pixels_data, eo_obj);
             if (ENFN->gl_get_pixels_post)
               ENFN->gl_get_pixels_post(ENC, output);
          }
     }

   /* TODO: not handling map, need to apply map to point */
   if ((obj->map->cur.map) && (obj->map->cur.map->count > 3) && (obj->map->cur.usemap))
     {
        evas_object_map_update(eo_obj, 0, 0, imagew, imageh, uvw, uvh);

        ERR("map not supported");
        return is_inside;
     }

   idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur->fill.x, o->cur->fill.w, &idw);
   idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur->fill.y, o->cur->fill.h, &idh);
   if (idw < 1) idw = 1;
   if (idh < 1) idh = 1;
   if (idx > 0) idx -= idw;
   if (idy > 0) idy -= idh;

   while (idx < obj->cur->geometry.w)
     {
        int ydy, dobreak_w = 0;

        ydy = idy;
        ix = idx;
        if ((o->cur->fill.w == obj->cur->geometry.w) &&
            (o->cur->fill.x == 0))
          {
             dobreak_w = 1;
             iw = obj->cur->geometry.w;
          }
        else
          iw = idx + idw - ix;

        while (idy < obj->cur->geometry.h)
          {
             int dobreak_h = 0;
             DATA8 alpha = 0;

             iy = idy;
             if ((o->cur->fill.h == obj->cur->geometry.h) &&
                 (o->cur->fill.y == 0))
               {
                  ih = obj->cur->geometry.h;
                  dobreak_h = 1;
               }
             else
               ih = idy + idh - iy;

             if ((o->cur->border.l == 0) && (o->cur->border.r == 0) &&
                 (o->cur->border.t == 0) && (o->cur->border.b == 0) &&
                 (o->cur->border.fill != 0))
               {
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            0, 0, imagew, imageh,
                                            obj->cur->geometry.x + ix,
                                            obj->cur->geometry.y + iy,
                                            iw, ih))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
               }
             else
               {
                  int inx, iny, inw, inh, outx, outy, outw, outh;
                  int bl, br, bt, bb, bsl, bsr, bst, bsb;
                  int imw, imh, ox, oy;

                  ox = obj->cur->geometry.x + ix;
                  oy = obj->cur->geometry.y + iy;
                  imw = imagew;
                  imh = imageh;
                  bl = o->cur->border.l;
                  br = o->cur->border.r;
                  bt = o->cur->border.t;
                  bb = o->cur->border.b;
                  if ((bl + br) > iw)
                    {
                       bl = iw / 2;
                       br = iw - bl;
                    }
                  if ((bl + br) > imw)
                    {
                       bl = imw / 2;
                       br = imw - bl;
                    }
                  if ((bt + bb) > ih)
                    {
                       bt = ih / 2;
                       bb = ih - bt;
                    }
                  if ((bt + bb) > imh)
                    {
                       bt = imh / 2;
                       bb = imh - bt;
                    }
                  if (!EINA_DBL_EQ(o->cur->border.scale, 1.0))
                    {
                       bsl = ((double)bl * o->cur->border.scale);
                       bsr = ((double)br * o->cur->border.scale);
                       bst = ((double)bt * o->cur->border.scale);
                       bsb = ((double)bb * o->cur->border.scale);
                    }
                  else
                    {
                       bsl = bl; bsr = br; bst = bt; bsb = bb;
                    }
                  // #--
                  // |
                  inx = 0; iny = 0;
                  inw = bl; inh = bt;
                  outx = ox; outy = oy;
                  outw = bsl; outh = bst;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }

                  // .##
                  // |
                  inx = bl; iny = 0;
                  inw = imw - bl - br; inh = bt;
                  outx = ox + bsl; outy = oy;
                  outw = iw - bsl - bsr; outh = bst;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  // --#
                  //   |
                  inx = imw - br; iny = 0;
                  inw = br; inh = bt;
                  outx = ox + iw - bsr; outy = oy;
                  outw = bsr; outh = bst;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  // .--
                  // #
                  inx = 0; iny = bt;
                  inw = bl; inh = imh - bt - bb;
                  outx = ox; outy = oy + bst;
                  outw = bsl; outh = ih - bst - bsb;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  // .--.
                  // |##|
                  if (o->cur->border.fill > EVAS_BORDER_FILL_NONE)
                    {
                       inx = bl; iny = bt;
                       inw = imw - bl - br; inh = imh - bt - bb;
                       outx = ox + bsl; outy = oy + bst;
                       outw = iw - bsl - bsr; outh = ih - bst - bsb;
                       if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                                 inx, iny, inw, inh,
                                                 outx, outy, outw, outh))
                         {
                            is_inside = alpha > 0;
                            dobreak_h = 1;
                            dobreak_w = 1;
                            break;
                         }
                    }
                  // --.
                  //   #
                  inx = imw - br; iny = bt;
                  inw = br; inh = imh - bt - bb;
                  outx = ox + iw - bsr; outy = oy + bst;
                  outw = bsr; outh = ih - bst - bsb;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  // |
                  // #--
                  inx = 0; iny = imh - bb;
                  inw = bl; inh = bb;
                  outx = ox; outy = oy + ih - bsb;
                  outw = bsl; outh = bsb;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  // |
                  // .##
                  inx = bl; iny = imh - bb;
                  inw = imw - bl - br; inh = bb;
                  outx = ox + bsl; outy = oy + ih - bsb;
                  outw = iw - bsl - bsr; outh = bsb;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
                  //   |
                  // --#
                  inx = imw - br; iny = imh - bb;
                  inw = br; inh = bb;
                  outx = ox + iw - bsr; outy = oy + ih - bsb;
                  outw = bsr; outh = bsb;
                  if (ENFN->pixel_alpha_get(pixels, px, py, &alpha,
                                            inx, iny, inw, inh,
                                            outx, outy, outw, outh))
                    {
                       is_inside = alpha > 0;
                       dobreak_h = 1;
                       dobreak_w = 1;
                       break;
                    }
               }
             idy += idh;
             if (dobreak_h) break;
          }
        idx += idw;
        idy = ydy;
        if (dobreak_w) break;
     }

   return is_inside;
}

static int
evas_object_image_has_opaque_rect(Evas_Object *eo_obj EINA_UNUSED,
                                  Evas_Object_Protected_Data *obj,
                                  void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;

   if ((obj->map->cur.map) && (obj->map->cur.usemap)) return 0;
   if (((o->cur->border.l | o->cur->border.r | o->cur->border.t | o->cur->border.b) != 0) &&
       (o->cur->border.fill == EVAS_BORDER_FILL_SOLID) &&
       (obj->cur->render_op == EVAS_RENDER_BLEND) &&
       (obj->cur->cache.clip.a == 255) &&
       (o->cur->fill.x == 0) &&
       (o->cur->fill.y == 0) &&
       (o->cur->fill.w == obj->cur->geometry.w) &&
       (o->cur->fill.h == obj->cur->geometry.h)
       ) return 1;
   return 0;
}

static int
evas_object_image_get_opaque_rect(Evas_Object *eo_obj EINA_UNUSED,
                                  Evas_Object_Protected_Data *obj,
                                  void *type_private_data,
                                  Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Image_Data *o = type_private_data;

   if (!o->cur->has_alpha)
     {
        *x = obj->cur->geometry.x;
        *y = obj->cur->geometry.y;
        *w = obj->cur->geometry.w;
        *h = obj->cur->geometry.h;
     }
   else if (o->cur->border.fill == EVAS_BORDER_FILL_SOLID)
     {
        *x = obj->cur->geometry.x + (o->cur->border.l * o->cur->border.scale);
        *y = obj->cur->geometry.y + (o->cur->border.t * o->cur->border.scale);
        *w = obj->cur->geometry.w - ((o->cur->border.l * o->cur->border.scale)
                                     + (o->cur->border.r * o->cur->border.scale));
        if (*w < 0) *w = 0;
        *h = obj->cur->geometry.h - ((o->cur->border.t * o->cur->border.scale)
                                     + (o->cur->border.b * o->cur->border.scale));
        if (*h < 0) *h = 0;
     }
   else
     {
        *w = 0;
        *h = 0;
     }
   return 1;
}

static int
evas_object_image_can_map(Evas_Object *obj EINA_UNUSED)
{
   return 1;
}

void *
_evas_image_data_convert_internal(Evas_Image_Data *o, void *data, Evas_Colorspace to_cspace)
{
   void *out = NULL;

   if (!data)
     return NULL;

   switch (o->cur->cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        out = evas_common_convert_argb8888_to(data,
                                              o->cur->image.w,
                                              o->cur->image.h,
                                              o->cur->image.stride >> 2,
                                              o->cur->has_alpha,
                                              to_cspace);
        break;

      case EVAS_COLORSPACE_RGB565_A5P:
        out = evas_common_convert_rgb565_a5p_to(data,
                                                o->cur->image.w,
                                                o->cur->image.h,
                                                o->cur->image.stride >> 1,
                                                o->cur->has_alpha,
                                                to_cspace);
        break;

      case EVAS_COLORSPACE_YCBCR422601_PL:
        out = evas_common_convert_yuv_422_601_to(data,
                                                 o->cur->image.w,
                                                 o->cur->image.h,
                                                 to_cspace);
        break;

      case EVAS_COLORSPACE_YCBCR422P601_PL:
        out = evas_common_convert_yuv_422P_601_to(data,
                                                  o->cur->image.w,
                                                  o->cur->image.h,
                                                  to_cspace);
        break;

      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
        out = evas_common_convert_yuv_420_601_to(data,
                                                 o->cur->image.w,
                                                 o->cur->image.h,
                                                 to_cspace);
        break;

      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        out = evas_common_convert_yuv_420T_601_to(data,
                                                  o->cur->image.w,
                                                  o->cur->image.h,
                                                  to_cspace);
        break;

      case EVAS_COLORSPACE_AGRY88:
        out = evas_common_convert_agry88_to(data,
                                            o->cur->image.w,
                                            o->cur->image.h,
                                            o->cur->image.stride,
                                            o->cur->has_alpha,
                                            to_cspace);
        break;

      case EVAS_COLORSPACE_GRY8:
        out = evas_common_convert_gry8_to(data,
                                          o->cur->image.w,
                                          o->cur->image.h,
                                          o->cur->image.stride,
                                          o->cur->has_alpha,
                                          to_cspace);
        break;

      default:
        WRN("unknown colorspace: %i\n", o->cur->cspace);
        break;
     }

   return out;
}

static void
evas_object_image_filled_resize_listener(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *einfo EINA_UNUSED)
{
   Evas_Image_Data *o = efl_data_scope_get(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS);
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(obj);
   if (sz.w < 1) sz.w = 1;
   if (sz.h < 1) sz.h = 1;
   _evas_image_fill_set(obj, o, 0, 0, sz.w, sz.h);
}

Eina_Bool
_evas_object_image_preloading_get(const Evas_Object *eo_obj)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   return o->preload;
}

Evas_Object *
_evas_object_image_video_parent_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   return o->video_surface ? o->pixels->video.parent : NULL;
}

void
_evas_object_image_video_overlay_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   if (obj->cur->cache.clip.x != obj->prev->cache.clip.x ||
       obj->cur->cache.clip.y != obj->prev->cache.clip.y ||
       o->created || !o->video_visible)
     o->delayed.video_move = EINA_TRUE;

   if (obj->cur->cache.clip.w != obj->prev->cache.clip.w ||
       obj->cur->cache.clip.h != obj->prev->cache.clip.h ||
       o->created || !o->video_visible)
     o->delayed.video_resize = EINA_TRUE;

   if (!o->video_visible || o->created)
     {
        o->delayed.video_show = EINA_TRUE;
        o->delayed.video_hide = EINA_FALSE;
     }
   else
     {
        /* Cancel dirty on the image */
        Eina_Rectangle *r;

        o->dirty_pixels = EINA_FALSE;

        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
        {
           EINA_LIST_FREE(pixi_write->pixel_updates, r)
             eina_rectangle_free(r);
        }
        EINA_COW_PIXEL_WRITE_END(o, pixi_write);
     }
   o->video_visible = EINA_TRUE;
   o->created = EINA_FALSE;
}

void
_evas_object_image_video_overlay_hide(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   if (o->video_visible || o->created)
     {
        o->delayed.video_hide = EINA_TRUE;
        o->delayed.video_show = EINA_FALSE;
     }
   if (evas_object_is_visible(eo_obj, obj))
     o->pixels->video.update_pixels(o->pixels->video.data, eo_obj, &o->pixels->video);
   o->video_visible = EINA_FALSE;
   o->created = EINA_FALSE;
}

void
_evas_object_image_video_overlay_do(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Image_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Evas_Public_Data *e = obj->layer->evas;

   if (o->delayed.video_move)
     o->pixels->video.move(o->pixels->video.data, eo_obj, &o->pixels->video,
                           obj->cur->cache.clip.x + e->framespace.x,
                           obj->cur->cache.clip.y + e->framespace.y);

   if (o->delayed.video_resize)
     o->pixels->video.resize(o->pixels->video.data, eo_obj,
                             &o->pixels->video,
                             obj->cur->cache.clip.w,
                             obj->cur->cache.clip.h);

   if (o->delayed.video_show)
     o->pixels->video.show(o->pixels->video.data, eo_obj, &o->pixels->video);
   else if (o->delayed.video_hide)
     o->pixels->video.hide(o->pixels->video.data, eo_obj, &o->pixels->video);

   o->delayed.video_move = EINA_FALSE;
   o->delayed.video_resize = EINA_FALSE;
   o->delayed.video_show = EINA_FALSE;
   o->delayed.video_hide = EINA_FALSE;
}

void *
_evas_object_image_surface_get(Evas_Object_Protected_Data *obj, Eina_Bool create)
{
   Evas_Image_Data *pd = obj->private_data;

   if (pd->engine_data &&
       (pd->cur->image.w == obj->cur->geometry.w) &&
       (pd->cur->image.h == obj->cur->geometry.h))
     return pd->engine_data;

   if (pd->engine_data)
     {
        ENFN->image_free(ENC, pd->engine_data);
        pd->engine_data = NULL;
     }

   if (!create) return pd->engine_data;

   // FIXME: alpha forced to 1 for now, need to figure out Evas alpha here
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(pd, state_write)
   {
      pd->engine_data = ENFN->image_map_surface_new(ENC,
                                                    obj->cur->geometry.w,
                                                    obj->cur->geometry.h,
                                                    1);
      state_write->image.w = obj->cur->geometry.w;
      state_write->image.h = obj->cur->geometry.h;
   }
   EINA_COW_IMAGE_STATE_WRITE_END(pd, state_write);

   return pd->engine_data;
}

EOLIAN static void
_efl_canvas_image_internal_efl_object_debug_name_override(Eo *eo_obj, Evas_Image_Data *o, Eina_Strbuf *sb)
{
   efl_debug_name_override(efl_super(eo_obj, MY_CLASS), sb);
   if (o->cur->f)
     {
        const char *fname = eina_file_filename_get(o->cur->f);
        eina_strbuf_append_printf(sb, ":file='%s',key='%s'", fname, o->cur->key);
     }
   else if (o->pixels && o->pixels->func.get_pixels)
     {
        eina_strbuf_append_printf(sb, ":get_pixels=%p:dirty=%d",
                                  o->pixels->func.get_pixels, o->dirty_pixels);
     }
   else if (o->cur->source)
     {
        eina_strbuf_append_printf(sb, ":proxy_source=%p", o->cur->source);
     }
   else
     {
        eina_strbuf_append_printf(sb, ":unknown_image");
     }
}

#define EFL_CANVAS_IMAGE_INTERNAL_EXTRA_OPS \
  EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _efl_canvas_image_internal_efl_object_dbg_info_get)

#include "canvas/efl_canvas_image_internal.eo.c"

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
