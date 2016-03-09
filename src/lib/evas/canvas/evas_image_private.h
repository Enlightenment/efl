#ifndef EVAS_IMAGE_PRIVATE_H
#define EVAS_IMAGE_PRIVATE_H

/* Those functions are shared between legacy evas_object_image.c and the
 * new efl_canvas classes (image, snapshot, proxy, ...)
 */

#define EVAS_FILTER_PROTECTED
#define EVAS_OBJECT_PROTECTED

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

#include "evas_filter.eo.h"
#include "evas_filter.h"

/* private struct for rectangle object internal data */
typedef struct _Evas_Image_Data Evas_Image_Data;
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
   Evas_Canvas3D_Scene *scene;

   union {
      const char    *file;
      Eina_File     *f;
   } u;
   const char    *key;
   int            frame;

   Evas_Colorspace    cspace;
   Evas_Image_Orient  orient;

   Eina_Bool      smooth_scale : 1;
   Eina_Bool      has_alpha :1;
   Eina_Bool      opaque_valid : 1;
   Eina_Bool      opaque : 1;
   Eina_Bool      mmaped_source : 1;
};

struct _Evas_Image_Data
{
   const Evas_Object_Image_State *cur;
   const Evas_Object_Image_State *prev;
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
   Eina_Bool         filled_set : 1;
   Eina_Bool         proxyrendering : 1;
   Eina_Bool         preloading : 1;
   Eina_Bool         video_surface : 1;
   Eina_Bool         video_visible : 1;
   Eina_Bool         created : 1;
   Eina_Bool         proxyerror : 1;
   Eina_Bool         proxy_src_clip : 1;
   Eina_Bool         written : 1;
   Eina_Bool         direct_render : 1;
   Eina_Bool         has_filter : 1;
   struct
   {
      Eina_Bool      video_move : 1;
      Eina_Bool      video_resize : 1;
      Eina_Bool      video_show : 1;
      Eina_Bool      video_hide : 1;
   } delayed;
   Eina_Bool         legacy_type : 1;
};

/* shared functions between legacy and new eo classes */
void _evas_image_init_set(const Eina_File *f, const char *file, const char *key, Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o, Evas_Image_Load_Opts *lo);
void _evas_image_done_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void _evas_image_cleanup(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void _evas_image_proxy_unset(Evas_Object *proxy, Evas_Object_Protected_Data *obj, Evas_Image_Data *o);
void _evas_image_proxy_set(Evas_Object *proxy, Evas_Object *src);
void _evas_image_proxy_error(Evas_Object *proxy, void *context, void *output, void *surface, int x, int y, Eina_Bool do_async);
void _evas_image_3d_render(Evas *eo_e, Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *o, Evas_Canvas3D_Scene *scene);
void _evas_image_3d_set(Evas_Object *eo_obj, Evas_Canvas3D_Scene *scene);
void _evas_image_3d_unset(Evas_Object *eo_obj, Evas_Object_Protected_Data *image, Evas_Image_Data *o);
Eina_Bool _evas_image_native_surface_set(Eo *eo_obj, Evas_Native_Surface *surf);
Evas_Native_Surface *_evas_image_native_surface_get(const Evas_Object *eo_obj);

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

# define EINA_COW_IMAGE_STATE_WRITE_BEGIN(Obj, Write) \
  EINA_COW_WRITE_BEGIN(evas_object_image_state_cow, Obj->cur, Evas_Object_Image_State, Write)

# define EINA_COW_IMAGE_STATE_WRITE_END(Obj, Write) \
  EINA_COW_WRITE_END(evas_object_image_state_cow, Obj->cur, Write)

#endif // EVAS_IMAGE_PRIVATE_H
