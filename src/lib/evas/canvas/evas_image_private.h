#ifndef EVAS_IMAGE_PRIVATE_H
#define EVAS_IMAGE_PRIVATE_H

/* Those functions are shared between legacy evas_object_image.c and the
 * new efl_canvas classes (image, snapshot, proxy, ...)
 */

#define EFL_CANVAS_FILTER_INTERNAL_PROTECTED
#define EFL_CANVAS_OBJECT_PROTECTED

#include "evas_common_private.h"

#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <math.h>

#include "evas_private.h"
#ifdef EVAS_CSERVE2
#include "../cserve2/evas_cs2_private.h"
#endif
#include "../common/evas_convert_color.h"
#include "../common/evas_convert_colorspace.h"
#include "../common/evas_convert_yuv.h"

#include "canvas/evas_image.eo.h"
#include "canvas/efl_canvas_filter_internal.eo.h"
#include "evas_filter.h"

/* private struct for rectangle object internal data */
typedef struct _Evas_Image_Data Evas_Image_Data;
typedef struct _Evas_Image_Map Evas_Image_Map;
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
      /* FIXME: no good match for eo */
      Evas_Object_Image_Pixels_Get_Cb  get_pixels;
      void                            *get_pixels_data;
   } func;
   Eina_Hash       *images_to_free; /* pixel void* -> Evas_Image_Legacy_Pixels_Entry */

   Evas_Video_Surface video;
   unsigned int video_caps;
};

struct _Evas_Object_Image_State
{
   Eina_Rectangle   fill;
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
   Evas_Canvas3D_Scene *scene;

   Eina_File     *f;
   const char    *key;
   int            frame;

   Evas_Colorspace    cspace;
   Evas_Image_Orient  orient;

   Eina_Bool      smooth_scale : 1;
   Eina_Bool      has_alpha :1;
   Eina_Bool      opaque_valid : 1;
   Eina_Bool      opaque : 1;
};

struct _Evas_Image_Data
{
   const Evas_Object_Image_State *cur;
   const Evas_Object_Image_State *prev;
   const Evas_Object_Image_Load_Opts *load_opts;
   const Evas_Object_Image_Pixels *pixels;

   void             *engine_data;
   void             *engine_data_prep;

   void             *plane;

   int               pixels_checked_out;
   int               load_error;

   Efl_Gfx_Image_Scale_Hint   scale_hint;
   Efl_Gfx_Image_Content_Hint content_hint;
   Efl_Flip               flip_value;
   Efl_Orient             orient_value;

   struct {
      short          w, h;
   } file_size;

   Eina_Bool         changed : 1;
   Eina_Bool         dirty_pixels : 1;
   Eina_Bool         filled : 1;
   Eina_Bool         filled_set : 1;
   Eina_Bool         proxyrendering : 1;
   Eina_Bool         preloading : 1;      //on preloading
   Eina_Bool         preloaded: 1;        //just finsihed preloading
   Eina_Bool         video_surface : 1;
   Eina_Bool         video_visible : 1;
   Eina_Bool         created : 1;
   Eina_Bool         proxyerror : 1;
   Eina_Bool         proxy_src_clip : 1;
   Eina_Bool         written : 1;
   Eina_Bool         direct_render : 1;
   Eina_Bool         has_filter : 1;
   Eina_Bool         buffer_data_set : 1;
   struct
   {
      Eina_Bool      video_move : 1;
      Eina_Bool      video_resize : 1;
      Eina_Bool      video_show : 1;
      Eina_Bool      video_hide : 1;
   } delayed;
   Eina_Bool         legacy_type : 1;
   Eina_Bool         skip_head : 1;
   Eina_Bool         can_scanout : 1;
};

/* shared functions between legacy and new eo classes */
void _evas_image_init_set(const Eina_File *f, const char *key, Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o, Evas_Image_Load_Opts *lo);
void _evas_image_done_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void _evas_image_cleanup(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void *_evas_image_pixels_get(Eo *eo_obj, Evas_Object_Protected_Data *obj, void *engine, void *output, void *context, void *surface, int x, int y, int *imagew, int *imageh, int *uvw, int *uvh, Eina_Bool filtered, Eina_Bool needs_post_render);

/* Efl.Gfx.Fill */
void _evas_image_fill_set(Eo *eo_obj, Evas_Image_Data *o, int x, int y, int w, int h);

/* Efl.File */
Eina_Bool _evas_image_mmap_set(Eo *eo_obj, const Eina_File *f, const char *key);
void _evas_image_mmap_get(const Eo *eo_obj, const Eina_File **f, const char **key);
Eina_Bool _evas_image_file_set(Eo *eo_obj, const char *file, const char *key);
void _evas_image_file_get(const Eo *eo_obj, const char **file, const char **key);

/* Efl.Image.Load */
Efl_Gfx_Image_Load_Error _evas_image_load_error_get(const Eo *eo_obj);
void _evas_image_load_post_update(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
void _evas_image_load_async_start(Eo *eo_obj);
void _evas_image_load_async_cancel(Eo *eo_obj);
void _evas_image_load_dpi_set(Eo *eo_obj, double dpi);
double _evas_image_load_dpi_get(const Eo *eo_obj);
void _evas_image_load_size_set(Eo *eo_obj, int w, int h);
void _evas_image_load_size_get(const Eo *eo_obj, int *w, int *h);
void _evas_image_load_scale_down_set(Eo *eo_obj, int scale_down);
int _evas_image_load_scale_down_get(const Eo *eo_obj);
void _evas_image_load_region_set(Eo *eo_obj, int x, int y, int w, int h);
void _evas_image_load_region_get(const Eo *eo_obj, int *x, int *y, int *w, int *h);
void _evas_image_load_head_skip_set(const Eo *eo_obj, Eina_Bool skip);
Eina_Bool _evas_image_load_head_skip_get(const Eo *eo_obj);
void _evas_image_load_orientation_set(Eo *eo_obj, Eina_Bool enable);
Eina_Bool _evas_image_load_orientation_get(const Eo *eo_obj);
Eina_Bool _evas_image_load_region_support_get(const Eo *eo_obj);
void _evas_image_orientation_set(Eo *eo_obj, Evas_Image_Data *o, Evas_Image_Orient orient);

/* Efl.Image_Animation_Controller */
Eina_Bool _evas_image_animated_get(const Eo *eo_obj);
int _evas_image_animated_frame_count_get(const Eo *eo_obj);
Efl_Gfx_Image_Animation_Controller_Loop_Hint _evas_image_animated_loop_type_get(const Eo *eo_obj);
int _evas_image_animated_loop_count_get(const Eo *eo_obj);
double _evas_image_animated_frame_duration_get(const Eo *eo_obj, int start_frame, int frame_num);
Eina_Bool _evas_image_animated_frame_set(Eo *eo_obj, int frame_index);
int _evas_image_animated_frame_get(const Eo *eo_obj);

/* Efl.Canvas.Proxy */
void _evas_image_proxy_unset(Evas_Object *proxy, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void _evas_image_proxy_set(Evas_Object *proxy, Evas_Object *src);
void _evas_image_proxy_error(Evas_Object *proxy, void *engine, void *output, void *context, void *surface, int x, int y, Eina_Bool do_async);
Eina_Bool _evas_image_proxy_source_set(Eo *eo_obj, Evas_Object *eo_src);
Evas_Object *_evas_image_proxy_source_get(const Eo *eo_obj);
void _evas_image_proxy_source_clip_set(Eo *eo_obj, Eina_Bool source_clip);
Eina_Bool _evas_image_proxy_source_clip_get(const Eo *eo_obj);
void _evas_image_proxy_source_events_set(Eo *eo_obj, Eina_Bool source_events);
Eina_Bool _evas_image_proxy_source_events_get(const Eo *eo_obj);

/* Efl.Canvas.Scene3d */
void _evas_image_3d_render(Evas *eo_e, Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o, Evas_Canvas3D_Scene *scene, void *engine, void *output);
void _evas_image_3d_set(Evas_Object *eo_obj, Evas_Canvas3D_Scene *scene);
void _evas_image_3d_unset(Evas_Object *eo_obj, Evas_Object_Protected_Data *image, Evas_Image_Data *o);

/* Efl.Canvas.Surface */
Eina_Bool _evas_image_native_surface_set(Eo *eo_obj, Evas_Native_Surface *surf);
Evas_Native_Surface *_evas_image_native_surface_get(const Evas_Object *eo_obj);

/* deprecated but in use */
void *_evas_image_data_convert_internal(Evas_Image_Data *o, void *data, Evas_Colorspace to_cspace);
void _evas_image_unload(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool dirty);
void _evas_image_load(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);

# define EINA_COW_IMAGE_STATE_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, Obj->cur, Evas_Object_Image_State, Write)

# define EINA_COW_IMAGE_STATE_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_state_cow, Obj->cur, Write)

# define EINA_COW_PIXEL_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_pixels_cow, Obj->pixels, Evas_Object_Image_Pixels, Write)

# define EINA_COW_PIXEL_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_pixels_cow, Obj->pixels, Write)

# define EINA_COW_LOAD_OPTS_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_load_opts_cow, Obj->load_opts, Evas_Object_Image_Load_Opts, Write)

# define EINA_COW_LOAD_OPTS_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_load_opts_cow, Obj->load_opts, Write)

# define EVAS_OBJECT_WRITE_IMAGE_FREE_FILE_AND_KEY(Obj)                 \
  if (Obj->cur->key) \
    {                                                                   \
       EINA_COW_IMAGE_STATE_WRITE_BEGIN(Obj, cur_write)                 \
         {                                                              \
            EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, Obj->prev, Evas_Object_Image_State, prev_write) \
              EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(cur_write, prev_write); \
            EINA_COW_WRITE_END(evas_object_image_state_cow, Obj->prev, prev_write); \
         }                                                              \
       EINA_COW_IMAGE_STATE_WRITE_END(Obj, cur_write);                  \
    }

#define FRAME_MAX 1024

#endif // EVAS_IMAGE_PRIVATE_H
