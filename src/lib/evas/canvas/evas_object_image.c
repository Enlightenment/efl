#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <math.h>

#include "evas_common_private.h"
#include "evas_private.h"
#ifdef EVAS_CSERVE2
#include "../cserve2/evas_cs2_private.h"
#endif
#include "../common/evas_convert_color.h"
#include "../common/evas_convert_colorspace.h"
#include "../common/evas_convert_yuv.h"
#include "evas_filter.h"

#define MY_CLASS EVAS_IMAGE_CLASS

#define MY_CLASS_NAME "Evas_Image"

#define VERBOSE_PROXY_ERROR 1

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image Evas_Image_Data;
typedef struct _Evas_Object_Image_Load_Opts Evas_Object_Image_Load_Opts;
typedef struct _Evas_Object_Image_Pixels Evas_Object_Image_Pixels;
typedef struct _Evas_Object_Image_State Evas_Object_Image_State;

struct _Evas_Object_Image_Load_Opts
{
   unsigned char  scale_down_by;
   double         dpi;
   short          w, h;
   struct {
      short       x, y, w, h;
   } region;
   struct {
      int src_x, src_y, src_w, src_h;
      int dst_w, dst_h;
      int smooth;
      int scale_hint;
   } scale_load;
   Eina_Bool  orientation : 1;
};

struct _Evas_Object_Image_Pixels
{
   Eina_List        *pixel_updates;
   struct {
      Evas_Object_Image_Pixels_Get_Cb  get_pixels;
      void                            *get_pixels_data;
   } func;

   Evas_Video_Surface video;
   unsigned int video_caps;
};

struct _Evas_Object_Image_State
{
   Evas_Coord_Rectangle fill;
   struct {
      short         w, h, stride;
   } image;
   struct {
      double        scale;
      short         l, r, t, b;
      unsigned char fill;
   } border;

   Evas_Object   *source;
   Evas_Map      *defmap;
   Evas_3D_Scene *scene;

   union {
      const char    *file;
      Eina_File     *f;
   } u;
   const char    *key;
   int            frame;
   int            spread;

   Evas_Colorspace cspace;

   struct {
      Eina_Stringshare    *code;
      Evas_Filter_Program *chain;
      Eina_Hash           *sources; // Evas_Filter_Proxy_Binding
      int                  sources_count;
      void                *output;
      Eina_Bool            changed : 1;
      Eina_Bool            invalid : 1; // Code parse failed
   } filter;

   Eina_Bool      smooth_scale : 1;
   Eina_Bool      has_alpha :1;
   Eina_Bool      opaque_valid : 1;
   Eina_Bool      opaque : 1;
   Eina_Bool      mmaped_source : 1;
};

struct _Evas_Object_Image
{
   const Evas_Object_Image_State *cur;
   const Evas_Object_Image_State *prev;

   // This pointer is an Eina_Cow pointer
   const Evas_Object_Image_Load_Opts *load_opts;
   const Evas_Object_Image_Pixels *pixels;

   void             *engine_data;

   int               pixels_checked_out;
   int               load_error;

   Evas_Image_Scale_Hint   scale_hint;
   Evas_Image_Content_Hint content_hint;

   Eina_Bool         changed : 1;
   Eina_Bool         dirty_pixels : 1;
   Eina_Bool         filled : 1;
   Eina_Bool         proxyrendering : 1;
   Eina_Bool         preloading : 1;
   Eina_Bool         video_surface : 1;
   Eina_Bool         video_visible : 1;
   Eina_Bool         created : 1;
   Eina_Bool         proxyerror : 1;
   Eina_Bool         proxy_src_clip : 1;
   Eina_Bool         written : 1;
   Eina_Bool         direct_render : 1;
   struct
   {
      Eina_Bool      video_move : 1;
      Eina_Bool      video_resize : 1;
      Eina_Bool      video_show : 1;
      Eina_Bool      video_hide : 1;
   } delayed;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *eo_obj, Eina_Bool dirty);
static void evas_object_image_load(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
static Evas_Coord evas_object_image_figure_x_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord evas_object_image_figure_y_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void evas_object_image_init(Evas_Object *eo_obj);
static void evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj,
				     void *type_private_data,
				     void *output, void *context, void *surface,
				     int x, int y, Eina_Bool do_async);
static void evas_object_image_free(Evas_Object *eo_obj,
				   Evas_Object_Protected_Data *obj);
static void evas_object_image_render_pre(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data);
static void evas_object_image_render_post(Evas_Object *eo_obj,
					  Evas_Object_Protected_Data *obj,
					  void *type_private_data);

static unsigned int evas_object_image_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_image_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_image_engine_data_get(Evas_Object *eo_obj);

static int evas_object_image_is_opaque(Evas_Object *eo_obj,
				       Evas_Object_Protected_Data *obj,
				       void *type_private_data);
static int evas_object_image_was_opaque(Evas_Object *eo_obj,
					Evas_Object_Protected_Data *obj,
					void *type_private_data);
static int evas_object_image_is_inside(Evas_Object *eo_obj,
				       Evas_Object_Protected_Data *obj,
				       void *type_private_data,
				       Evas_Coord x, Evas_Coord y);
static int evas_object_image_has_opaque_rect(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);
static int evas_object_image_get_opaque_rect(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data,
					     Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
static int evas_object_image_can_map(Evas_Object *eo_obj);

static void *evas_object_image_data_convert_internal(Evas_Image_Data *o, void *data, Evas_Colorspace to_cspace);
static void evas_object_image_filled_resize_listener(void *data, Evas *eo_e, Evas_Object *eo_obj, void *einfo);

static void _proxy_unset(Evas_Object *proxy, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
static void _proxy_set(Evas_Object *proxy, Evas_Object *src);
static void _proxy_error(Evas_Object *proxy, void *context, void *output, void *surface, int x, int y, Eina_Bool do_async);

static void _3d_set(Evas_Object *eo_obj, Evas_3D_Scene *scene);
static void _3d_unset(Evas_Object *eo_obj, Evas_Object_Protected_Data *image, Evas_Image_Data *o);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
     evas_object_image_render,
     evas_object_image_render_pre,
     evas_object_image_render_post,
     evas_object_image_id_get,
     evas_object_image_visual_id_get,
     evas_object_image_engine_data_get,
     /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_image_is_opaque,
     evas_object_image_was_opaque,
     evas_object_image_is_inside,
     NULL,
     NULL,
     NULL,
     evas_object_image_has_opaque_rect,
     evas_object_image_get_opaque_rect,
     evas_object_image_can_map
};

static const Evas_Object_Image_Load_Opts default_load_opts = {
  0, 0.0, 0, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0
};

static const Evas_Object_Image_Pixels default_pixels = {
  NULL, { NULL, NULL }, { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }, ~0x0
};

static const Evas_Object_Image_State default_state = {
  { 0, 0, 0, 0 }, // fill
  { 0, 0, 0 }, // image
  { 1.0, 0, 0, 0, 0, 1 }, // border
  NULL, NULL, NULL,  //source, defmap, scene
  { NULL }, //u
  NULL, //key
  0, //frame
  EVAS_TEXTURE_REPEAT,
  EVAS_COLORSPACE_ARGB8888,

  // filter
  { NULL, NULL, NULL, 0, NULL, EINA_FALSE, EINA_FALSE },

  // flags
  EINA_TRUE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE
};

Eina_Cow *evas_object_image_load_opts_cow = NULL;
Eina_Cow *evas_object_image_pixels_cow = NULL;
Eina_Cow *evas_object_image_state_cow = NULL;

# define EINA_COW_PIXEL_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_pixels_cow, Obj->pixels, Evas_Object_Image_Pixels, Write)

# define EINA_COW_PIXEL_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_pixels_cow, Obj->pixels, Write)


# define EINA_COW_LOAD_OPTS_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_load_opts_cow, Obj->load_opts, Evas_Object_Image_Load_Opts, Write)

# define EINA_COW_LOAD_OPTS_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_load_opts_cow, Obj->load_opts, Write)

# define EINA_COW_IMAGE_STATE_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, Obj->cur, Evas_Object_Image_State, Write)

# define EINA_COW_IMAGE_STATE_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_state_cow, Obj->cur, Write)

# define EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(Obj)                 \
  if ((!Obj->cur->mmaped_source && Obj->cur->u.file) || Obj->cur->key) \
    {                                                                   \
       EINA_COW_IMAGE_STATE_WRITE_BEGIN(Obj, cur_write)                 \
         {                                                              \
            EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, Obj->prev, Evas_Object_Image_State, prev_write) \
              EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(cur_write, prev_write); \
            EINA_COW_WRITE_END(evas_object_image_state_cow, Obj->prev, prev_write); \
         }                                                              \
       EINA_COW_IMAGE_STATE_WRITE_END(Obj, cur_write);                  \
    }

static void
_evas_object_image_cleanup(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->opaque_valid = 0;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if (o->cur->source) _proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _3d_unset(eo_obj, obj, o);
}

static Eina_Bool
_init_cow(Eo *eo_obj)
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
        eo_error_set(eo_obj);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static void
_evas_image_eo_base_constructor(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas *eo_e;
   Eo *parent = NULL;
   Evas_Colorspace cspace;

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   eo_do(eo_obj, parent = eo_parent_get());
   eo_e = evas_object_evas_get(parent);

   evas_object_image_init(eo_obj);
   evas_object_inject(eo_obj, obj, eo_e);

   if (!_init_cow(eo_obj))
     return;

   o->load_opts = eina_cow_alloc(evas_object_image_load_opts_cow);
   o->pixels = eina_cow_alloc(evas_object_image_pixels_cow);
   o->cur = eina_cow_alloc(evas_object_image_state_cow);
   o->prev = eina_cow_alloc(evas_object_image_state_cow);
   o->proxy_src_clip = EINA_TRUE;

   cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
                                                                o->engine_data);

   if (cspace != o->cur->cspace)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          state_write->cspace = cspace;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
}

EAPI Evas_Object *
evas_object_image_add(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->engine.func, NULL);
   Evas_Object *eo_obj = eo_add(EVAS_IMAGE_CLASS, eo_e);
   eo_unref(eo_obj);
   return eo_obj;
}

EAPI Evas_Object *
evas_object_image_filled_add(Evas *eo_e)
{
   Evas_Object *eo_obj;
   eo_obj = evas_object_image_add(eo_e);
   evas_object_image_filled_set(eo_obj, 1);
   return eo_obj;
}

EAPI void
evas_object_image_memfile_set(Evas_Object *eo_obj, void *data, int size, char *format EINA_UNUSED, char *key)
{
   Eina_File *f;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   f = eina_file_virtualize(NULL, data, size, EINA_TRUE);
   if (!f) return ;
   eo_do(eo_obj, evas_obj_image_mmap_set(f, key));
   eina_file_close(f);
}

static void
_image_init_set(const Eina_File *f, const char *file, const char *key,
                Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o,
                Evas_Image_Load_Opts *lo)
{
   if (o->cur->source) _proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _3d_unset(eo_obj, obj, o);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        if (f)
          {
             if (!state_write->mmaped_source)
               eina_stringshare_del(state_write->u.file);
	     else if (state_write->u.f)
	       eina_file_close(state_write->u.f);
             state_write->u.f = eina_file_dup(f);
          }
        else
          {
             if (!state_write->mmaped_source)
               eina_stringshare_replace(&state_write->u.file, file);
             else
	       {
                  if (state_write->u.f) eina_file_close(state_write->u.f);
                  state_write->u.file = eina_stringshare_add(file);
	       }
          }
        state_write->mmaped_source = !!f;
        eina_stringshare_replace(&state_write->key, key);
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->prev->u.file != NULL || o->prev->key != NULL)
     {
        EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, state_write)
          {
             state_write->u.file = NULL;
             state_write->key = NULL;
          }
        EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, state_write);
     }

   if (o->engine_data)
     {
        if (o->preloading)
          {
             o->preloading = EINA_FALSE;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output, o->engine_data, eo_obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output, o->engine_data);
     }
   o->load_error = EVAS_LOAD_ERROR_NONE;
   lo->scale_down_by = o->load_opts->scale_down_by;
   lo->dpi = o->load_opts->dpi;
   lo->w = o->load_opts->w;
   lo->h = o->load_opts->h;
   lo->region.x = o->load_opts->region.x;
   lo->region.y = o->load_opts->region.y;
   lo->region.w = o->load_opts->region.w;
   lo->region.h = o->load_opts->region.h;
   lo->scale_load.src_x = o->load_opts->scale_load.src_x;
   lo->scale_load.src_y = o->load_opts->scale_load.src_y;
   lo->scale_load.src_w = o->load_opts->scale_load.src_w;
   lo->scale_load.src_h = o->load_opts->scale_load.src_h;
   lo->scale_load.dst_w = o->load_opts->scale_load.dst_w;
   lo->scale_load.dst_h = o->load_opts->scale_load.dst_h;
   lo->scale_load.smooth = o->load_opts->scale_load.smooth;
   lo->scale_load.scale_hint = o->load_opts->scale_load.scale_hint;
   lo->orientation = o->load_opts->orientation;
   lo->degree = 0;
}

static void
_image_done_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   Eina_Bool resize_call = EINA_FALSE;

   if (o->engine_data)
     {
        int w, h;
        int stride;

        obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output, o->engine_data, &w, &h);
        if (obj->layer->evas->engine.func->image_stride_get)
          obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output, o->engine_data, &stride);
        else
          stride = w * 4;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             state_write->has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output, o->engine_data);
             state_write->cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output, o->engine_data);

             if ((o->cur->image.w != w) || (o->cur->image.h != h))
               resize_call = EINA_TRUE;

             state_write->image.w = w;
             state_write->image.h = h;
             state_write->image.stride = stride;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
   else
     {
        if (o->load_error == EVAS_LOAD_ERROR_NONE)
          o->load_error = EVAS_LOAD_ERROR_GENERIC;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             state_write->has_alpha = EINA_TRUE;
             state_write->cspace = EVAS_COLORSPACE_ARGB8888;

             if ((state_write->image.w != 0) || (state_write->image.h != 0))
               resize_call = EINA_TRUE;

             state_write->image.w = 0;
             state_write->image.h = 0;
             state_write->image.stride = 0;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
   o->written = EINA_FALSE;
   o->changed = EINA_TRUE;
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_image_mmap_set(Eo *eo_obj, Evas_Image_Data *o, const Eina_File *f, const char *key)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Load_Opts lo;

   if (o->cur->u.f == f)
     {
        if ((!o->cur->key) && (!key))
          return;
        if ((o->cur->key) && (key) && (!strcmp(o->cur->key, key)))
          return;
     }

   _image_init_set(f, NULL, key, eo_obj, obj, o, &lo);
   o->engine_data = obj->layer->evas->engine.func->image_mmap(obj->layer->evas->engine.data.output,
							      o->cur->u.f,
							      o->cur->key,
							      &o->load_error,
							      &lo);
   _image_done_set(eo_obj, obj, o);
}

EOLIAN static void
_evas_image_mmap_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, const Eina_File **f, const char **key)
{
   if (f)
     *f = o->cur->mmaped_source ? o->cur->u.f : NULL;
   if (key)
     *key = o->cur->key;
}

EOLIAN static void
_evas_image_file_set(Eo *eo_obj, Evas_Image_Data *o, const char *file, const char *key)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Load_Opts lo;

   if ((o->cur->u.file) && (file) && (!strcmp(o->cur->u.file, file)))
     {
        if ((!o->cur->key) && (!key))
          return;
        if ((o->cur->key) && (key) && (!strcmp(o->cur->key, key)))
          return;
     }
   /*
    * WTF? why cancel a null image preload? this is just silly (tm)
    if (!o->engine_data)
     obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
							      o->engine_data,
							      eo_obj);
 */
   _image_init_set(NULL, file, key, eo_obj, obj, o, &lo);
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
                                                              o->cur->u.file,
                                                              o->cur->key,
                                                              &o->load_error,
                                                              &lo);
   _image_done_set(eo_obj, obj, o);
}

EOLIAN static void
_evas_image_file_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, const char **file, const char **key)
{
   if (file) *file = o->cur->u.file;
   if (key) *key = o->cur->key;
}

EOLIAN static Eina_Bool
_evas_image_source_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Object *eo_src)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (obj->delete_me && eo_src)
     {
        WRN("Setting deleted object %p as image source %p", eo_src, eo_obj);
        return EINA_FALSE;
     }
   if (eo_src)
     {
        Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJECT_CLASS);
        if (src->delete_me)
          {
             WRN("Setting object %p to deleted image source %p", eo_src, eo_obj);
             return EINA_FALSE;
          }
        if (!src->layer)
          {
             CRI("No evas surface associated with source object (%p)", eo_src);
             return EINA_FALSE;
          }
        if (!obj->layer)
          {
             CRI("No evas surface associated with destination object (%p)", eo_obj);
             return EINA_FALSE;
          }
        if ((obj->layer && src->layer) &&
            (obj->layer->evas != src->layer->evas))
          {
             CRI("Setting object %p from Evas (%p) from another Evas (%p)", eo_src, src->layer->evas, obj->layer->evas);
             return EINA_FALSE;
          }
        if (eo_src == eo_obj)
          {
             CRI("Setting object %p as a source for itself", obj);
             return EINA_FALSE;
          }
     }
   if (o->cur->source == eo_src) return EINA_TRUE;

   _evas_object_image_cleanup(eo_obj, obj, o);
   /* Kill the image if any */
   if (o->cur->u.file || o->cur->key)
      evas_object_image_file_set(eo_obj, NULL, NULL);

   if (eo_src) _proxy_set(eo_obj, eo_src);
   else _proxy_unset(eo_obj, obj, o);

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_evas_image_source_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->source;
}

EAPI Eina_Bool
evas_object_image_source_unset(Evas_Object *eo_obj)
{
   Eina_Bool result = EINA_FALSE;
   eo_do(eo_obj, result = evas_obj_image_source_set(NULL));
   return result;
}

EOLIAN static void
_evas_image_source_clip_set(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Eina_Bool source_clip)
{
   Evas_Object_Protected_Data *src_obj;

   source_clip = !!source_clip;
   if (o->proxy_src_clip == source_clip) return;
   o->proxy_src_clip = source_clip;

   if (!o->cur->source) return;

   src_obj = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
   evas_object_change(o->cur->source, src_obj);
}

EOLIAN static Eina_Bool
_evas_image_source_clip_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->proxy_src_clip;
}

EOLIAN static void
_evas_image_source_events_set(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Eina_Bool source_events)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   source_events = !!source_events;
   if (obj->proxy->src_events == source_events) return;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_events = source_events;
   EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write);

   if (!o->cur->source) return;
   if ((obj->proxy->src_invisible) || (!source_events)) return;
   //FIXME: Feed mouse events here.
}

EOLIAN static Eina_Bool
_evas_image_source_events_get(Eo *eo_obj, Evas_Image_Data *_pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   return obj->proxy->src_events;
}

EOLIAN static void
_evas_image_eo_base_dbg_info_get(Eo *eo_obj, Evas_Image_Data *o, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *file, *key;
   if (o->cur->mmaped_source)
     file = eina_file_filename_get(o->cur->u.f);
   else
     file = o->cur->u.file;
   key = o->cur->key;

   EO_DBG_INFO_APPEND(group, "Image File", EINA_VALUE_TYPE_STRING, file);
   EO_DBG_INFO_APPEND(group, "Key", EINA_VALUE_TYPE_STRING, key);
   EO_DBG_INFO_APPEND(group, "Source", EINA_VALUE_TYPE_UINT64,
                          (uint64_t) (uintptr_t) evas_object_image_source_get(eo_obj));

   if (evas_object_image_load_error_get(eo_obj) != EVAS_LOAD_ERROR_NONE)
     {
        Evas_Load_Error error = EVAS_LOAD_ERROR_GENERIC;
        eo_do(eo_obj, error = evas_obj_image_load_error_get());
        EO_DBG_INFO_APPEND(group, "Load Error", EINA_VALUE_TYPE_STRING,
                                evas_load_error_str(error));
     }
}

EOLIAN static void
_evas_image_source_visible_set(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Eina_Bool visible)
{
   Evas_Object_Protected_Data *src_obj;

   if (!o->cur->source) return;

   visible = !!visible;
   src_obj = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
   if (src_obj->proxy->src_invisible == !visible) return;

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

EOLIAN static Eina_Bool
_evas_image_source_visible_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *src_obj;
   Eina_Bool visible;

   if (!o->cur->source) visible = EINA_FALSE;
   src_obj = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
   if (src_obj) visible = !src_obj->proxy->src_invisible;
   else visible = EINA_FALSE;

   return visible;
}


EOLIAN static void
_evas_image_scene_set(Eo *eo_obj, Evas_Image_Data *o, Evas_3D_Scene *scene)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Load_Opts lo;

   if (o->cur->scene == scene) return;

   _image_init_set(NULL, NULL, NULL, eo_obj, obj, o, &lo);
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
                                                              o->cur->u.file,
                                                              o->cur->key,
                                                              &o->load_error,
                                                              &lo);
   _image_done_set(eo_obj, obj, o);

   if (scene) _3d_set(eo_obj, scene);
   else _3d_unset(eo_obj, obj, o);
}

EOLIAN static Evas_3D_Scene *
_evas_image_scene_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->scene;
}

EOLIAN static void
_evas_image_border_set(Eo *eo_obj, Evas_Image_Data *o, int l, int r, int t, int b)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur->border.l == l) &&
       (o->cur->border.r == r) &&
       (o->cur->border.t == t) &&
       (o->cur->border.b == b)) return;

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
_evas_image_border_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, int *l, int *r, int *t, int *b)
{
   if (l) *l = o->cur->border.l;
   if (r) *r = o->cur->border.r;
   if (t) *t = o->cur->border.t;
   if (b) *b = o->cur->border.b;
}

EOLIAN static void
_evas_image_border_center_fill_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Border_Fill_Mode fill)
{
   if (fill == o->cur->border.fill) return;
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->border.fill = fill;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static Evas_Border_Fill_Mode
_evas_image_border_center_fill_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->border.fill;
}

static void
_evas_image_filled_set(Eo *eo_obj, Evas_Image_Data* o, Eina_Bool setting)
{
   setting = !!setting;
   if (o->filled == setting) return;

   o->filled = setting;
   if (!o->filled)
     evas_object_event_callback_del(eo_obj, EVAS_CALLBACK_RESIZE,
                                    evas_object_image_filled_resize_listener);
   else
     {
        Evas_Coord w, h;

        evas_object_geometry_get(eo_obj, NULL, NULL, &w, &h);
        evas_object_image_fill_set(eo_obj, 0, 0, w, h);

        evas_object_event_callback_add(eo_obj, EVAS_CALLBACK_RESIZE,
                                       evas_object_image_filled_resize_listener,
                                       NULL);
     }
}

EOLIAN static Eina_Bool
_evas_image_filled_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->filled;
}

EOLIAN static void
_evas_image_border_scale_set(Eo *eo_obj, Evas_Image_Data *o, double scale)
{
   Evas_Object_Protected_Data *obj;

   if (scale == o->cur->border.scale) return;
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->border.scale = scale;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static double
_evas_image_border_scale_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->border.scale;
}

EOLIAN static void
_evas_image_fill_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Protected_Data *obj;

   if (w == 0) return;
   if (h == 0) return;
   if (w < 0) w = -w;
   if (h < 0) h = -h;

   if ((o->cur->fill.x == x) &&
       (o->cur->fill.y == y) &&
       (o->cur->fill.w == w) &&
       (o->cur->fill.h == h)) return;

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
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_image_fill_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (x) *x = o->cur->fill.x;
   if (y) *y = o->cur->fill.y;
   if (w) *w = o->cur->fill.w;
   if (h) *h = o->cur->fill.h;
}

EOLIAN static void
_evas_image_fill_spread_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Fill_Spread spread)
{
   Evas_Object_Protected_Data *obj;

   if (spread == (Evas_Fill_Spread)o->cur->spread) return;

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->spread = spread;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static Evas_Fill_Spread
_evas_image_fill_spread_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (Evas_Fill_Spread)o->cur->spread;;
}

EOLIAN static void
_evas_image_size_set(Eo *eo_obj, Evas_Image_Data *o, int w, int h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   int stride = 0;

   if (o->cur->scene) return;

   _evas_object_image_cleanup(eo_obj, obj, o);
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
      o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output, o->engine_data, w, h);
   else
      o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data
      (obj->layer->evas->engine.data.output, w, h, NULL, o->cur->has_alpha,
          o->cur->cspace);

   if (o->engine_data)
     {
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
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
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur->has_alpha);
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

EOLIAN static void
_evas_image_size_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, int *w, int *h)
{
   if (w) *w = o->cur->image.w;
   if (h) *h = o->cur->image.h;
}

EOLIAN static int
_evas_image_stride_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->image.stride;;
}

EOLIAN static Evas_Load_Error
_evas_image_load_error_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->load_error;
}

EOLIAN static void*
_evas_image_data_convert(Eo *eo_obj, Evas_Image_Data *o, Evas_Colorspace to_cspace)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   DATA32 *data;
   void* result = NULL;

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output, o->engine_data, eo_obj);
     }
   if (!o->engine_data) return NULL;
   if (o->video_surface)
     o->pixels->video.update_pixels(o->pixels->video.data, eo_obj, &o->pixels->video);
   if (o->cur->cspace == to_cspace) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output, o->engine_data, 0, &data, &o->load_error);
   result = evas_object_image_data_convert_internal(o, data, to_cspace);
   if (o->engine_data)
     {
        o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output, o->engine_data, data);
     }

   return result;
}

EOLIAN static void
_evas_image_data_set(Eo *eo_obj, Evas_Image_Data *o, void *data)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   void *p_data;
   Eina_Bool resize_call = EINA_FALSE;


   evas_render_rendering_wait(obj->layer->evas);

   _evas_object_image_cleanup(eo_obj, obj, o);
   p_data = o->engine_data;
   if (data)
     {
        if (o->engine_data)
          {
             o->engine_data =
               obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
                                                             o->engine_data,
                                                             data);
          }
        else
          {
             o->engine_data =
                obj->layer->evas->engine.func->image_new_from_data(obj->layer->evas->engine.data.output,
                                                                  o->cur->image.w,
                                                                  o->cur->image.h,
                                                                  data,
                                                                  o->cur->has_alpha,
                                                                  o->cur->cspace);
          }
        if (o->engine_data)
          {
             int stride = 0;

             if (obj->layer->evas->engine.func->image_scale_hint_set)
                obj->layer->evas->engine.func->image_scale_hint_set
                (obj->layer->evas->engine.data.output,
                    o->engine_data, o->scale_hint);
             if (obj->layer->evas->engine.func->image_content_hint_set)
                obj->layer->evas->engine.func->image_content_hint_set
                (obj->layer->evas->engine.data.output,
                    o->engine_data, o->content_hint);
             if (obj->layer->evas->engine.func->image_stride_get)
                obj->layer->evas->engine.func->image_stride_get
                (obj->layer->evas->engine.data.output,
                    o->engine_data, &stride);
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
          obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output, o->engine_data);
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
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur->has_alpha);
*/
   if (o->pixels_checked_out > 0) o->pixels_checked_out--;
   if (p_data != o->engine_data)
     {
        EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(o);
        o->pixels_checked_out = 0;
     }
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
}

EOLIAN static void*
_evas_image_data_get(Eo *eo_obj, Evas_Image_Data *_pd EINA_UNUSED, Eina_Bool for_writing)
{
   Evas_Image_Data *o = (Evas_Image_Data *) _pd;
   DATA32 *data;

   if (!o->engine_data) return NULL;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (for_writing) evas_render_rendering_wait(obj->layer->evas);

   data = NULL;
   if (obj->layer->evas->engine.func->image_scale_hint_set)
      obj->layer->evas->engine.func->image_scale_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->scale_hint);
   if (obj->layer->evas->engine.func->image_content_hint_set)
      obj->layer->evas->engine.func->image_content_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->content_hint);
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output, o->engine_data, for_writing, &data, &o->load_error);

   /* if we fail to get engine_data, we have to return NULL */
   if (!o->engine_data) return NULL;

   if (o->engine_data)
     {
        int stride = 0;

        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
        else
           stride = o->cur->image.w * 4;

        if (o->cur->image.stride != stride)
          {
             EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
               state_write->image.stride = stride;
             EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
          }
     }
   o->pixels_checked_out++;
   if (for_writing)
     {
        o->written = EINA_TRUE;
        EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(o);
     }

   return data;
}

EAPI void
evas_object_image_preload(Evas_Object *eo_obj, Eina_Bool cancel)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (cancel)
     eo_do(eo_obj, evas_obj_image_preload_cancel());
   else
     eo_do(eo_obj, evas_obj_image_preload_begin());
}

static void
_image_preload_internal(Eo *eo_obj, void *_pd, Eina_Bool cancel)
{
   Evas_Image_Data *o = _pd;

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
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      eo_obj);
          }
     }
   else
     {
        if (!o->preloading)
          {
             o->preloading = EINA_TRUE;
             obj->layer->evas->engine.func->image_data_preload_request(obj->layer->evas->engine.data.output,
                                                                       o->engine_data,
                                                                       eo_obj);
          }
     }
}

EOLIAN static void
_evas_image_preload_begin(Eo *eo_obj, Evas_Image_Data *_pd EINA_UNUSED)
{
   _image_preload_internal(eo_obj, _pd, EINA_FALSE);
}

EOLIAN static void
_evas_image_preload_cancel(Eo *eo_obj, Evas_Image_Data *_pd EINA_UNUSED)
{
   _image_preload_internal(eo_obj, _pd, EINA_TRUE);
}

EOLIAN static void
_evas_image_data_copy_set(Eo *eo_obj, Evas_Image_Data *o, void *data)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (!data) return;
   _evas_object_image_cleanup(eo_obj, obj, o);
   if ((o->cur->image.w <= 0) ||
       (o->cur->image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                               o->engine_data);
   o->engine_data =
     obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
                                                               o->cur->image.w,
                                                               o->cur->image.h,
                                                               data,
                                                               o->cur->has_alpha,
                                                               o->cur->cspace);
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data =
          obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
                                                         o->engine_data,
                                                         o->cur->has_alpha);
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
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
   EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(o);
}

EOLIAN static void
_evas_image_data_update_add(Eo *eo_obj, Evas_Image_Data *o, int x, int y, int w, int h)
{
   Eina_Rectangle *r;
   int cnt;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur->image.w, o->cur->image.h);
   if ((w <= 0)  || (h <= 0)) return;
   if (!o->written) return;
   cnt = eina_list_count(o->pixels->pixel_updates);
   if (cnt == 1)
     { // detect single blob case already there to do a nop
        if ((r = o->pixels->pixel_updates->data))
          { // already a single full rect there.
             if ((r->x == 0) && (r->y == 0) && (r->w == o->cur->image.w) && (r->h == o->cur->image.h))
               return;
          }
     }
   if (cnt >= 512)
     { // too many update rects - just make a single blob update
        EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
          {
             EINA_LIST_FREE(pixi_write->pixel_updates, r) eina_rectangle_free(r);
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_image_alpha_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool has_alpha)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if (((has_alpha) && (o->cur->has_alpha)) ||
       ((!has_alpha) && (!o->cur->has_alpha)))
     return;
   if (o->cur->has_alpha != has_alpha)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          state_write->has_alpha = has_alpha;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data =
          obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
                                                         o->engine_data,
                                                         o->cur->has_alpha);
        if (obj->layer->evas->engine.func->image_scale_hint_set)
           obj->layer->evas->engine.func->image_scale_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
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
   evas_object_image_data_update_add(eo_obj, 0, 0, o->cur->image.w, o->cur->image.h);
   EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(o);
}

EOLIAN static Eina_Bool
_evas_image_alpha_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->has_alpha;
}

EOLIAN static void
_evas_image_smooth_scale_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool smooth_scale)
{
   Evas_Object_Protected_Data *obj;

   if (((smooth_scale) && (o->cur->smooth_scale)) ||
       ((!smooth_scale) && (!o->cur->smooth_scale)))
     return;
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->smooth_scale = smooth_scale;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_evas_image_smooth_scale_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->smooth_scale;
}

EOLIAN static void
_evas_image_reload(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if ((!o->cur->u.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur->image.w, o->cur->image.h);
   o->written = EINA_FALSE;
   evas_object_image_unload(eo_obj, 1);
   evas_object_inform_call_image_unloaded(eo_obj);
   evas_object_image_load(eo_obj, obj, o);

   EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, prev_write)
     {
        prev_write->u.file = NULL;
        prev_write->key = NULL;
     }
   EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, prev_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_evas_image_save(Eo *eo_obj, Evas_Image_Data *o, const char *file, const char *key, const char *flags)
{
   DATA32 *data = NULL;
   int quality = 80, compress = 9, ok = 0;
   char *encoding = NULL;
   RGBA_Image *im;
   if (!o->engine_data) return 0;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output, o->engine_data, 0, &data, &o->load_error);
   if (flags)
     {
        char *p, *pp;
        char *tflags;

        tflags = alloca(strlen(flags) + 1);
        strcpy(tflags, flags);
        p = tflags;
        while (p)
          {
             pp = strchr(p, ' ');
             if (pp) *pp = 0;
             sscanf(p, "quality=%i", &quality);
             sscanf(p, "compress=%i", &compress);
             sscanf(p, "encoding=%ms", &encoding);
             if (pp) p = pp + 1;
             else break;
          }
     }
   im = (RGBA_Image*) evas_cache_image_data(evas_common_image_cache_get(),
                                            o->cur->image.w,
                                            o->cur->image.h,
                                            data,
                                            o->cur->has_alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   if (im)
     {
        if (o->cur->cspace == EVAS_COLORSPACE_ARGB8888)
          im->image.data = data;
        else
          im->image.data = evas_object_image_data_convert_internal(o,
                                                                   data,
                                                                   EVAS_COLORSPACE_ARGB8888);
        if (im->image.data)
          {
             ok = evas_common_save_image_to_file(im, file, key, quality, compress, encoding);

             if (o->cur->cspace != EVAS_COLORSPACE_ARGB8888)
               free(im->image.data);
          }

        evas_cache_image_drop(&im->cache_entry);
     }
   o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
                                                                  o->engine_data,
                                                                  data);

   free(encoding);
   return ok;
}

EOLIAN static Eina_Bool
_evas_image_pixels_import(Eo *eo_obj, Evas_Image_Data *o, Evas_Pixel_Import_Source *pixels)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   _evas_object_image_cleanup(eo_obj, obj, o);
   if ((pixels->w != o->cur->image.w) || (pixels->h != o->cur->image.h)) return 0;

   switch (pixels->format)
     {
#if 0
      case EVAS_PIXEL_FORMAT_ARGB32:
	  {
	     if (o->engine_data)
	       {
		  DATA32 *image_pixels = NULL;

		  o->engine_data =
		    obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  1,
								  &image_pixels,
                                                                  &o->load_error);
/* FIXME: need to actualyl support this */
/*		  memcpy(image_pixels, pixels->rows, o->cur->image.w * o->cur->image.h * 4);*/
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur->has_alpha);
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

                  o->engine_data =
                     obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output, o->engine_data, 1, &image_pixels,&o->load_error);
                  if (image_pixels)
                    evas_common_convert_yuv_420p_601_rgba((DATA8 **) pixels->rows, (DATA8 *) image_pixels, o->cur->image.w, o->cur->image.h);
                  if (o->engine_data)
                    o->engine_data =
                       obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output, o->engine_data, image_pixels);
                  if (o->engine_data)
                    o->engine_data =
                       obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output, o->engine_data, o->cur->has_alpha);
                  o->changed = EINA_TRUE;
                  evas_object_change(eo_obj, obj);
               }
          }
        break;
      default:
        return 0;
        break;
     }
   return 1;
}

EOLIAN static void
_evas_image_pixels_get_callback_set(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Evas_Object_Image_Pixels_Get_Cb func, void *data)
{
   EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
     {
        pixi_write->func.get_pixels = func;
        pixi_write->func.get_pixels_data = data;
     }
   EINA_COW_PIXEL_WRITE_END(o, pixi_write);
}

EOLIAN static void
_evas_image_pixels_dirty_set(Eo *eo_obj, Evas_Image_Data *o, Eina_Bool dirty)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (dirty) o->dirty_pixels = EINA_TRUE;
   else o->dirty_pixels = EINA_FALSE;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_evas_image_pixels_dirty_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (o->dirty_pixels ? 1 : 0);
}

EOLIAN static void
_evas_image_load_dpi_set(Eo *eo_obj, Evas_Image_Data *o, double dpi)
{
   if (dpi == o->load_opts->dpi) return;

   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     low->dpi = dpi;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EOLIAN static double
_evas_image_load_dpi_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->load_opts->dpi;
}

EOLIAN static void
_evas_image_load_size_set(Eo *eo_obj, Evas_Image_Data *o, int w, int h)
{
   if ((o->load_opts->w == w) && (o->load_opts->h == h)) return;

   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     {
        low->w = w;
        low->h = h;
     }
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
   o->proxyerror = 0;
}

EOLIAN static void
_evas_image_load_size_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, int *w, int *h)
{
   if (w) *w = o->load_opts->w;
   if (h) *h = o->load_opts->h;
}

EOLIAN static void
_evas_image_load_scale_down_set(Eo *eo_obj, Evas_Image_Data *o, int scale_down)
{
   if (o->load_opts->scale_down_by == scale_down) return;
   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     low->scale_down_by = scale_down;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);

   if (o->cur->u.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EOLIAN static int
_evas_image_load_scale_down_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->load_opts->scale_down_by;
}

EOLIAN static void
_evas_image_load_region_set(Eo *eo_obj, Evas_Image_Data *o, int x, int y, int w, int h)
{
   if ((o->load_opts->region.x == x) && (o->load_opts->region.y == y) &&
       (o->load_opts->region.w == w) && (o->load_opts->region.h == h)) return;

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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj, obj, o);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EOLIAN static void
_evas_image_load_region_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, int *x, int *y, int *w, int *h)
{
   if (x) *x = o->load_opts->region.x;
   if (y) *y = o->load_opts->region.y;
   if (w) *w = o->load_opts->region.w;
   if (h) *h = o->load_opts->region.h;
}

EOLIAN static void
_evas_image_load_orientation_set(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o, Eina_Bool enable)
{
   if (o->load_opts->orientation == !!enable) return;

   EINA_COW_LOAD_OPTS_WRITE_BEGIN(o, low)
     low->orientation = !!enable;
   EINA_COW_LOAD_OPTS_WRITE_END(o, low);
}

EOLIAN static Eina_Bool
_evas_image_load_orientation_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->load_opts->orientation;;
}

EOLIAN static void
_evas_image_colorspace_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Colorspace cspace)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   _evas_object_image_cleanup(eo_obj, obj, o);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->cspace = cspace;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->engine_data)
     obj->layer->evas->engine.func->image_colorspace_set(obj->layer->evas->engine.data.output, o->engine_data, cspace);
}

EOLIAN static Evas_Colorspace
_evas_image_colorspace_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->cur->cspace;
}

EOLIAN static void
_evas_image_video_surface_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Video_Surface *surf)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   _evas_object_image_cleanup(eo_obj, obj, o);
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

EOLIAN static const Evas_Video_Surface*
_evas_image_video_surface_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (!o->video_surface ? NULL : &o->pixels->video);
}

EOLIAN static void
_evas_image_video_surface_caps_set(Eo *eo_obj, Evas_Image_Data *o, unsigned int caps)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   _evas_object_image_cleanup(eo_obj, obj, o);

   if (caps == o->pixels->video_caps)
      return;

   EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
     pixi_write->video_caps = caps;
   EINA_COW_PIXEL_WRITE_END(o, pixi_write)
}

EOLIAN static unsigned int
_evas_image_video_surface_caps_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return (!o->video_surface ? 0 : o->pixels->video_caps);
}

static void
_on_image_native_surface_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *einfo EINA_UNUSED)
{
   evas_object_image_native_surface_set(obj, NULL);
}

EAPI void
evas_object_image_native_surface_set(Evas_Object *eo_obj, Evas_Native_Surface *surf)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   evas_object_event_callback_del_full
     (eo_obj, EVAS_CALLBACK_DEL, _on_image_native_surface_del, NULL);
   if (surf) // We need to unset native surf on del to remove shared hash refs
     evas_object_event_callback_add
     (eo_obj, EVAS_CALLBACK_DEL, _on_image_native_surface_del, NULL);
   eo_do(eo_obj, evas_obj_image_native_surface_set(surf));
}

EOLIAN static void
_evas_image_native_surface_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Native_Surface *surf)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   evas_render_rendering_wait(obj->layer->evas);

   _evas_object_image_cleanup(eo_obj, obj, o);
   if (!obj->layer->evas->engine.func->image_native_set) return;
   if ((surf) &&
       ((surf->version < 2) ||
        (surf->version > EVAS_NATIVE_SURFACE_VERSION))) return;
   o->engine_data = obj->layer->evas->engine.func->image_native_set(obj->layer->evas->engine.data.output, o->engine_data, surf);
}

EOLIAN static Evas_Native_Surface*
_evas_image_native_surface_get(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Native_Surface *surf;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   surf =
      (!obj->layer->evas->engine.func->image_native_get ? NULL :
         obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output, o->engine_data));

   return surf;
}

EOLIAN static void
_evas_image_scale_hint_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Image_Scale_Hint hint)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (o->scale_hint == hint) return;
   o->scale_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;

        if (obj->layer->evas->engine.func->image_scale_hint_set)
          obj->layer->evas->engine.func->image_scale_hint_set
             (obj->layer->evas->engine.data.output,
               o->engine_data, o->scale_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
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

EOLIAN static Evas_Image_Scale_Hint
_evas_image_scale_hint_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->scale_hint;;
}

EOLIAN static void
_evas_image_content_hint_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Image_Content_Hint hint)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (o->content_hint == hint) return;
   o->content_hint = hint;
   if (o->engine_data)
     {
        int stride = 0;

        if (obj->layer->evas->engine.func->image_content_hint_set)
           obj->layer->evas->engine.func->image_content_hint_set
           (obj->layer->evas->engine.data.output,
               o->engine_data, o->content_hint);
        if (obj->layer->evas->engine.func->image_stride_get)
           obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
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

EAPI void
evas_object_image_alpha_mask_set(Evas_Object *eo_obj, Eina_Bool ismask)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!ismask) return;
   /* Convert to A8 if not already */

   /* done */

}

#define FRAME_MAX 1024
EOLIAN static Evas_Image_Content_Hint
_evas_image_content_hint_get(Eo *eo_obj EINA_UNUSED, Evas_Image_Data *o)
{
   return o->content_hint;
}

EOLIAN static Eina_Bool
_evas_image_region_support_get(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   Eina_Bool region;
   region =  obj->layer->evas->engine.func->image_can_region_get(
      obj->layer->evas->engine.data.output,
      o->engine_data);

   return region;
}

/* animated feature */
EOLIAN static Eina_Bool
_evas_image_animated_get(Eo *eo_obj, Evas_Image_Data *o)
{
   Eina_Bool animated;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   animated =
      obj->layer->evas->engine.func->image_animated_get ?
      obj->layer->evas->engine.func->image_animated_get(obj->layer->evas->engine.data.output, o->engine_data) :
      EINA_FALSE;

   return animated;
}

EOLIAN static int
_evas_image_animated_frame_count_get(Eo *eo_obj, Evas_Image_Data *o)
{
   int frame_count;
   frame_count = -1;

   if (!evas_object_image_animated_get(eo_obj)) return frame_count;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (obj->layer->evas->engine.func->image_animated_frame_count_get)
     frame_count = obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);

   return frame_count;
}

EOLIAN static Evas_Image_Animated_Loop_Hint
_evas_image_animated_loop_type_get(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   Evas_Image_Animated_Loop_Hint hint;
   hint = EVAS_IMAGE_ANIMATED_HINT_NONE;
   if (!evas_object_image_animated_get(eo_obj)) return hint;

   if (obj->layer->evas->engine.func->image_animated_loop_type_get)
      hint = obj->layer->evas->engine.func->image_animated_loop_type_get(obj->layer->evas->engine.data.output, o->engine_data);

   return hint;
}

EOLIAN static int
_evas_image_animated_loop_count_get(Eo *eo_obj, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   int loop_count;
   loop_count = -1;
   if (!evas_object_image_animated_get(eo_obj)) return loop_count;

   loop_count =
      obj->layer->evas->engine.func->image_animated_loop_count_get ?
      obj->layer->evas->engine.func->image_animated_loop_count_get(obj->layer->evas->engine.data.output, o->engine_data) :
      -1;

   return loop_count;
}

EOLIAN static double
_evas_image_animated_frame_duration_get(Eo *eo_obj, Evas_Image_Data *o, int start_frame, int frame_num)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   int frame_count = 0;

   double frame_duration;
   frame_duration = -1;

   if (!obj->layer->evas->engine.func->image_animated_frame_count_get) return frame_duration;

   frame_count = obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);

   if ((start_frame + frame_num) > frame_count) return frame_duration;
   if (obj->layer->evas->engine.func->image_animated_frame_duration_get)
     frame_duration = obj->layer->evas->engine.func->image_animated_frame_duration_get(obj->layer->evas->engine.data.output, o->engine_data, start_frame, frame_num);

   return frame_duration;
}

EOLIAN static void
_evas_image_animated_frame_set(Eo *eo_obj, Evas_Image_Data *o, int frame_index)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   int frame_count = 0;

   if (!o->cur->u.file) return;
   if (o->cur->frame == frame_index) return;

   if (!evas_object_image_animated_get(eo_obj)) return;

   frame_count = evas_object_image_animated_frame_count_get(eo_obj);

   /* limit the size of frame to FRAME_MAX */
   if ((frame_count > FRAME_MAX) || (frame_count < 0) || (frame_index > frame_count))
     return;

   if (!obj->layer->evas->engine.func->image_animated_frame_set) return;
   if (!obj->layer->evas->engine.func->image_animated_frame_set(obj->layer->evas->engine.data.output, o->engine_data, frame_index))
     return;

   EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, o->prev, Evas_Object_Image_State, prev_write)
     prev_write->frame = o->cur->frame;
   EINA_COW_WRITE_END(evas_object_image_state_cow, o->prev, prev_write);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->frame = frame_index;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);

}

EOLIAN void
_evas_canvas_image_cache_flush(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_render_rendering_wait(e);
   e->engine.func->image_cache_flush(e->engine.data.output);
}

EOLIAN void
_evas_canvas_image_cache_reload(Eo *eo_e, Evas_Public_Data *e)
{
   Evas_Layer *layer;

   evas_image_cache_flush(eo_e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object_Protected_Data *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
             if (eo_isa(obj->object, MY_CLASS))
               {
                  evas_object_image_unload(obj->object, 1);
                  evas_object_inform_call_image_unloaded(obj->object);
               }
	  }
     }
   evas_image_cache_flush(eo_e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object_Protected_Data *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
             if (eo_isa(obj->object, MY_CLASS))
               {
                  Evas_Image_Data *o = eo_data_scope_get(obj->object, MY_CLASS);
                  evas_object_image_load(obj->object, obj, o);
                  o->changed = EINA_TRUE;
                  evas_object_change(obj->object, obj);
               }
	  }
     }
   evas_image_cache_flush(eo_e);
}

EOLIAN void
_evas_canvas_image_cache_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int size)
{
   if (size < 0) size = 0;
   evas_render_rendering_wait(e);
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

EOLIAN int
_evas_canvas_image_cache_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->engine.func->image_cache_get(e->engine.data.output);
}

EOLIAN Eina_Bool
_evas_canvas_image_max_size_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int *maxw, int *maxh)
{
   int w = 0, h = 0;

   if (maxw) *maxw = 0xffff;
   if (maxh) *maxh = 0xffff;
   if (!e->engine.func->image_max_size_get) return EINA_FALSE;
   e->engine.func->image_max_size_get(e->engine.data.output, &w, &h);
   if (maxw) *maxw = w;
   if (maxh) *maxh = h;
   return EINA_TRUE;
}

/* all nice and private */
static void
_proxy_unset(Evas_Object *proxy, Evas_Object_Protected_Data *cur_proxy, Evas_Image_Data *o)
{
   Evas_Object_Protected_Data *cur_source;

   if (!o->cur->source) return;

   cur_source = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, cur_source->proxy, Evas_Object_Proxy_Data, proxy_source_write)
     {
       proxy_source_write->proxies = eina_list_remove(proxy_source_write->proxies,
						      proxy);

       if (eina_list_count(proxy_source_write->proxies) == 0)
          {
             if (proxy_source_write->surface)
               cur_proxy->layer->evas->engine.func->image_map_surface_free(cur_proxy->layer->evas->engine.data.output,
                                                                           proxy_source_write->surface);
             proxy_source_write->surface = NULL;
             proxy_source_write->redraw = EINA_FALSE;
          }

       if (proxy_source_write->src_invisible)
	 {
	   proxy_source_write->src_invisible = EINA_FALSE;
	   cur_source->changed_src_visible = EINA_TRUE;
	   evas_object_change(o->cur->source, cur_source);
	   evas_object_smart_member_cache_invalidate(o->cur->source, EINA_FALSE,
						     EINA_FALSE, EINA_TRUE);
	 }
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, cur_source->proxy, proxy_source_write);

   if (o->cur->source)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          state_write->source = NULL;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   if (o->cur->defmap)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             evas_map_free(state_write->defmap);
             state_write->defmap = NULL;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, cur_source->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->is_proxy = EINA_FALSE;
   EINA_COW_WRITE_END(evas_object_proxy_cow, cur_proxy->proxy, proxy_write);
}

static void
_proxy_set(Evas_Object *eo_proxy, Evas_Object *eo_src)
{
   Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJECT_CLASS);
   Evas_Object_Protected_Data *proxy = eo_data_scope_get(eo_proxy, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_proxy, MY_CLASS);

   evas_object_image_file_set(eo_proxy, NULL, NULL);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, proxy->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->is_proxy = EINA_TRUE;
   EINA_COW_WRITE_END(evas_object_proxy_cow, proxy->proxy, proxy_write);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     state_write->source = eo_src;
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->proxyerror = 0;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, src->proxy, Evas_Object_Proxy_Data, proxy_src_write)
     {
        if (!eina_list_data_find(proxy_src_write->proxies, eo_proxy))
          proxy_src_write->proxies = eina_list_append(proxy_src_write->proxies,
                                                      eo_proxy);
        proxy_src_write->redraw = EINA_TRUE;
     }
   EINA_COW_WRITE_END(evas_object_proxy_cow, src->proxy, proxy_src_write);
}

/* Some moron just set a proxy on a proxy.
 * Give them some pixels.  A random color
 */
static void
_proxy_error(Evas_Object *eo_proxy, void *context, void *output, void *surface,
             int x, int y, Eina_Bool do_async)
{
   Evas_Func *func;
   Evas_Image_Data *o = eo_data_scope_get(eo_proxy, MY_CLASS);

   if (!o->proxyerror) printf("Err: Argh! Recursive proxies.\n");
   o->proxyerror = 1;

   Evas_Object_Protected_Data *proxy = eo_data_scope_get(eo_proxy, EVAS_OBJECT_CLASS);
   func = proxy->layer->evas->engine.func;
   func->context_color_set(output, context, 0, 0, 0, 255);
   func->context_multiplier_unset(output, context);
   func->context_render_op_set(output, context, proxy->cur->render_op);
   func->rectangle_draw(output, context, surface, proxy->cur->geometry.x + x,
                        proxy->cur->geometry.y + y,
                        proxy->cur->geometry.w,
                        proxy->cur->geometry.h,
                        do_async);
   return;
}


/**
 * Render the source object when a proxy is set.
 *
 * Used to force a draw if necessary, else just makes sure it's available.
 */
static void
_proxy_subrender(Evas *eo_e, Evas_Object *eo_source, Evas_Object *eo_proxy, Evas_Object_Protected_Data *proxy_obj, Eina_Bool do_async)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Evas_Object_Protected_Data *source;
   void *ctx;
   int w, h;

   if (!eo_source) return;
   source = eo_data_scope_get(eo_source, EVAS_OBJECT_CLASS);

   w = source->cur->geometry.w;
   h = source->cur->geometry.h;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
        proxy_write->redraw = EINA_FALSE;

        /* We need to redraw surface then */
        if ((proxy_write->surface) &&
            ((proxy_write->w != w) || (proxy_write->h != h)))
          {
             e->engine.func->image_map_surface_free(e->engine.data.output,
                                                    proxy_write->surface);
             proxy_write->surface = NULL;
          }

        /* FIXME: Hardcoded alpha 'on' */
        /* FIXME (cont): Should see if the object has alpha */
        if (!proxy_write->surface)
          {
             proxy_write->surface = e->engine.func->image_map_surface_new
               (e->engine.data.output, w, h, 1);
             if (!proxy_write->surface) goto end;
             proxy_write->w = w;
             proxy_write->h = h;
          }

        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0,
                                          0, 0);
        e->engine.func->context_render_op_set(e->engine.data.output, ctx,
                                              EVAS_RENDER_COPY);
        e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                       proxy_write->surface, 0, 0, w, h,
                                       do_async);
        e->engine.func->context_free(e->engine.data.output, ctx);

        ctx = e->engine.func->context_new(e->engine.data.output);

        Eina_Bool source_clip = EINA_FALSE;
        eo_do(eo_proxy, source_clip = evas_obj_image_source_clip_get());

        Evas_Proxy_Render_Data proxy_render_data = {
             .eo_proxy = eo_proxy,
             .proxy_obj = proxy_obj,
             .eo_src = eo_source,
             .source_clip = source_clip
        };
        evas_render_mapped(e, eo_source, source, ctx, proxy_write->surface,
                           -source->cur->geometry.x,
                           -source->cur->geometry.y,
                           1, 0, 0, e->output.w, e->output.h,
                           &proxy_render_data
#ifdef REND_DBG
                           , 1
#endif
                           , do_async);

        e->engine.func->context_free(e->engine.data.output, ctx);
        proxy_write->surface = e->engine.func->image_dirty_region
           (e->engine.data.output, proxy_write->surface, 0, 0, w, h);
     }
 end:
   EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write);
}

static void
_3d_set(Evas_Object *eo_obj, Evas_3D_Scene *scene)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   Evas_3D_Scene_Data *pd_scene = eo_data_scope_get(scene,
                                                    EVAS_3D_SCENE_CLASS);
   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        data->surface = NULL;
        data->w = 0;
        data->h = 0;
        eo_ref(scene);
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->scene = scene;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   pd_scene->images = eina_list_append(pd_scene->images, eo_obj);
}

static void
_3d_unset(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj,
          Evas_Image_Data *o)
{
   Evas_Public_Data *e;

   if (!o->cur->scene) return;

   Evas_3D_Scene_Data *pd_scene =
      eo_data_scope_get(o->cur->scene, EVAS_3D_SCENE_CLASS);

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        pd_scene->images = eina_list_remove(pd_scene->images, eo_obj);
        eo_unref(state_write->scene);
        state_write->scene = NULL;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   if (o->cur->defmap)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             evas_map_free(state_write->defmap);
             state_write->defmap = NULL;
          }
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);
     }

   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        e = obj->layer->evas;

        if (data->surface)
          e->engine.func->image_free(e->engine.data.output, data->surface);

        data->surface = NULL;
        data->w = 0;
        data->h = 0;
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);
}

static void
_3d_render(Evas *eo_e, Evas_Object *eo_obj EINA_UNUSED,
           Evas_Object_Protected_Data *obj, Evas_Image_Data *o EINA_UNUSED,
           Evas_3D_Scene *scene)
{
   Evas_Public_Data *e;
   Eina_Bool need_native_set = EINA_FALSE;
   Evas_3D_Scene_Public_Data scene_data;
   Evas_3D_Scene_Data *pd_scene = NULL;

   pd_scene = eo_data_scope_get(scene, EVAS_3D_SCENE_CLASS);

   if ((pd_scene->w == 0) || (pd_scene->h == 0)) return;
   if (!pd_scene->camera_node)
     {
        WRN("Camera has not been set to scene(%p)", scene);
        return;
     }

   e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (pd_scene->surface)
     {
        int w = 0;
        int h = 0;

        if (e->engine.func->drawable_size_get)
          {
             e->engine.func->drawable_size_get(e->engine.data.output,
                                               pd_scene->surface, &w, &h);
          }
        if ((w != pd_scene->w) || (h != pd_scene->h))
          {
             if (e->engine.func->drawable_free)
               {
                  e->engine.func->drawable_free(e->engine.data.output,
                                                pd_scene->surface);
               }
             pd_scene->surface = NULL;
             need_native_set = EINA_TRUE;
          }
     }
   else
     {
        /* TODO: Hard-coded alpha on. */
        if (e->engine.func->drawable_new)
          {
             pd_scene->surface =
                e->engine.func->drawable_new(e->engine.data.output,
                                             pd_scene->w, pd_scene->h, 1);
          }
        need_native_set = EINA_TRUE;
     }

   EINA_COW_WRITE_BEGIN(evas_object_3d_cow, obj->data_3d, Evas_Object_3D_Data,
                        data)
     {
        if (need_native_set)
          {
             if (e->engine.func->image_drawable_set)
               {
                  data->surface =
                     e->engine.func->image_drawable_set(e->engine.data.output,
                                                        data->surface,
                                                        pd_scene->surface);
               }
          }
        data->w = pd_scene->w;
        data->h = pd_scene->h;
     }
   EINA_COW_WRITE_END(evas_object_3d_cow, obj->data_3d, data);

   evas_3d_scene_data_init(&scene_data);

   scene_data.bg_color = pd_scene->bg_color;
   scene_data.camera_node = pd_scene->camera_node;

   /* Phase 1 - Update scene graph tree. */
   eo_do(scene, evas_3d_object_update());

   /* Phase 2 - Do frustum culling and get visible model nodes. */
   evas_3d_node_tree_traverse(pd_scene->root_node,
                              EVAS_3D_TREE_TRAVERSE_LEVEL_ORDER, EINA_TRUE,
                              evas_3d_node_mesh_collect, &scene_data);

   /* Phase 3 - Collect active light nodes in the scene graph tree. */
   evas_3d_node_tree_traverse(pd_scene->root_node,
                              EVAS_3D_TREE_TRAVERSE_ANY_ORDER, EINA_FALSE,
                              evas_3d_node_light_collect, &scene_data);

   /* Phase 5 - Draw the scene. */
   if (e->engine.func->drawable_scene_render)
     {
        e->engine.func->drawable_scene_render(e->engine.data.output,
                                              pd_scene->surface, &scene_data);
     }
   /* Clean up temporary resources. */
   evas_3d_scene_data_fini(&scene_data);
}

static void
evas_object_image_unload(Evas_Object *eo_obj, Eina_Bool dirty)
{
   Evas_Image_Data *o;
   Eina_Bool resize_call = EINA_FALSE;

   o = eo_data_scope_get(eo_obj, MY_CLASS);
   if ((!o->cur->u.file) ||
       (o->pixels_checked_out > 0)) return;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (dirty)
     {
        if (o->engine_data)
           o->engine_data = obj->layer->evas->engine.func->image_dirty_region
           (obj->layer->evas->engine.data.output,
               o->engine_data,
               0, 0,
               o->cur->image.w, o->cur->image.h);
     }
   if (o->engine_data)
     {
        if (o->preloading)
          {
             o->preloading = EINA_FALSE;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      eo_obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   o->engine_data = NULL;
   o->load_error = EVAS_LOAD_ERROR_NONE;

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

static void
evas_object_image_load(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o)
{
   Evas_Image_Load_Opts lo;

   if (o->engine_data) return;

   lo.scale_down_by = o->load_opts->scale_down_by;
   lo.dpi = o->load_opts->dpi;
   lo.w = o->load_opts->w;
   lo.h = o->load_opts->h;
   lo.region.x = o->load_opts->region.x;
   lo.region.y = o->load_opts->region.y;
   lo.region.w = o->load_opts->region.w;
   lo.region.h = o->load_opts->region.h;
   lo.scale_load.src_x = o->load_opts->scale_load.src_x;
   lo.scale_load.src_y = o->load_opts->scale_load.src_y;
   lo.scale_load.src_w = o->load_opts->scale_load.src_w;
   lo.scale_load.src_h = o->load_opts->scale_load.src_h;
   lo.scale_load.dst_w = o->load_opts->scale_load.dst_w;
   lo.scale_load.dst_h = o->load_opts->scale_load.dst_h;
   lo.scale_load.smooth = o->load_opts->scale_load.smooth;
   lo.scale_load.scale_hint = o->load_opts->scale_load.scale_hint;
   lo.orientation = o->load_opts->orientation;
   lo.degree = 0;
   if (o->cur->mmaped_source)
     o->engine_data = obj->layer->evas->engine.func->image_mmap
       (obj->layer->evas->engine.data.output,
        o->cur->u.f,
        o->cur->key,
        &o->load_error,
        &lo);
   else
     o->engine_data = obj->layer->evas->engine.func->image_load
       (obj->layer->evas->engine.data.output,
        o->cur->u.file,
        o->cur->key,
        &o->load_error,
        &lo);

   if (o->engine_data)
     {
        int w, h;
        int stride = 0;
        Eina_Bool resize_call = EINA_FALSE;

        obj->layer->evas->engine.func->image_size_get
           (obj->layer->evas->engine.data.output,
            o->engine_data, &w, &h);
        if (obj->layer->evas->engine.func->image_stride_get)
          obj->layer->evas->engine.func->image_stride_get
             (obj->layer->evas->engine.data.output,
              o->engine_data, &stride);
        else
          stride = w * 4;

        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          {
             state_write->has_alpha = obj->layer->evas->engine.func->image_alpha_get
               (obj->layer->evas->engine.data.output,
                o->engine_data);
             state_write->cspace = obj->layer->evas->engine.func->image_colorspace_get
               (obj->layer->evas->engine.data.output,
                o->engine_data);
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
        o->load_error = EVAS_LOAD_ERROR_GENERIC;
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
        while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
        while (start < 0) start += size;
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
        while (start - size > 0) start -= size;
     }
   else if (start < 0)
     {
        while (start < 0) start += size;
     }
   start = ((start * obj->layer->evas->output.h) /
            (Evas_Coord)obj->layer->evas->viewport.h);
   *size_ret = h;
   return start;
}

static void
evas_object_image_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;
}

EOLIAN static void
_evas_image_eo_base_destructor(Eo *eo_obj, Evas_Image_Data *_pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   evas_object_image_video_surface_set(eo_obj, NULL);
   evas_object_image_free(eo_obj, obj);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

void
_evas_object_image_free(Evas_Object *obj)
{
   Evas_Image_Data *o;

   if (!eo_isa(obj, MY_CLASS)) return;

   o = eo_data_scope_get(obj, MY_CLASS);

   // eina_cow_free reset the pointer to the default read only state
   eina_cow_free(evas_object_image_load_opts_cow, (const Eina_Cow_Data**) &o->load_opts);
   eina_cow_free(evas_object_image_pixels_cow, (const Eina_Cow_Data**) &o->pixels);
   eina_cow_free(evas_object_image_state_cow, (const Eina_Cow_Data**) &o->cur);
   eina_cow_free(evas_object_image_state_cow, (const Eina_Cow_Data**) &o->prev);
}

static void
evas_object_image_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;

   /* free obj */
   if (!o->cur->mmaped_source)
     {
        if (o->cur->u.file) eina_stringshare_del(o->cur->u.file);
     }
   else
     {
        if (o->cur->u.f) eina_file_close(o->cur->u.f);
     }
   if (o->cur->key) eina_stringshare_del(o->cur->key);
   if (o->cur->source) _proxy_unset(eo_obj, obj, o);
   if (o->cur->scene) _3d_unset(eo_obj, obj, o);
   if (obj->layer && obj->layer->evas)
     {
       if (o->engine_data)
	 {
	   if (o->preloading)
	     {
	       o->preloading = EINA_FALSE;
	       obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
									o->engine_data,
									eo_obj);
	     }
	   obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
						     o->engine_data);
	 }
       if (o->video_surface)
	 {
	   o->video_surface = EINA_FALSE;
	   obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, eo_obj);
	 }
     }
   o->engine_data = NULL;
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
            void *data, void *context, void *surface, void *image,
            int src_x, int src_y, int src_w, int src_h, int dst_x,
            int dst_y, int dst_w, int dst_h, int smooth,
            Eina_Bool do_async)
{
   Eina_Bool async_unref;

   async_unref = obj->layer->evas->engine.func->image_draw(data, context, surface,
                                                           image, src_x, src_y,
                                                           src_w, src_h, dst_x,
                                                           dst_y, dst_w, dst_h,
                                                           smooth, do_async);
   if (do_async && async_unref)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_ref((Image_Entry *)image);
        else
#endif
          evas_cache_image_ref((Image_Entry *)image);

        evas_unref_queue_image_put(obj->layer->evas, image);
     }
}

void
evas_draw_image_map_async_check(Evas_Object_Protected_Data *obj,
                                void *data, void *context, void *surface,
                                void *image, RGBA_Map *m, int smooth, int level,
                                Eina_Bool do_async)
{
   Eina_Bool async_unref;

   async_unref = obj->layer->evas->engine.func->image_map_draw(data, context,
                                                               surface, image, m,
                                                               smooth, level,
                                                               do_async);
   if (do_async && async_unref)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_ref((Image_Entry *)image);
        else
#endif
          evas_cache_image_ref((Image_Entry *)image);

        evas_unref_queue_image_put(obj->layer->evas, image);
     }
}

static void *
evas_process_dirty_pixels(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o,
			  void *output, void *pixels)
{
   int direct_override = 0;
   int direct_force_off = 0;

   if (o->dirty_pixels)
     {
        if (o->pixels->func.get_pixels)
          {
             Evas_Coord x, y, w, h;

             if (obj->layer->evas->engine.func->image_native_get)
               {
                  Evas_Native_Surface *ns;
                  ns = obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output, o->engine_data);
                  if ( (ns) &&
                       (ns->type == EVAS_NATIVE_SURFACE_OPENGL) &&
                       (ns->data.opengl.texture_id) &&
                       (!ns->data.opengl.framebuffer_id) )
                    {
                       // Check if we can do direct rendering...
                       if (obj->layer->evas->engine.func->gl_direct_override_get)
                         obj->layer->evas->engine.func->gl_direct_override_get(output, &direct_override, &direct_force_off);
                       if ( (((obj->cur->geometry.w == o->cur->image.w) &&
                              (obj->cur->geometry.h == o->cur->image.h) &&
                              (obj->cur->color.r == 255) &&
                              (obj->cur->color.g == 255) &&
                              (obj->cur->color.b == 255) &&
                              (obj->cur->color.a == 255) &&
                              (!obj->map->cur.map) &&
                              (!o->cur->has_alpha)) ||
                             (direct_override)) &&
                            (!direct_force_off) )
                         {

                            if (obj->layer->evas->engine.func->gl_get_pixels_set)
                              {
                                 obj->layer->evas->engine.func->gl_get_pixels_set(output, o->pixels->func.get_pixels, o->pixels->func.get_pixels_data, eo_obj);
                              }

                            o->direct_render = EINA_TRUE;
                         }
                       else
                         o->direct_render = EINA_FALSE;

                    }

                  if ( (ns) &&
                       (ns->type == EVAS_NATIVE_SURFACE_X11))
                    {
                       if (obj->layer->evas->engine.func->context_flush)
                         obj->layer->evas->engine.func->context_flush(output);
                    }
               }

             x = obj->cur->geometry.x;
             y = obj->cur->geometry.y;
             w = obj->cur->geometry.w;
             h = obj->cur->geometry.h;

             if (!o->direct_render)
               o->pixels->func.get_pixels(o->pixels->func.get_pixels_data, eo_obj);

             if (!(obj->cur->geometry.x == x &&
                   obj->cur->geometry.y == y &&
                   obj->cur->geometry.w == w &&
                   obj->cur->geometry.h == h))
               CRI("Evas_Image_Data geometry did change during pixels get callback !");

             o->engine_data = obj->layer->evas->engine.func->image_dirty_region
               (obj->layer->evas->engine.data.output, o->engine_data,
                0, 0, o->cur->image.w, o->cur->image.h);
	     if (o->engine_data != pixels)
	       pixels = o->engine_data;
          }
        o->dirty_pixels = EINA_FALSE;
     }
   else
     {
        // Check if the it's not dirty but it has direct rendering
        if (o->direct_render)
          {
             obj->layer->evas->engine.func->gl_get_pixels_set(output, o->pixels->func.get_pixels, o->pixels->func.get_pixels_data, eo_obj);
          }
     }

   return pixels;
}

static void
_filter_cb(Evas_Filter_Context *ctx, void *data, Eina_Bool success)
{
   Eo *eo_obj = data;

   // Destroy context as we won't reuse it.
   evas_filter_context_destroy(ctx);

   // Redraw image normally
   if (!success)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        Evas_Image_Data *o = (Evas_Image_Data *) obj->private_data;

        ERR("Filter failed at runtime!");
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          state_write->filter.invalid = EINA_TRUE;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)

        // Update object
        obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        o->changed = EINA_TRUE;
        evas_object_change(eo_obj, obj);
        evas_object_clip_dirty(eo_obj, obj);
        evas_object_coords_recalc(eo_obj, obj);
        evas_object_inform_call_resize(eo_obj);
     }
}

static void
evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *type_private_data,
			 void *output, void *context, void *surface, int x, int y, Eina_Bool do_async)
{
   Evas_Image_Data *o = type_private_data;
   int imagew, imageh, uvw, uvh;
   void *pixels;

   if ((o->cur->fill.w < 1) || (o->cur->fill.h < 1))
     return; /* no error message, already printed in pre_render */

   /* Proxy sanity */
   if (o->proxyrendering)
     {
        _proxy_error(eo_obj, context, output, surface, x, y, EINA_FALSE);
        return;
     }

   /* We are displaying the overlay */
   if (o->video_visible)
     {
        /* Create a transparent rectangle */
        obj->layer->evas->engine.func->context_color_set(output,
                                                         context,
                                                         0, 0, 0, 0);
        obj->layer->evas->engine.func->context_multiplier_unset(output,
                                                                context);
        obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                             EVAS_RENDER_COPY);
        obj->layer->evas->engine.func->rectangle_draw(output,
                                                      context,
                                                      surface,
                                                      obj->cur->geometry.x + x,
                                                      obj->cur->geometry.y + y,
                                                      obj->cur->geometry.w,
                                                      obj->cur->geometry.h,
                                                      do_async);

        return;
     }

   obj->layer->evas->engine.func->context_color_set(output,
                                                    context,
                                                    255, 255, 255, 255);

   if ((obj->cur->cache.clip.r == 255) &&
       (obj->cur->cache.clip.g == 255) &&
       (obj->cur->cache.clip.b == 255) &&
       (obj->cur->cache.clip.a == 255))
     {
        obj->layer->evas->engine.func->context_multiplier_unset(output,
                                                                context);
     }
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
                                                           context,
                                                           obj->cur->cache.clip.r,
                                                           obj->cur->cache.clip.g,
                                                           obj->cur->cache.clip.b,
                                                           obj->cur->cache.clip.a);

   obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                        obj->cur->render_op);

   // Clear out the pixel get stuff..
   if (obj->layer->evas->engine.func->gl_get_pixels_set)
     {
        obj->layer->evas->engine.func->gl_get_pixels_set(output, NULL, NULL, NULL);
     }

   Evas_Object_Protected_Data *source =
      (o->cur->source ?
       eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS):
       NULL);

   if (o->cur->scene)
     {
        _3d_render(obj->layer->evas->evas, eo_obj, obj, o, o->cur->scene);
        pixels = obj->data_3d->surface;
        imagew = obj->data_3d->w;
        imageh = obj->data_3d->h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (!o->cur->source)
     {
        pixels = evas_process_dirty_pixels(eo_obj, obj, o, output, o->engine_data);
        /* pixels = o->engine_data; */
        imagew = o->cur->image.w;
        imageh = o->cur->image.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->proxy->surface && !source->proxy->redraw)
     {
        pixels = source->proxy->surface;
        imagew = source->proxy->w;
        imageh = source->proxy->h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->type == o_type &&
            ((Evas_Image_Data *)eo_data_scope_get(o->cur->source, MY_CLASS))->engine_data)
     {
        Evas_Image_Data *oi;
        oi = eo_data_scope_get(o->cur->source, MY_CLASS);
        pixels = oi->engine_data;
        imagew = oi->cur->image.w;
        imageh = oi->cur->image.h;
        uvw = source->cur->geometry.w;
        uvh = source->cur->geometry.h;
     }
   else
     {
        o->proxyrendering = EINA_TRUE;
        _proxy_subrender(obj->layer->evas->evas, o->cur->source, eo_obj, obj,
                         EINA_FALSE);
        pixels = source->proxy->surface;
        imagew = source->proxy->w;
        imageh = source->proxy->h;
        uvw = imagew;
        uvh = imageh;
        o->proxyrendering = EINA_FALSE;
     }

   if (pixels)
     {
        Evas_Coord idw, idh, idx, idy;
        int l = 0, r = 0, t = 0, b = 0;
        int ix, iy, iw, ih;

        if ((obj->map->cur.map) && (obj->map->cur.map->count > 3) && (obj->map->cur.usemap))
          {
             evas_object_map_update(eo_obj, x, y, imagew, imageh, uvw, uvh);

             evas_draw_image_map_async_check(
                 obj, output, context, surface, pixels, obj->map->spans,
                 o->cur->smooth_scale | obj->map->cur.map->smooth, 0, do_async);
          }
        else
          {
             Evas_Filter_Context *filter = NULL;
             void * const surface_save = surface;
             void * const context_save = context;
             Eina_Bool input_stolen = EINA_FALSE;
             Eina_Bool clear = EINA_FALSE;
             int W, H, offx, offy;

start_draw:
             obj->layer->evas->engine.func->image_scale_hint_set(output,
                                                                 pixels,
                                                                 o->scale_hint);
             /* This is technically a bug here: If the value is recreated
              * (which is returned)it may be a new object, however exactly 0
              * of all the evas engines do this. */
             obj->layer->evas->engine.func->image_border_set(output, pixels,
                                                             o->cur->border.l, o->cur->border.r,
                                                             o->cur->border.t, o->cur->border.b);
             idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur->fill.x, o->cur->fill.w, &idw);
             idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur->fill.y, o->cur->fill.h, &idh);
             if (idw < 1) idw = 1;
             if (idh < 1) idh = 1;
             if (idx > 0) idx -= idw;
             if (idy > 0) idy -= idh;

             offx = obj->cur->geometry.x + x;
             offy = obj->cur->geometry.y + y;

             /* Evas Filters on Image Object. Yet another experimental piece of code.
              * Replace current context, offset and surface by filter-made targets.
              *
              * Mechanism:
              * 1. Create a new surface & context, matching the object geometry
              * 2. Render all borders & tiles into that surface
              * 3. Apply effect and draw this surface into the original target
              *
              * Notes:
              * This becomes VERY SLOW when the image OBJECT size grows... even if
              * the image itself is very small. But we can't really apply a proper blur
              * over one tile and repeat since blurs should overlap.
              *
              * Behaviour depends on the 'filled' flag:
              * - If filled, then scale down the image to accomodate the whole filter's padding.
              * - Otherwise, draw image in place and clip the filter's effects (eg. blur will be clipped out).
              */
             if (!o->cur->filter.invalid && o->cur->filter.code)
               {
                  Evas_Filter_Program *pgm = o->cur->filter.chain;;
                  Eina_Bool redraw = (o->changed || o->cur->filter.changed);
                  Eina_Bool ok;

                  evas_filter_program_padding_get(pgm, &l, &r, &t, &b);
                  W = obj->cur->geometry.w;
                  H = obj->cur->geometry.h;

                  if (!redraw && o->cur->filter.output)
                    {
                       if (o->cur->filter.sources && o->cur->filter.sources_count > 0)
                         {
                            Evas_Filter_Proxy_Binding *pb;
                            Evas_Object_Protected_Data *prxsource;
                            Eina_Iterator *iter;

                            iter = eina_hash_iterator_data_new(o->cur->filter.sources);
                            EINA_ITERATOR_FOREACH(iter, pb)
                              {
                                 prxsource = eo_data_scope_get(pb->eo_source, EVAS_OBJECT_CLASS);
                                 if (prxsource->changed)
                                   {
                                      redraw = EINA_TRUE;
                                      break;
                                   }
                              }
                            eina_iterator_free(iter);
                         }

                       if (!redraw)
                         {
                            // Render this image only
                            obj->layer->evas->engine.func->image_draw(output, context,
                                             surface, o->cur->filter.output,
                                             0, 0, W, H,
                                             offx, offy, W, H,
                                             EINA_TRUE,
                                             do_async);
                            return;
                         }
                    }

                  if (!pgm)
                    {
                       pgm = evas_filter_program_new("Image", EINA_FALSE);
                       evas_filter_program_source_set_all(pgm, o->cur->filter.sources);
                       ok = evas_filter_program_parse(pgm, o->cur->filter.code);
                       if (!ok) goto state_write;
                    }

                  filter = evas_filter_context_new(obj->layer->evas, do_async);
                  ok = evas_filter_context_program_use(filter, pgm);
                  if (!ok) goto state_write;

                  evas_filter_context_proxy_render_all(filter, eo_obj, EINA_FALSE);
                  ok = evas_filter_context_buffers_allocate_all(filter, W, H);
                  if (!ok) goto state_write;

                  if (obj->layer->evas->engine.func->gl_surface_read_pixels)
                    {
                       // GL case: Create new surface, draw to it (OpenGL) and call glReadPixels
                       surface = obj->layer->evas->engine.func->image_map_surface_new(output, W, H, EINA_TRUE);
                       input_stolen = EINA_FALSE;
                    }
                  else
                    {
                       // Software case: Directly draw into the filter's input buffer
                       surface = evas_filter_buffer_backing_steal(filter, EVAS_FILTER_BUFFER_INPUT_ID);
                       input_stolen = EINA_TRUE;
                    }

                  if (!surface)
                    {
                       surface = surface_save;
                       ok = EINA_FALSE;
                    }

state_write:
                  EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                    {
                       if (ok)
                         {
                            context = obj->layer->evas->engine.func->context_new(output);
                            clear = EINA_TRUE;
                            state_write->filter.chain = pgm;
                            if (!o->filled)
                              {
                                 offx = 0;
                                 offy = 0;
                              }
                            else
                              {
                                 offx = l;
                                 offy = t;
                              }
                         }
                       else
                         {
                            ERR("Failed to apply this filter");
                            evas_filter_context_destroy(filter);
                            evas_filter_program_del(pgm);
                            state_write->filter.invalid = EINA_TRUE;
                            state_write->filter.chain = NULL;
                            l = r = t = b = 0;
                            filter = NULL;
                         }
                    }
                  EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)

                    if (clear)
                      {
                         obj->layer->evas->engine.func->context_color_set(output, context, 0, 0, 0, 0);
                         obj->layer->evas->engine.func->context_render_op_set(output, context, EVAS_RENDER_COPY);
                         obj->layer->evas->engine.func->rectangle_draw(output, context, surface, 0, 0, W, H, EINA_FALSE);
                         obj->layer->evas->engine.func->context_color_set(output, context, 255, 255, 255, 255);
                         obj->layer->evas->engine.func->context_render_op_set(output, context, EVAS_RENDER_BLEND);
                      }
               }

             while ((int)idx < obj->cur->geometry.w)
               {
                  Evas_Coord ydy;
                  int dobreak_w = 0;

                  ydy = idy;
                  ix = idx;
                  if ((o->cur->fill.w == obj->cur->geometry.w) &&
                      (o->cur->fill.x == 0))
                    {
                       dobreak_w = 1;
                       iw = obj->cur->geometry.w;
                    }
                  else
                    iw = ((int)(idx + idw)) - ix;

                  // Filter stuff
                  if (o->filled)
                    {
                       iw -= l + r;
                       if (iw <= 0) break;
                    }

                  while ((int)idy < obj->cur->geometry.h)
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
                         ih = ((int)(idy + idh)) - iy;

                       // Filter stuff
                       if (o->filled)
                         {
                            ih -= t + b;
                            if (ih <= 0) break;
                         }

                       if ((o->cur->border.l == 0) &&
                           (o->cur->border.r == 0) &&
                           (o->cur->border.t == 0) &&
                           (o->cur->border.b == 0) &&
                           (o->cur->border.fill != 0))
                         {
                            _draw_image
                              (obj, output, context, surface, pixels,
                               0, 0,
                               imagew, imageh,
                               offx + ix,
                               offy + iy,
                               iw, ih,
                               o->cur->smooth_scale,
                               do_async);
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
                            if ((bl + br) > imw)
                              {
                                 if ((bl + br) > 0)
                                   {
                                      bl = (bl * imw) / (bl + br);
                                      br = imw - bl;
                                   }
                              }
                            if ((bt + bb) > imh)
                              {
                                 if ((bt + bb) > 0)
                                   {
                                      bt = (bt * imh) / (bt + bb);
                                      bb = imh - bt;
                                   }
                              }
                            if (o->cur->border.scale != 1.0)
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
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .##.
                            // |  |
                            // '--'
                            inx = bl; iny = 0;
                            inw = imw - bl - br; inh = bt;
                            outx = ox + bsl; outy = oy;
                            outw = iw - bsl - bsr; outh = bst;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .--#
                            // |  |
                            // '--'
                            inx = imw - br; iny = 0;
                            inw = br; inh = bt;
                            outx = ox + iw - bsr; outy = oy;
                            outw = bsr; outh = bst;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .--.
                            // #  |
                            // '--'
                            inx = 0; iny = bt;
                            inw = bl; inh = imh - bt - bb;
                            outx = ox; outy = oy + bst;
                            outw = bsl; outh = ih - bst - bsb;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
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
                                     (obj->cur->render_op == EVAS_RENDER_BLEND))
                                   {
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           EVAS_RENDER_COPY);
                                      _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           obj->cur->render_op);
                                   }
                                 else
                                   _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                              }
                            // .--.
                            // |  #
                            // '--'
                            inx = imw - br; iny = bt;
                            inw = br; inh = imh - bt - bb;
                            outx = ox + iw - bsr; outy = oy + bst;
                            outw = bsr; outh = ih - bst - bsb;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .--.
                            // |  |
                            // #--'
                            inx = 0; iny = imh - bb;
                            inw = bl; inh = bb;
                            outx = ox; outy = oy + ih - bsb;
                            outw = bsl; outh = bsb;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .--.
                            // |  |
                            // '##'
                            inx = bl; iny = imh - bb;
                            inw = imw - bl - br; inh = bb;
                            outx = ox + bsl; outy = oy + ih - bsb;
                            outw = iw - bsl - bsr; outh = bsb;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                            // .--.
                            // |  |
                            // '--#
                            inx = imw - br; iny = imh - bb;
                            inw = br; inh = bb;
                            outx = ox + iw - bsr; outy = oy + ih - bsb;
                            outw = bsr; outh = bsb;
                            _draw_image(obj, output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur->smooth_scale, do_async);
                         }
                       idy += idh;
                       if (dobreak_h) break;
                    }
                  idx += idw;
                  idy = ydy;
                  if (dobreak_w) break;
               }

             /* Evas filters wrap-up */
             if (filter)
               {
                  Eina_Bool ok;

                  void *outbuf = evas_filter_buffer_backing_steal(filter, EVAS_FILTER_BUFFER_OUTPUT_ID);
                  if (outbuf != o->cur->filter.output)
                    {
                       evas_filter_buffer_backing_release(filter, o->cur->filter.output);
                       EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                         state_write->filter.output = outbuf;
                       EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)
                    }

                  if (!input_stolen)
                    evas_filter_image_draw(filter, context, EVAS_FILTER_BUFFER_INPUT_ID, surface, do_async);
                  else
                    evas_filter_buffer_backing_release(filter, surface);

                  evas_filter_target_set(filter, context_save, surface_save, obj->cur->geometry.x + x, obj->cur->geometry.y + y);
                  evas_filter_context_post_run_callback_set(filter, _filter_cb, eo_obj);
                  ok = evas_filter_run(filter);

                  if (!input_stolen)
                    obj->layer->evas->engine.func->image_map_surface_free(output, surface);
                  obj->layer->evas->engine.func->context_free(output, context);

                  if (!ok)
                    {
                       ERR("Rendering failed");
                       EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                         state_write->filter.invalid = EINA_TRUE;
                       EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)
                       l = r = t = b = 0;
                       context = context_save;
                       surface = surface_save;
                       input_stolen = EINA_FALSE;
                       clear = EINA_FALSE;
                       filter = NULL;
                       goto start_draw;
                    }
                  else
                    {
                       EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
                         state_write->filter.changed = EINA_FALSE;
                       EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)
                    }
               }
          }
     }
}

static void
evas_object_image_render_pre(Evas_Object *eo_obj,
			     Evas_Object_Protected_Data *obj,
			     void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;
   int is_v = 0, was_v = 0;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   Evas_Public_Data *e = obj->layer->evas;

   if ((o->cur->fill.w < 1) || (o->cur->fill.h < 1))
     {
        ERR("%p has invalid fill size: %dx%d. Ignored",
            eo_obj, o->cur->fill.w, o->cur->fill.h);
        return;
     }

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
        Evas_Object_Protected_Data *source = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
        if (source->proxy->redraw || source->changed)
          {
             /* XXX: Do I need to sort out the map here? */
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             goto done;
          }
     }
   else if (o->cur->scene)
     {
        Evas_3D_Scene *scene = o->cur->scene;
        Eina_Bool dirty;

        eo_do(scene, dirty = evas_3d_object_dirty_get(EVAS_3D_STATE_ANY));
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
        if (!o->pixels->pixel_updates) goto done;
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
        if (!o->pixels->pixel_updates) goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixels->pixel_updates) goto done;
     }
   /* if it changed render op */
   if (obj->cur->render_op != obj->prev->render_op)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixels->pixel_updates) goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur->anti_alias != obj->prev->anti_alias)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixels->pixel_updates) goto done;
     }
   if (o->changed)
     {
        if (((o->cur->u.file) && (!o->prev->u.file)) ||
            ((!o->cur->u.file) && (o->prev->u.file)) ||
            ((o->cur->key) && (!o->prev->key)) ||
            ((!o->cur->key) && (o->prev->key))
           )
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
          }
        if ((o->cur->image.w != o->prev->image.w) ||
            (o->cur->image.h != o->prev->image.h) ||
            (o->cur->has_alpha != o->prev->has_alpha) ||
            (o->cur->cspace != o->prev->cspace) ||
            (o->cur->smooth_scale != o->prev->smooth_scale))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
          }
        if ((o->cur->border.l != o->prev->border.l) ||
            (o->cur->border.r != o->prev->border.r) ||
            (o->cur->border.t != o->prev->border.t) ||
            (o->cur->border.b != o->prev->border.b) ||
            (o->cur->border.fill != o->prev->border.fill) ||
            (o->cur->border.scale != o->prev->border.scale))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
          }
        if (o->dirty_pixels)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
          }
        if (o->cur->frame != o->prev->frame)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
          }

     }
   if (((obj->cur->geometry.x != obj->prev->geometry.x) ||
        (obj->cur->geometry.y != obj->prev->geometry.y) ||
        (obj->cur->geometry.w != obj->prev->geometry.w) ||
        (obj->cur->geometry.h != obj->prev->geometry.h))
      )
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixels->pixel_updates) goto done;
     }
   if (o->cur->filter.changed)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixels->pixel_updates) goto done;
     }
   if (o->changed)
     {
        if ((o->cur->fill.x != o->prev->fill.x) ||
            (o->cur->fill.y != o->prev->fill.y) ||
            (o->cur->fill.w != o->prev->fill.w) ||
            (o->cur->fill.h != o->prev->fill.h))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixels->pixel_updates) goto done;
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

                  EINA_COW_PIXEL_WRITE_BEGIN(o, pixi_write)
                    {
                       EINA_LIST_FREE(pixi_write->pixel_updates, rr)
                         {
                            Evas_Coord idw, idh, idx, idy;
                            int x, y, w, h;
                            e->engine.func->image_dirty_region(e->engine.data.output, o->engine_data, rr->x, rr->y, rr->w, rr->h);

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
                         }
                    }
                  EINA_COW_PIXEL_WRITE_END(o, pixi_write);
                  goto done;
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
                  e->engine.func->image_dirty_region(e->engine.data.output, o->engine_data, 0, 0, o->cur->image.w, o->cur->image.h);

                  evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj,
                                                      obj);
                  goto done;
               }
          }
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* aren't fully opaque and we are visible */
   if (evas_object_is_visible(eo_obj, obj) &&
       evas_object_is_opaque(eo_obj, obj))
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
        e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                                x + e->framespace.x,
                                                y + e->framespace.y,
                                                w, h);
     }
   done:
   evas_object_render_pre_effect_updates(&e->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *eo_obj,
			      Evas_Object_Protected_Data *obj EINA_UNUSED,
			      void *type_private_data)
{
   Evas_Image_Data *o = type_private_data;
   Eina_Rectangle *r;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);

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
   evas_object_cur_prev(eo_obj);
   eina_cow_memcpy(evas_object_image_state_cow, (const Eina_Cow_Data **) &o->prev, o->cur);
   /* FIXME: copy strings across */
}

static unsigned int evas_object_image_id_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static unsigned int evas_object_image_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static void *evas_object_image_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
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

        // FIXME: use proxy
        if (o->cur->source)
          {
             Evas_Object_Protected_Data *cur_source = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
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
             if (
                 ((m->points[0].x == m->points[3].x) &&
                     (m->points[1].x == m->points[2].x) &&
                     (m->points[0].y == m->points[1].y) &&
                     (m->points[2].y == m->points[3].y))
                 ||
                 ((m->points[0].x == m->points[1].x) &&
                     (m->points[2].x == m->points[3].x) &&
                     (m->points[0].y == m->points[3].y) &&
                     (m->points[1].y == m->points[2].y))
                )
               {
                  if ((m->points[0].x == obj->cur->geometry.x) &&
                      (m->points[0].y == obj->cur->geometry.y) &&
                      (m->points[2].x == (obj->cur->geometry.x + obj->cur->geometry.w)) &&
                      (m->points[2].y == (obj->cur->geometry.y + obj->cur->geometry.h)))
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
          return o->prev->opaque; /* FIXME: Should go poke at the object */
        if (o->prev->has_alpha)
          return o->prev->opaque;

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
             if (
                 ((m->points[0].x == m->points[3].x) &&
                     (m->points[1].x == m->points[2].x) &&
                     (m->points[0].y == m->points[1].y) &&
                     (m->points[2].y == m->points[3].y))
                 ||
                 ((m->points[0].x == m->points[1].x) &&
                     (m->points[2].x == m->points[3].x) &&
                     (m->points[0].y == m->points[3].y) &&
                     (m->points[1].y == m->points[2].y))
                )
               {
                  if ((m->points[0].x == obj->prev->geometry.x) &&
                      (m->points[0].y == obj->prev->geometry.y) &&
                      (m->points[2].x == (obj->prev->geometry.x + obj->prev->geometry.w)) &&
                      (m->points[2].y == (obj->prev->geometry.y + obj->prev->geometry.h)))
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
   int imagew, imageh, uvw, uvh;
   void *pixels;
   Evas_Func *eng = obj->layer->evas->engine.func;
   int is_inside = 0;

   /* the following code is similar to evas_object_image_render(), but doesn't
    * draw, just get the pixels so we can check the transparency.
    */
   Evas_Object_Protected_Data *source =
      (o->cur->source ?
       eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS):
       NULL);

   if (o->cur->scene)
     {
        _3d_render(obj->layer->evas->evas, eo_obj, obj, o, o->cur->scene);
        pixels = obj->data_3d->surface;
        imagew = obj->data_3d->w;
        imageh = obj->data_3d->h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (!o->cur->source)
     {
        pixels = o->engine_data;
        imagew = o->cur->image.w;
        imageh = o->cur->image.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->proxy->surface && !source->proxy->redraw)
     {
        pixels = source->proxy->surface;
        imagew = source->proxy->w;
        imageh = source->proxy->h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->type == o_type &&
            ((Evas_Image_Data *)eo_data_scope_get(o->cur->source, MY_CLASS))->engine_data)
     {
        Evas_Image_Data *oi;
        oi = eo_data_scope_get(o->cur->source, MY_CLASS);
        pixels = oi->engine_data;
        imagew = oi->cur->image.w;
        imageh = oi->cur->image.h;
        uvw = source->cur->geometry.w;
        uvh = source->cur->geometry.h;
     }
   else
     {
        o->proxyrendering = EINA_TRUE;
        _proxy_subrender(obj->layer->evas->evas, o->cur->source, eo_obj, obj,
                         EINA_FALSE);
        pixels = source->proxy->surface;
        imagew = source->proxy->w;
        imageh = source->proxy->h;
        uvw = imagew;
        uvh = imageh;
        o->proxyrendering = EINA_FALSE;
     }

   if (pixels)
     {
        Evas_Coord idw, idh, idx, idy;
        int ix, iy, iw, ih;

        /* TODO: not handling o->dirty_pixels && o->pixels->func.get_pixels,
         * should we handle it now or believe they were done in the last render?
         */
        if (o->dirty_pixels)
          {
             if (o->pixels->func.get_pixels)
               {
                  ERR("dirty_pixels && get_pixels not supported");
               }
          }

        /* TODO: not handling map, need to apply map to point */
        if ((obj->map->cur.map) && (obj->map->cur.map->count > 3) && (obj->map->cur.usemap))
          {
             evas_object_map_update(eo_obj, 0, 0, imagew, imageh, uvw, uvh);

             ERR("map not supported");
          }
        else
          {
             void *im;
             DATA32 *data = NULL;
             int err = 0;

             im = obj->layer->evas->engine.func->image_data_get
               (obj->layer->evas->engine.data.output, pixels, 0, &data, &err);
             if ((!im) || (!data) || (err))
               {
                  ERR("Couldn't get image pixels %p: im=%p, data=%p, err=%d",
                      pixels, im, data, err);
                  goto end;
               }

             idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur->fill.x, o->cur->fill.w, &idw);
             idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur->fill.y, o->cur->fill.h, &idh);
             if (idw < 1) idw = 1;
             if (idh < 1) idh = 1;
             if (idx > 0) idx -= idw;
             if (idy > 0) idy -= idh;
             while ((int)idx < obj->cur->geometry.w)
               {
                  Evas_Coord ydy;
                  int dobreak_w = 0;
                  ydy = idy;
                  ix = idx;
                  if ((o->cur->fill.w == obj->cur->geometry.w) &&
                      (o->cur->fill.x == 0))
                    {
                       dobreak_w = 1;
                       iw = obj->cur->geometry.w;
                    }
                  else
                    iw = ((int)(idx + idw)) - ix;
                  while ((int)idy < obj->cur->geometry.h)
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
                         ih = ((int)(idy + idh)) - iy;
                       if ((o->cur->border.l == 0) &&
                           (o->cur->border.r == 0) &&
                           (o->cur->border.t == 0) &&
                           (o->cur->border.b == 0) &&
                           (o->cur->border.fill != 0))
                         {
                            /* NOTE: render handles cserve2 here,
                             * we don't need to
                             */
                              {
                                 DATA8 alpha = 0;

                                 if (eng->pixel_alpha_get(pixels, px, py, &alpha,
                                                          0, 0,
                                                          imagew, imageh,
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
                         }
                       else
                         {
                            int inx, iny, inw, inh, outx, outy, outw, outh;
                            int bl, br, bt, bb, bsl, bsr, bst, bsb;
                            int imw, imh, ox, oy;
                            DATA8 alpha = 0;

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
                            if (o->cur->border.scale != 1.0)
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                                 if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
                            if (eng->pixel_alpha_get(pixels, px, py, &alpha,
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
          }
     }

 end:
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

   if (o->cur->border.scale == 1.0)
     {
        *x = obj->cur->geometry.x + o->cur->border.l;
        *y = obj->cur->geometry.y + o->cur->border.t;
        *w = obj->cur->geometry.w - (o->cur->border.l + o->cur->border.r);
        if (*w < 0) *w = 0;
        *h = obj->cur->geometry.h - (o->cur->border.t + o->cur->border.b);
        if (*h < 0) *h = 0;
     }
   else
     {
        *x = obj->cur->geometry.x + (o->cur->border.l * o->cur->border.scale);
        *y = obj->cur->geometry.y + (o->cur->border.t * o->cur->border.scale);
        *w = obj->cur->geometry.w - ((o->cur->border.l * o->cur->border.scale) + (o->cur->border.r * o->cur->border.scale));
        if (*w < 0) *w = 0;
        *h = obj->cur->geometry.h - ((o->cur->border.t * o->cur->border.scale) + (o->cur->border.b * o->cur->border.scale));
        if (*h < 0) *h = 0;
     }
   return 1;
}

static int
evas_object_image_can_map(Evas_Object *obj EINA_UNUSED)
{
   return 1;
}

static void *
evas_object_image_data_convert_internal(Evas_Image_Data *o, void *data, Evas_Colorspace to_cspace)
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
        default:
          WRN("unknow colorspace: %i\n", o->cur->cspace);
          break;
     }

   return out;
}

static void
evas_object_image_filled_resize_listener(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *einfo EINA_UNUSED)
{
   Evas_Coord w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_image_fill_set(obj, 0, 0, w, h);
}

Evas_Object *
_evas_object_image_source_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   return o->cur->source;
}

Eina_Bool
_evas_object_image_preloading_get(const Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   return o->preloading;
}

void
_evas_object_image_preloading_set(Evas_Object *eo_obj, Eina_Bool preloading)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   o->preloading = preloading;
}

void
_evas_object_image_preloading_check(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (obj->layer->evas->engine.func->image_load_error_get)
      o->load_error = obj->layer->evas->engine.func->image_load_error_get
      (obj->layer->evas->engine.data.output, o->engine_data);
}

Evas_Object *
_evas_object_image_video_parent_get(Evas_Object *eo_obj)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   return o->video_surface ? o->pixels->video.parent : NULL;
}

void
_evas_object_image_video_overlay_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
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

/* Evas Filters functions. Similar to Evas_Text. */

EOLIAN static void
_evas_image_filter_program_set(Eo *eo_obj, Evas_Image_Data *o, const char *arg)
{
   Evas_Object_Protected_Data *obj;
   Evas_Filter_Program *pgm = NULL;

   if (!o) return;
   if (o->cur->filter.code == arg) return;
   if (o->cur->filter.code && arg && !strcmp(arg, o->cur->filter.code)) return;

   // Parse filter program
   evas_filter_program_del(o->cur->filter.chain);
   if (arg)
     {
        pgm = evas_filter_program_new("Evas_Text: Filter Program", EINA_FALSE);
        evas_filter_program_source_set_all(pgm, o->cur->filter.sources);
        if (!evas_filter_program_parse(pgm, arg))
          {
             ERR("Parsing failed!");
             evas_filter_program_del(pgm);
             pgm = NULL;
          }
     }

   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->filter.chain = pgm;
        state_write->filter.changed = EINA_TRUE;
        state_write->filter.invalid = (pgm == NULL);
        eina_stringshare_replace(&state_write->filter.code, arg);
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   // Update object
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

static void
_filter_source_hash_free_cb(void *data)
{
   Evas_Filter_Proxy_Binding *pb = data;
   Evas_Object_Protected_Data *proxy, *source;
   Evas_Image_Data *o;

   proxy = eo_data_scope_get(pb->eo_proxy, EVAS_OBJECT_CLASS);
   source = eo_data_scope_get(pb->eo_source, EVAS_OBJECT_CLASS);

   if (source)
     {
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy,
                             Evas_Object_Proxy_Data, source_write)
          source_write->proxies = eina_list_remove(source_write->proxies, pb->eo_proxy);
        EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)
     }

   o = eo_data_scope_get(pb->eo_proxy, MY_CLASS);

   if (o && proxy)
     {
        EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
          state_write->filter.sources_count--;
        EINA_COW_IMAGE_STATE_WRITE_END(o, state_write)

        if (!o->cur->filter.sources_count)
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, proxy->proxy,
                                  Evas_Object_Proxy_Data, proxy_write)
               proxy_write->is_proxy = EINA_FALSE;
             EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write)
          }
     }

   eina_stringshare_del(pb->name);
   free(pb);
}

EOLIAN static void
_evas_image_filter_source_set(Eo *eo_obj, Evas_Image_Data *o, const char *name,
                              Evas_Object *eo_source)
{

   Evas_Object_Protected_Data *obj;
   Evas_Filter_Program *pgm = o->cur->filter.chain;
   Evas_Filter_Proxy_Binding *pb, *pb_old = NULL;
   Evas_Object_Protected_Data *source = NULL;
   Eina_Hash *sources = o->cur->filter.sources;
   int sources_count = o->cur->filter.sources_count;

   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (eo_source) source = eo_data_scope_get(eo_source, EVAS_OBJECT_CLASS);

   if (!name)
     {
        if (!eo_source || !o->cur->filter.sources) return;
        if (eina_hash_del_by_data(o->cur->filter.sources, eo_source))
          goto update;
        return;
     }

   if (!sources)
     {
        if (!source) return;
        sources = eina_hash_string_small_new
              (EINA_FREE_CB(_filter_source_hash_free_cb));
     }
   else
     {
        pb_old = eina_hash_find(sources, name);
        if (pb_old)
          {
             if (pb_old->eo_source == eo_source) goto update;
             eina_hash_del(sources, name, pb_old);
          }
     }

   if (!source)
     {
        pb_old = eina_hash_find(sources, name);
        if (!pb_old) return;
        eina_hash_del_by_key(sources, name);
        goto update;
     }

   pb = calloc(1, sizeof(*pb));
   pb->eo_proxy = eo_obj;
   pb->eo_source = eo_source;
   pb->name = eina_stringshare_add(name);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy,
                        Evas_Object_Proxy_Data, source_write)
     if (!eina_list_data_find(source_write->proxies, eo_obj))
       source_write->proxies = eina_list_append(source_write->proxies, eo_obj);
   EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy,
                        Evas_Object_Proxy_Data, proxy_write)
     proxy_write->is_proxy = EINA_TRUE;
   EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write)

   eina_hash_add(sources, pb->name, pb);
   sources_count++;

   evas_filter_program_source_set_all(pgm, sources);

update:
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(o, state_write)
     {
        state_write->filter.changed = EINA_TRUE;
        state_write->filter.invalid = EINA_FALSE;
        state_write->filter.sources = sources;
        state_write->filter.sources_count = sources_count;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(o, state_write);

   // Update object
   o->changed = 1;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj);
}

EOLIAN Eina_Bool
_evas_image_filter_padding_get(Eo *obj EINA_UNUSED, Evas_Image_Data *o,
                               int *l, int *r, int *t, int *b)
{
   Evas_Filter_Program *pgm = o->cur->filter.chain;
   int pl = 0, pr = 0, pt = 0, pb = 0;
   Eina_Bool used = EINA_FALSE;

   if (pgm)
     {
        used = !o->cur->filter.invalid;
        evas_filter_program_padding_get(pgm, &pl, &pr, &pt, &pb);
     }

   if (l) *l = pl;
   if (r) *r = pr;
   if (t) *t = pt;
   if (b) *b = pb;
   return used;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/

#include "canvas/evas_image.eo.c"
