#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>

#include "evas_common.h"
#include "evas_private.h"
#include "../engines/common/evas_convert_color.h"
#include "../engines/common/evas_convert_colorspace.h"
#include "../engines/common/evas_convert_yuv.h"

#define VERBOSE_PROXY_ERROR 1

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image      Evas_Object_Image;

struct _Evas_Object_Image
{
   DATA32            magic;

   struct {
      int                  spread;
      Evas_Coord_Rectangle fill;
      struct {
	 short         w, h, stride;
      } image;
      struct {
	 short         l, r, t, b;
	 unsigned char fill;
         double        scale;
      } border;

      Evas_Object   *source;
      Evas_Map      *defmap;
      const char    *file;
      const char    *key;
      int            frame;
      Evas_Colorspace cspace;

      unsigned char  smooth_scale : 1;
      unsigned char  has_alpha :1;
      unsigned char  opaque :1;
      unsigned char  opaque_valid :1;
   } cur, prev;

   int               pixels_checked_out;
   int               load_error;
   Eina_List        *pixel_updates;

   struct {
      unsigned char  scale_down_by;
      double         dpi;
      short          w, h;
      struct {
         short       x, y, w, h;
      } region;
      Eina_Bool  orientation : 1;
   } load_opts;

   struct {
      Evas_Object_Image_Pixels_Get_Cb  get_pixels;
      void                            *get_pixels_data;
   } func;

   Evas_Video_Surface video;

   const char             *tmpf;
   int                     tmpf_fd;

   Evas_Image_Scale_Hint   scale_hint;
   Evas_Image_Content_Hint content_hint;

   void             *engine_data;

   unsigned char     changed : 1;
   unsigned char     dirty_pixels : 1;
   unsigned char     filled : 1;
   unsigned char     proxyrendering : 1;
   unsigned char     preloading : 1;
   unsigned char     video_rendering : 1;
   unsigned char     video_surface : 1;
   unsigned char     video_visible : 1;
   unsigned char     created : 1;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *obj, Eina_Bool dirty);
static void evas_object_image_load(Evas_Object *obj);
static Evas_Coord evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void evas_object_image_init(Evas_Object *obj);
static void *evas_object_image_new(void);
static void evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_image_free(Evas_Object *obj);
static void evas_object_image_render_pre(Evas_Object *obj);
static void evas_object_image_render_post(Evas_Object *obj);

static unsigned int evas_object_image_id_get(Evas_Object *obj);
static unsigned int evas_object_image_visual_id_get(Evas_Object *obj);
static void *evas_object_image_engine_data_get(Evas_Object *obj);

static int evas_object_image_is_opaque(Evas_Object *obj);
static int evas_object_image_was_opaque(Evas_Object *obj);
static int evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static int evas_object_image_has_opaque_rect(Evas_Object *obj);
static int evas_object_image_get_opaque_rect(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
static int evas_object_image_can_map(Evas_Object *obj);

static void *evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace);
static void evas_object_image_filled_resize_listener(void *data, Evas *e, Evas_Object *obj, void *einfo);

static void _proxy_unset(Evas_Object *proxy);
static void _proxy_set(Evas_Object *proxy, Evas_Object *src);
static void _proxy_error(Evas_Object *proxy, void *context, void *output, void *surface, int x, int y);

static void _cleanup_tmpf(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_image_free,
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

EVAS_MEMPOOL(_mp_obj);

static void
_evas_object_image_cleanup(Evas_Object *obj, Evas_Object_Image *o)
{
   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = 0;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
     }
   if (o->tmpf) _cleanup_tmpf(obj);
   if (o->cur.source) _proxy_unset(obj);
}

EAPI Evas_Object *
evas_object_image_add(Evas *e)
{
   Evas_Object *obj;
   Evas_Object_Image *o;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new(e);
   evas_object_image_init(obj);
   evas_object_inject(obj, e);
   o = (Evas_Object_Image *)(obj->object_data);
   o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
								       o->engine_data);
   return obj;
}

EAPI Evas_Object *
evas_object_image_filled_add(Evas *e)
{
   Evas_Object *obj;
   obj = evas_object_image_add(e);
   evas_object_image_filled_set(obj, 1);
   return obj;
}

static void
_cleanup_tmpf(Evas_Object *obj)
{
   Evas_Object_Image *o;
   
   o = (Evas_Object_Image *)(obj->object_data);
   if (!o->tmpf) return;
#ifdef __linux__
#else
   unlink(o->tmpf);
#endif
   if (o->tmpf_fd >= 0) close(o->tmpf_fd);
   eina_stringshare_del(o->tmpf);
   o->tmpf_fd = -1;
   o->tmpf = NULL;
}

static void
_create_tmpf(Evas_Object *obj, void *data, int size, char *format __UNUSED__)
{
   Evas_Object_Image *o;
   char buf[4096];
   void *dst;
   int fd = -1;
   
   o = (Evas_Object_Image *)(obj->object_data);
#ifdef __linux__
   snprintf(buf, sizeof(buf), "/dev/shm/.evas-tmpf-%i-%p-%i-XXXXXX", 
            (int)getpid(), data, (int)size);
   fd = mkstemp(buf);
#endif   
   if (fd < 0)
     {
        snprintf(buf, sizeof(buf), "/tmp/.evas-tmpf-%i-%p-%i-XXXXXX", 
                 (int)getpid(), data, (int)size);
        fd = mkstemp(buf);
     }
   if (fd < 0) return;
   if (ftruncate(fd, size) < 0)
     {
        unlink(buf);
        close(fd);
        return;
     }
   unlink(buf);
   
   eina_mmap_safety_enabled_set(EINA_TRUE);
   
   dst = mmap(NULL, size, 
              PROT_READ | PROT_WRITE, 
              MAP_SHARED, 
              fd, 0);
   if (dst == MAP_FAILED)
     {
        close(fd);
        return;
     }
   o->tmpf_fd = fd;
#ifdef __linux__
   snprintf(buf, sizeof(buf), "/proc/%li/fd/%i", (long)getpid(), fd);
#endif
   o->tmpf = eina_stringshare_add(buf);
   memcpy(dst, data, size);
   munmap(dst, size);
}

EAPI void
evas_object_image_memfile_set(Evas_Object *obj, void *data, int size, char *format, char *key)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _cleanup_tmpf(obj);
   evas_object_image_file_set(obj, NULL, NULL);
   // invalidate the cache effectively
   evas_object_image_alpha_set(obj, !o->cur.has_alpha);
   evas_object_image_alpha_set(obj, !o->cur.has_alpha);

   if ((size < 1) || (!data)) return;

   _create_tmpf(obj, data, size, format);
   evas_object_image_file_set(obj, o->tmpf, key);
   if (!o->engine_data)
     {
        ERR("unable to load '%s' from memory", o->tmpf);
        _cleanup_tmpf(obj);
        return;
     }
}

EAPI void
evas_object_image_file_set(Evas_Object *obj, const char *file, const char *key)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->tmpf) && (file != o->tmpf)) _cleanup_tmpf(obj);
   if ((o->cur.file) && (file) && (!strcmp(o->cur.file, file)))
     {
	if ((!o->cur.key) && (!key))
	  return;
	if ((o->cur.key) && (key) && (!strcmp(o->cur.key, key)))
	  return;
     }
/*
 * WTF? why cancel a null image preload? this is just silly (tm)
   if (!o->engine_data)
     obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
							      o->engine_data,
							      obj);
 */
   if (o->cur.source) _proxy_unset(obj);
   if (o->cur.file) eina_stringshare_del(o->cur.file);
   if (o->cur.key) eina_stringshare_del(o->cur.key);
   if (file) o->cur.file = eina_stringshare_add(file);
   else o->cur.file = NULL;
   if (key) o->cur.key = eina_stringshare_add(key);
   else o->cur.key = NULL;
   o->prev.file = NULL;
   o->prev.key = NULL;
   if (o->engine_data)
     {
        if (o->preloading)
          {
             o->preloading = 0;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   o->load_error = EVAS_LOAD_ERROR_NONE;
   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   lo.region.x = o->load_opts.region.x;
   lo.region.y = o->load_opts.region.y;
   lo.region.w = o->load_opts.region.w;
   lo.region.h = o->load_opts.region.h;
   lo.orientation = o->load_opts.orientation;
   o->engine_data = obj->layer->evas->engine.func->image_load(obj->layer->evas->engine.data.output,
							      o->cur.file,
							      o->cur.key,
							      &o->load_error,
							      &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride;

	obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output,
						      o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output,
							  o->engine_data, &stride);
	else
	  stride = w * 4;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output,
									    o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	if (o->load_error == EVAS_LOAD_ERROR_NONE)
	  o->load_error = EVAS_LOAD_ERROR_GENERIC;
	o->cur.has_alpha = 1;
	o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_image_file_get(const Evas_Object *obj, const char **file, const char **key)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   if (file) *file = o->cur.file;
   if (key) *key = o->cur.key;
}

EAPI Eina_Bool
evas_object_image_source_set(Evas_Object *obj, Evas_Object *src)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (src == obj) return EINA_FALSE;
   if (o->cur.source == src) return EINA_TRUE;

   _evas_object_image_cleanup(obj, o);
   /* Kill the image if any */
   if (o->cur.file || o->cur.key)
      evas_object_image_file_set(obj, NULL, NULL);

   if (src)
     {
        _proxy_set(obj, src);
     }

   return EINA_TRUE;
}


EAPI Evas_Object *
evas_object_image_source_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = obj->object_data;
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();

   return o->cur.source;
}

EAPI Eina_Bool
evas_object_image_source_unset(Evas_Object *obj)
{
   return evas_object_image_source_set(obj, NULL);
}

EAPI void
evas_object_image_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (l < 0) l = 0;
   if (r < 0) r = 0;
   if (t < 0) t = 0;
   if (b < 0) b = 0;
   if ((o->cur.border.l == l) &&
       (o->cur.border.r == r) &&
       (o->cur.border.t == t) &&
       (o->cur.border.b == b)) return;
   o->cur.border.l = l;
   o->cur.border.r = r;
   o->cur.border.t = t;
   o->cur.border.b = b;
   o->cur.opaque_valid = 0;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_image_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   if (l) *l = o->cur.border.l;
   if (r) *r = o->cur.border.r;
   if (t) *t = o->cur.border.t;
   if (b) *b = o->cur.border.b;
}

EAPI void
evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Border_Fill_Mode fill)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (fill == o->cur.border.fill) return;
   o->cur.border.fill = fill;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Evas_Border_Fill_Mode
evas_object_image_border_center_fill_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.border.fill;
}

EAPI void
evas_object_image_filled_set(Evas_Object *obj, Eina_Bool setting)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

   setting = !!setting;
   if (o->filled == setting) return;

   o->filled = setting;
   if (!o->filled)
     evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE, evas_object_image_filled_resize_listener);
   else
     {
	Evas_Coord w, h;

	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
	evas_object_image_fill_set(obj, 0, 0, w, h);

	evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, evas_object_image_filled_resize_listener, NULL);
     }
}

EAPI Eina_Bool
evas_object_image_filled_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   return o->filled;
}

EAPI void
evas_object_image_border_scale_set(Evas_Object *obj, double scale)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (scale == o->cur.border.scale) return;
   o->cur.border.scale = scale;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI double
evas_object_image_border_scale_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 1.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 1.0;
   MAGIC_CHECK_END();
   return o->cur.border.scale;
}

EAPI void
evas_object_image_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Image *o;

   if (w < 0) w = -w;
   if (h < 0) h = -h;
   if (w == 0) return;
   if (h == 0) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->cur.opaque_valid = 0;   
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_image_fill_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
}


EAPI void
evas_object_image_fill_spread_set(Evas_Object *obj, Evas_Fill_Spread spread)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (spread == (Evas_Fill_Spread)o->cur.spread) return;
   o->cur.spread = spread;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Evas_Fill_Spread
evas_object_image_fill_spread_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   return (Evas_Fill_Spread)o->cur.spread;
}

EAPI void
evas_object_image_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;
   int stride = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w > 32768) return;
   if (h > 32768) return;
   if ((w == o->cur.image.w) &&
       (h == o->cur.image.h)) return;
   o->cur.image.w = w;
   o->cur.image.h = h;
   if (o->engine_data)
      o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output,
                                                                     o->engine_data,
                                                                     w, h);
   else
      o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data
      (obj->layer->evas->engine.data.output, w, h, NULL, o->cur.has_alpha,
          o->cur.cspace);

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
   o->cur.image.stride = stride;

/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_image_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->cur.image.w;
   if (h) *h = o->cur.image.h;
}

EAPI int
evas_object_image_stride_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.image.stride;
}

EAPI Evas_Load_Error
evas_object_image_load_error_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_error;
}

EAPI void *
evas_object_image_data_convert(Evas_Object *obj, Evas_Colorspace to_cspace)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = 0;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
     }
   if (!o->engine_data) return NULL;
   if (o->cur.cspace == to_cspace) return NULL;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data,
                                                                  &o->load_error);
   return evas_object_image_data_convert_internal(o, data, to_cspace);
}

EAPI void
evas_object_image_data_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;
   void *p_data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
     evas_common_pipe_op_image_flush(o->engine_data);
#endif
   p_data = o->engine_data;
   if (data)
     {
	if (o->engine_data)
	  o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
									 o->engine_data,
									 data);
	else
	  o->engine_data = obj->layer->evas->engine.func->image_new_from_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
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
                stride = o->cur.image.w * 4;
             o->cur.image.stride = stride;
         }
     }
   else
     {
	if (o->engine_data)
	  obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
						    o->engine_data);
	o->load_error = EVAS_LOAD_ERROR_NONE;
	o->cur.image.w = 0;
	o->cur.image.h = 0;
	o->cur.image.stride = 0;
	o->engine_data = NULL;
     }
/* FIXME - in engine call above
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
*/
   if (o->pixels_checked_out > 0) o->pixels_checked_out--;
   if (p_data != o->engine_data)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
	o->pixels_checked_out = 0;
     }
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void *
evas_object_image_data_get(const Evas_Object *obj, Eina_Bool for_writing)
{
   Evas_Object_Image *o;
   DATA32 *data;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->engine_data) return NULL;
#ifdef EVAS_FRAME_QUEUING
   evas_common_pipe_op_image_flush(o->engine_data);
#endif

   data = NULL;
   if (obj->layer->evas->engine.func->image_scale_hint_set)
      obj->layer->evas->engine.func->image_scale_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->scale_hint);
   if (obj->layer->evas->engine.func->image_content_hint_set)
      obj->layer->evas->engine.func->image_content_hint_set
      (obj->layer->evas->engine.data.output,
          o->engine_data, o->content_hint);
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  for_writing,
								  &data,
                                                                  &o->load_error);

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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   o->pixels_checked_out++;
   if (for_writing)
     {
	EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
     }

   return data;
}

EAPI void
evas_object_image_preload(Evas_Object *obj, Eina_Bool cancel)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return ;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return ;
   MAGIC_CHECK_END();
   if (!o->engine_data)
     {
        o->preloading = 1;
	evas_object_inform_call_image_preloaded(obj);
	return;
     }
   // FIXME: if already busy preloading, then dont request again until
   // preload done
   if (cancel)
     {
        if (o->preloading)
          {
             o->preloading = 0;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      obj);
          }
     }
   else
     {
        if (!o->preloading)
          {
             o->preloading = 1;
             obj->layer->evas->engine.func->image_data_preload_request(obj->layer->evas->engine.data.output,
                                                                       o->engine_data,
                                                                       obj);
          }
     }
}

EAPI void
evas_object_image_data_copy_set(Evas_Object *obj, void *data)
{
   Evas_Object_Image *o;

   if (!data) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
   if ((o->cur.image.w <= 0) ||
       (o->cur.image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
					       o->engine_data);
   o->engine_data = obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
									      o->cur.image.w,
									      o->cur.image.h,
									      data,
									      o->cur.has_alpha,
									      o->cur.cspace);
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
                                                                        o->engine_data,
                                                                        o->cur.has_alpha);
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   o->pixels_checked_out = 0;
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}

EAPI void
evas_object_image_data_update_add(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.image.w, o->cur.image.h);
   if ((w <= 0)  || (h <= 0)) return;
   NEW_RECT(r, x, y, w, h);
   if (r) o->pixel_updates = eina_list_append(o->pixel_updates, r);
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_image_alpha_set(Evas_Object *obj, Eina_Bool has_alpha)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = 0;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
     }
   if (((has_alpha) && (o->cur.has_alpha)) ||
       ((!has_alpha) && (!o->cur.has_alpha)))
     return;
   o->cur.has_alpha = has_alpha;
   if (o->engine_data)
     {
        int stride = 0;

#ifdef EVAS_FRAME_QUEUING
        evas_common_pipe_op_image_flush(o->engine_data);
#endif
        o->engine_data = obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								     o->engine_data,
								     o->cur.has_alpha);
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
   evas_object_image_data_update_add(obj, 0, 0, o->cur.image.w, o->cur.image.h);
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}


EAPI Eina_Bool
evas_object_image_alpha_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.has_alpha;
}

EAPI void
evas_object_image_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth_scale)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (((smooth_scale) && (o->cur.smooth_scale)) ||
       ((!smooth_scale) && (!o->cur.smooth_scale)))
     return;
   o->cur.smooth_scale = smooth_scale;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Eina_Bool
evas_object_image_smooth_scale_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->cur.smooth_scale;
}

EAPI void
evas_object_image_reload(Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = 0;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 obj);
     }
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
   evas_object_image_unload(obj, 1);
   evas_object_inform_call_image_unloaded(obj);
   evas_object_image_load(obj);
   o->prev.file = NULL;
   o->prev.key = NULL;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Eina_Bool
evas_object_image_save(const Evas_Object *obj, const char *file, const char *key, const char *flags)
{
   Evas_Object_Image *o;
   DATA32 *data = NULL;
   int quality = 80, compress = 9, ok = 0;
   RGBA_Image *im;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();

   if (!o->engine_data) return 0;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  0,
								  &data,
                                                                  &o->load_error);
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
	     if (pp) p = pp + 1;
	     else break;
	  }
     }
   im = (RGBA_Image*) evas_cache_image_data(evas_common_image_cache_get(),
                                            o->cur.image.w,
                                            o->cur.image.h,
                                            data,
                                            o->cur.has_alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   if (im)
     {
	if (o->cur.cspace == EVAS_COLORSPACE_ARGB8888)
	  im->image.data = data;
	else
	  im->image.data = evas_object_image_data_convert_internal(o,
								   data,
								   EVAS_COLORSPACE_ARGB8888);
	if (im->image.data)
	  {
	     ok = evas_common_save_image_to_file(im, file, key, quality, compress);

	     if (o->cur.cspace != EVAS_COLORSPACE_ARGB8888)
	       free(im->image.data);
	  }

	evas_cache_image_drop(&im->cache_entry);
     }
   return ok;
}

EAPI Eina_Bool
evas_object_image_pixels_import(Evas_Object *obj, Evas_Pixel_Import_Source *pixels)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
   if ((pixels->w != o->cur.image.w) || (pixels->h != o->cur.image.h)) return 0;
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
/*		  memcpy(image_pixels, pixels->rows, o->cur.image.w * o->cur.image.h * 4);*/
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
#ifdef BUILD_CONVERT_YUV
      case EVAS_PIXEL_FORMAT_YUV420P_601:
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
		  if (image_pixels)
		    evas_common_convert_yuv_420p_601_rgba((DATA8 **) pixels->rows,
							  (DATA8 *) image_pixels,
							  o->cur.image.w,
							  o->cur.image.h);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
								  o->engine_data,
								  image_pixels);
		  if (o->engine_data)
		    o->engine_data =
		    obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
								   o->engine_data,
								   o->cur.has_alpha);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
	break;
#endif
      default:
	return 0;
	break;
     }
   return 1;
}

EAPI void
evas_object_image_pixels_get_callback_set(Evas_Object *obj, Evas_Object_Image_Pixels_Get_Cb func, void *data)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->func.get_pixels = func;
   o->func.get_pixels_data = data;
}

EAPI void
evas_object_image_pixels_dirty_set(Evas_Object *obj, Eina_Bool dirty)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (dirty) o->dirty_pixels = 1;
   else o->dirty_pixels = 0;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Eina_Bool
evas_object_image_pixels_dirty_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   if (o->dirty_pixels) return 1;
   return 0;
}

EAPI void
evas_object_image_load_dpi_set(Evas_Object *obj, double dpi)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (dpi == o->load_opts.dpi) return;
   o->load_opts.dpi = dpi;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
        evas_object_inform_call_image_unloaded(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI double
evas_object_image_load_dpi_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0.0;
   MAGIC_CHECK_END();
   return o->load_opts.dpi;
}

EAPI void
evas_object_image_load_size_set(Evas_Object *obj, int w, int h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->load_opts.w == w) && (o->load_opts.h == h)) return;
   o->load_opts.w = w;
   o->load_opts.h = h;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
        evas_object_inform_call_image_unloaded(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI void
evas_object_image_load_size_get(const Evas_Object *obj, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (w) *w = o->load_opts.w;
   if (h) *h = o->load_opts.h;
}

EAPI void
evas_object_image_load_scale_down_set(Evas_Object *obj, int scale_down)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->load_opts.scale_down_by == scale_down) return;
   o->load_opts.scale_down_by = scale_down;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
        evas_object_inform_call_image_unloaded(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI int
evas_object_image_load_scale_down_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return 0;
   MAGIC_CHECK_END();
   return o->load_opts.scale_down_by;
}

EAPI void
evas_object_image_load_region_set(Evas_Object *obj, int x, int y, int w, int h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if ((o->load_opts.region.x == x) && (o->load_opts.region.y == y) &&
       (o->load_opts.region.w == w) && (o->load_opts.region.h == h)) return;
   o->load_opts.region.x = x;
   o->load_opts.region.y = y;
   o->load_opts.region.w = w;
   o->load_opts.region.h = h;
   if (o->cur.file)
     {
	evas_object_image_unload(obj, 0);
        evas_object_inform_call_image_unloaded(obj);
	evas_object_image_load(obj);
	o->changed = 1;
	evas_object_change(obj);
     }
}

EAPI void
evas_object_image_load_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->load_opts.region.x;
   if (y) *y = o->load_opts.region.y;
   if (w) *w = o->load_opts.region.w;
   if (h) *h = o->load_opts.region.h;
}

EAPI void
evas_object_image_load_orientation_set(Evas_Object *obj, Eina_Bool enable)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   o->load_opts.orientation = !!enable;
}

EAPI Eina_Bool
evas_object_image_load_orientation_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   return o->load_opts.orientation;
}

EAPI void
evas_object_image_colorspace_set(Evas_Object *obj, Evas_Colorspace cspace)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

   _evas_object_image_cleanup(obj, o);
#ifdef EVAS_FRAME_QUEUING
   if ((Evas_Colorspace)o->cur.cspace != cspace)
     {
        if (o->engine_data)
          evas_common_pipe_op_image_flush(o->engine_data);
     }
#endif

   o->cur.cspace = cspace;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_colorspace_set(obj->layer->evas->engine.data.output,
							 o->engine_data,
							 cspace);
}

EAPI Evas_Colorspace
evas_object_image_colorspace_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   return o->cur.cspace;
}

EAPI void
evas_object_image_video_surface_set(Evas_Object *obj, Evas_Video_Surface *surf)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
   if (o->video_surface)
     {
        o->video_surface = 0;
	obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, obj);
     }

   if (surf)
     {
        fprintf(stderr, "video surface ?\n");
        if (surf->version != EVAS_VIDEO_SURFACE_VERSION) return ;

	if (!surf->update_pixels ||
	    !surf->move ||
	    !surf->resize ||
	    !surf->hide ||
	    !surf->show)
	  return ;

        o->created = EINA_TRUE;
	o->video_surface = 1;
	o->video = *surf;

        fprintf(stderr, "yes\n");
	obj->layer->evas->video_objects = eina_list_append(obj->layer->evas->video_objects, obj);
     }
   else
     {
        o->video_surface = 0;
	o->video.update_pixels = NULL;
	o->video.move = NULL;
	o->video.resize = NULL;
	o->video.hide = NULL;
	o->video.show = NULL;
	o->video.data = NULL;
     }
}

EAPI const Evas_Video_Surface *
evas_object_image_video_surface_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!o->video_surface) return NULL;
   return &o->video;
}

EAPI void
evas_object_image_native_surface_set(Evas_Object *obj, Evas_Native_Surface *surf)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   _evas_object_image_cleanup(obj, o);
   if (!obj->layer->evas->engine.func->image_native_set) return;
   if ((surf) &&
       ((surf->version < 2) ||
        (surf->version > EVAS_NATIVE_SURFACE_VERSION))) return;
   o->engine_data = 
      obj->layer->evas->engine.func->image_native_set(obj->layer->evas->engine.data.output,
                                                      o->engine_data,
                                                      surf);
}

EAPI Evas_Native_Surface *
evas_object_image_native_surface_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->layer->evas->engine.func->image_native_get) return NULL;
   return obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output,
							  o->engine_data);
}

EAPI void
evas_object_image_scale_hint_set(Evas_Object *obj, Evas_Image_Scale_Hint hint)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->scale_hint == hint) return;
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
      evas_common_pipe_op_image_flush(o->engine_data);
#endif
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
}

EAPI Evas_Image_Scale_Hint
evas_object_image_scale_hint_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_SCALE_HINT_NONE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_IMAGE_SCALE_HINT_NONE;
   MAGIC_CHECK_END();
   return o->scale_hint;
}

EAPI void
evas_object_image_content_hint_set(Evas_Object *obj, Evas_Image_Content_Hint hint)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   if (o->content_hint == hint) return;
#ifdef EVAS_FRAME_QUEUING
   if (o->engine_data)
      evas_common_pipe_op_image_flush(o->engine_data);
#endif
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
}

EAPI void
evas_object_image_alpha_mask_set(Evas_Object *obj, Eina_Bool ismask)
{
   Evas_Object_Image *o;

   if (!ismask) return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

   /* Convert to A8 if not already */

   /* done */

}

#define FRAME_MAX 1024
EAPI Evas_Image_Content_Hint
evas_object_image_content_hint_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_CONTENT_HINT_NONE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_IMAGE_CONTENT_HINT_NONE;
   MAGIC_CHECK_END();
   return o->content_hint;
}

/* animated feature */
EAPI Eina_Bool
evas_object_image_animated_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (obj->layer->evas->engine.func->image_animated_get)
     return obj->layer->evas->engine.func->image_animated_get(obj->layer->evas->engine.data.output, o->engine_data);
   return EINA_FALSE;
}

EAPI int
evas_object_image_animated_frame_count_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return -1;
   MAGIC_CHECK_END();

   if (!evas_object_image_animated_get(obj)) return -1;
   if (obj->layer->evas->engine.func->image_animated_frame_count_get)
     return obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);
   return -1;
}

EAPI Evas_Image_Animated_Loop_Hint
evas_object_image_animated_loop_type_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_ANIMATED_HINT_NONE;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EVAS_IMAGE_ANIMATED_HINT_NONE;
   MAGIC_CHECK_END();

   if (!evas_object_image_animated_get(obj)) return EVAS_IMAGE_ANIMATED_HINT_NONE;

   if (obj->layer->evas->engine.func->image_animated_loop_type_get)
     return obj->layer->evas->engine.func->image_animated_loop_type_get(obj->layer->evas->engine.data.output, o->engine_data);
   return EVAS_IMAGE_ANIMATED_HINT_NONE;
}

EAPI int
evas_object_image_animated_loop_count_get(const Evas_Object *obj)
{
   Evas_Object_Image *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return -1;
   MAGIC_CHECK_END();

   if (!evas_object_image_animated_get(obj)) return -1;

   if (obj->layer->evas->engine.func->image_animated_loop_count_get)
     return obj->layer->evas->engine.func->image_animated_loop_count_get(obj->layer->evas->engine.data.output, o->engine_data);
   return -1;
}

EAPI double
evas_object_image_animated_frame_duration_get(const Evas_Object *obj, int start_frame, int frame_num)
{
   Evas_Object_Image *o;
   int frame_count = 0;

   if (start_frame < 1) return -1;
   if (frame_num < 0) return -1;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return -1;
   MAGIC_CHECK_END();

   if (!evas_object_image_animated_get(obj)) return -1;

   if (!obj->layer->evas->engine.func->image_animated_frame_count_get) return -1;

   frame_count = obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);

   if ((start_frame + frame_num) > frame_count) return -1;
   if (obj->layer->evas->engine.func->image_animated_frame_duration_get)
     return obj->layer->evas->engine.func->image_animated_frame_duration_get(obj->layer->evas->engine.data.output, o->engine_data, start_frame, frame_num);
   return -1;
}

EAPI void
evas_object_image_animated_frame_set(Evas_Object *obj, int frame_index)
{
   Evas_Object_Image *o;
   int frame_count = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();

   if (!o->cur.file) return;
   if (o->cur.frame == frame_index) return;

   if (!evas_object_image_animated_get(obj)) return;

   frame_count = evas_object_image_animated_frame_count_get(obj);

   /* limit the size of frame to FRAME_MAX */
   if ((frame_count > FRAME_MAX) || (frame_count < 0) || (frame_index > frame_count))
     return;

   if (!obj->layer->evas->engine.func->image_animated_frame_set) return;
   if (!obj->layer->evas->engine.func->image_animated_frame_set(obj->layer->evas->engine.data.output, o->engine_data, frame_index))
     return;

   o->prev.frame = o->cur.frame;
   o->cur.frame = frame_index;

   o->changed = 1;
   evas_object_change(obj);

}

EAPI void
evas_image_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   e->engine.func->image_cache_flush(e->engine.data.output);
}

EAPI void
evas_image_cache_reload(Evas *e)
{
   Evas_Layer *layer;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_image_cache_flush(e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
	     Evas_Object_Image *o;

	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_unload(obj, 1);
                  evas_object_inform_call_image_unloaded(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(layer->objects, obj)
	  {
	     Evas_Object_Image *o;

	     o = (Evas_Object_Image *)(obj->object_data);
	     if (o->magic == MAGIC_OBJ_IMAGE)
	       {
		  evas_object_image_load(obj);
		  o->changed = 1;
		  evas_object_change(obj);
	       }
	  }
     }
   evas_image_cache_flush(e);
}

EAPI void
evas_image_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (size < 0) size = 0;
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

EAPI int
evas_image_cache_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();

   return e->engine.func->image_cache_get(e->engine.data.output);
}

EAPI Eina_Bool
evas_image_max_size_get(const Evas *e, int *maxw, int *maxh)
{
   int w = 0, h = 0;
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   
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
_proxy_unset(Evas_Object *proxy)
{
   Evas_Object_Image *o;

   o = proxy->object_data;
   if (!o->cur.source) return;

   o->cur.source->proxy.proxies = eina_list_remove(o->cur.source->proxy.proxies, proxy);

   o->cur.source = NULL;
   if (o->cur.defmap)
     {
        evas_map_free(o->cur.defmap);
        o->cur.defmap = NULL;
     }
}


static void
_proxy_set(Evas_Object *proxy, Evas_Object *src)
{
   Evas_Object_Image *o;

   o = proxy->object_data;

   evas_object_image_file_set(proxy, NULL, NULL);

   o->cur.source = src;

   src->proxy.proxies = eina_list_append(src->proxy.proxies, proxy);
   src->proxy.redraw = EINA_TRUE;
}

/* Some moron just set a proxy on a proxy.
 * Give them some pixels.  A random color
 */
static void
_proxy_error(Evas_Object *proxy, void *context, void *output, void *surface,
             int x, int y)
{
   Evas_Func *func;
   int r = rand() % 255;
   int g = rand() % 255;
   int b = rand() % 255;
   
   /* XXX: Eina log error or something I'm sure
    * If it bugs you, just fix it.  Don't tell me */
   if (VERBOSE_PROXY_ERROR) printf("Err: Argh! Recursive proxies.\n");
   
   func = proxy->layer->evas->engine.func;
   func->context_color_set(output, context, r, g, b, 255);
   func->context_multiplier_unset(output, context);
   func->context_render_op_set(output, context, proxy->cur.render_op);
   func->rectangle_draw(output, context, surface, proxy->cur.geometry.x + x,
                        proxy->cur.geometry.y + y,
                        proxy->cur.geometry.w,
                        proxy->cur.geometry.h);
   return;
}

/*
static void
_proxy_subrender_recurse(Evas_Object *obj, Evas_Object *clip, void *output, void *surface, void *ctx, int x, int y)
{
   Evas_Object *obj2;
   Evas *e = obj->layer->evas;
   
   if (obj->clip.clipees) return;
   if (!obj->cur.visible) return;
   if ((!clip) || (clip != obj->cur.clipper))
     {
        if (!obj->cur.cache.clip.visible) return;
        if ((obj->cur.cache.clip.a == 0) &&
            (obj->cur.render_op == EVAS_RENDER_BLEND)) return;
     }
   if ((obj->func->is_visible) && (!obj->func->is_visible(obj))) return;
   
   if (!obj->pre_render_done)
      obj->func->render_pre(obj);
   ctx = e->engine.func->context_new(output);
   if (obj->smart.smart)
     {
        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
          {
             _proxy_subrender_recurse(obj2, clip, output, surface, ctx, x, y);
          }
     }
   else
     {
        obj->func->render(obj, output, ctx, surface, x, y);
     }
   e->engine.func->context_free(output, ctx);
}
*/

/**
 * Render the source object when a proxy is set.
 *
 * Used to force a draw if necessary, else just makes sures it's available.
 */
static void
_proxy_subrender(Evas *e, Evas_Object *source)
{
   void *ctx;
/*   Evas_Object *obj2, *clip;*/
   int w, h;

   if (!source) return;

   w = source->cur.geometry.w;
   h = source->cur.geometry.h;

   source->proxy.redraw = EINA_FALSE;

   /* We need to redraw surface then */
   if ((source->proxy.surface) && 
       ((source->proxy.w != w) || (source->proxy.h != h)))
     {
        e->engine.func->image_map_surface_free(e->engine.data.output,
                                               source->proxy.surface);
        source->proxy.surface = NULL;
     }
   
   /* FIXME: Hardcoded alpha 'on' */
   /* FIXME (cont): Should see if the object has alpha */
   if (!source->proxy.surface)
     {
        source->proxy.surface = e->engine.func->image_map_surface_new
           (e->engine.data.output, w, h, 1);
        source->proxy.w = w;
        source->proxy.h = h;
     }

   ctx = e->engine.func->context_new(e->engine.data.output);
   e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0, 0, 0);
   e->engine.func->context_render_op_set(e->engine.data.output, ctx, EVAS_RENDER_COPY);
   e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                  source->proxy.surface, 0, 0, w, h);
   e->engine.func->context_free(e->engine.data.output, ctx);
   
   ctx = e->engine.func->context_new(e->engine.data.output);
   evas_render_mapped(e, source, ctx, source->proxy.surface,
                      -source->cur.geometry.x,
                      -source->cur.geometry.y,
                      1, 0, 0, e->output.w, e->output.h);
   e->engine.func->context_free(e->engine.data.output, ctx);
   source->proxy.surface = e->engine.func->image_dirty_region
      (e->engine.data.output, source->proxy.surface, 0, 0, w, h);
/*   
   ctx = e->engine.func->context_new(e->engine.data.output);
   if (source->smart.smart)
     {
        clip = evas_object_smart_clipped_clipper_get(source);
        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(source), obj2)
          {
             _proxy_subrender_recurse(obj2, clip, e->engine.data.output,
                                      source->proxy.surface,
                                      ctx,
                                      -source->cur.geometry.x,
                                      -source->cur.geometry.y);
          }
     }
   else
     {
        if (!source->pre_render_done)
           source->func->render_pre(source);
        source->func->render(source, e->engine.data.output, ctx,
                             source->proxy.surface,
                             -source->cur.geometry.x,
                             -source->cur.geometry.y);
     }
   
   e->engine.func->context_free(e->engine.data.output, ctx);
   source->proxy.surface = e->engine.func->image_dirty_region
      (e->engine.data.output, source->proxy.surface, 0, 0, w, h);
 */
}

#if 0 // filtering disabled
/*
 *
 * Note that this is similar to proxy_subrender_recurse.  It should be
 * possible to merge I guess
 */
static void
image_filter_draw_under_recurse(Evas *e, Evas_Object *obj, Evas_Object *stop,
                                void *output, void *ctx, void *surface,
                                int x, int y)
{
   Evas_Object *obj2;
   
   if (obj->clip.clipees) return;
   /* FIXME: Doing bounding box test */
   if (!evas_object_is_in_output_rect(obj, stop->cur.geometry.x,
                                      stop->cur.geometry.y,
                                      stop->cur.geometry.w,
                                      stop->cur.geometry.h))
      return;
   
   if (!evas_object_is_visible(obj)) return;
   obj->pre_render_done = 1;
   ctx = e->engine.func->context_new(output);
   
   if (obj->smart.smart)
     {
        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj2)
          {
             if (obj2 == stop) return;
             image_filter_draw_under_recurse(e, obj2, stop, output, surface, 
                                             ctx, x, y);
          }
     }
   else
      obj->func->render(obj, output, ctx, surface, x ,y);
   e->engine.func->context_free(output, ctx);
}

/*
 * Draw all visible objects intersecting an object which are _beneath_ it.
 */
static void
image_filter_draw_under(Evas *e, Evas_Object *stop, void *output, void *ctx, void *surface, int dx, int dy)
{
   Evas_Layer *lay;
   int x, y;
   
   x = stop->cur.geometry.x - dx;
   y = stop->cur.geometry.y - dy;
   
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        Evas_Object *obj;
        EINA_INLIST_FOREACH(lay->objects, obj)
          {
             if (obj->delete_me) continue;
             if (obj == stop) return;
             /* FIXME: Do bounding box check */
             image_filter_draw_under_recurse(e, obj, stop, output, ctx, 
                                             surface, -x, -y);
          }
     }
   e->engine.func->image_dirty_region(output, surface, 0, 0, 300, 300);
   e->engine.func->output_flush(output);
}

/*
 * Update the filtered object.
 *
 * Creates a new context, and renders stuff (filtered) onto that.
 */
Filtered_Image *
image_filter_update(Evas *e, Evas_Object *obj, void *src, int imagew, int imageh, int *outw, int *outh)
{
   int w, h;
   void *ctx;
   Evas_Filter_Info *info;
   void *surface;
   Eina_Bool alpha;
   
   info = obj->filter;
   
   if (info->mode == EVAS_FILTER_MODE_BELOW)
     {
        w = obj->cur.geometry.w;
        h = obj->cur.geometry.h;
        evas_filter_get_size(info, w, h, &imagew, &imageh, EINA_TRUE);
        alpha = EINA_FALSE;
     }
   else
     {
        evas_filter_get_size(info, imagew, imageh, &w, &h, EINA_FALSE);
        alpha = e->engine.func->image_alpha_get(e->engine.data.output, src);
     }
   
   /* Certain filters may make alpha images anyway */
   if (alpha == EINA_FALSE) alpha = evas_filter_always_alpha(info);
   
   surface = e->engine.func->image_map_surface_new(e->engine.data.output, w, h,
                                                   alpha);
   
   if (info->mode == EVAS_FILTER_MODE_BELOW)
     {
        void *subsurface;
        int disw, dish;
        int dx, dy;
        disw = obj->cur.geometry.w;
        dish = obj->cur.geometry.h;
        dx = (imagew - w) >> 1;
        dy = (imageh - h) >> 1;
        subsurface = e->engine.func->image_map_surface_new
           (e->engine.data.output, imagew, imageh, 1);
        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 255, 0, 255);
        e->engine.func->context_render_op_set(e->engine.data.output, ctx, EVAS_RENDER_COPY);
        e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                       subsurface, 0, 0, imagew, imageh);
        
        image_filter_draw_under(e, obj, e->engine.data.output, ctx,
                                subsurface, dx, dy);
        
        e->engine.func->context_free(e->engine.data.output, ctx);
        
        ctx = e->engine.func->context_new(e->engine.data.output);
        
        e->engine.func->image_draw_filtered(e->engine.data.output,
                                            ctx, surface, subsurface, info);
        
        e->engine.func->context_free(e->engine.data.output, ctx);
        
        e->engine.func->image_map_surface_free(e->engine.data.output,
                                               subsurface);
     }
   else
     {
        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->image_draw_filtered(e->engine.data.output,
                                            ctx, surface, src, info);
        e->engine.func->context_free(e->engine.data.output, ctx);
     }
   
   e->engine.func->image_dirty_region(e->engine.data.output, surface, 
                                      0, 0, w, h);
   if (outw) *outw = w;
   if (outh) *outh = h;
   return e->engine.func->image_filtered_save(src, surface,
                                              obj->filter->key,
                                              obj->filter->len);
}
#endif

static void
evas_object_image_unload(Evas_Object *obj, Eina_Bool dirty)
{
   Evas_Object_Image *o;
   
   o = (Evas_Object_Image *)(obj->object_data);
   
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (dirty)
     {
        if (o->engine_data)
           o->engine_data = obj->layer->evas->engine.func->image_dirty_region
           (obj->layer->evas->engine.data.output,
               o->engine_data,
               0, 0,
               o->cur.image.w, o->cur.image.h);
     }
   if (o->engine_data)
     {
        if (o->preloading)
          {
             o->preloading = 0;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   o->engine_data = NULL;
   o->load_error = EVAS_LOAD_ERROR_NONE;
   o->cur.has_alpha = 1;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
   o->cur.image.stride = 0;
}

static void
evas_object_image_load(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Evas_Image_Load_Opts lo;

   o = (Evas_Object_Image *)(obj->object_data);
   if (o->engine_data) return;

   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   lo.region.x = o->load_opts.region.x;
   lo.region.y = o->load_opts.region.y;
   lo.region.w = o->load_opts.region.w;
   lo.region.h = o->load_opts.region.h;
   lo.orientation = o->load_opts.orientation;
   o->engine_data = obj->layer->evas->engine.func->image_load
      (obj->layer->evas->engine.data.output,
          o->cur.file,
          o->cur.key,
          &o->load_error,
          &lo);
   if (o->engine_data)
     {
	int w, h;
	int stride = 0;

	obj->layer->evas->engine.func->image_size_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &w, &h);
	if (obj->layer->evas->engine.func->image_stride_get)
	  obj->layer->evas->engine.func->image_stride_get
           (obj->layer->evas->engine.data.output,
               o->engine_data, &stride);
	else
	  stride = w * 4;
	o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get
           (obj->layer->evas->engine.data.output,
               o->engine_data);
	o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get
           (obj->layer->evas->engine.data.output,
               o->engine_data);
	o->cur.image.w = w;
	o->cur.image.h = h;
	o->cur.image.stride = stride;
     }
   else
     {
	o->load_error = EVAS_LOAD_ERROR_GENERIC;
     }
}

static Evas_Coord
evas_object_image_figure_x_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
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
evas_object_image_figure_y_fill(Evas_Object *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret)
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
evas_object_image_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_image_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 0;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_image_new(void)
{
   Evas_Object_Image *o;

   /* alloc obj private data */
   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object_image", Evas_Object_Image, 256, NULL);
   o = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object_Image);
   if (!o) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, o, Evas_Object_Image);
   o->magic = MAGIC_OBJ_IMAGE;
   o->cur.fill.w = 0;
   o->cur.fill.h = 0;
   o->cur.smooth_scale = 1;
   o->cur.border.fill = 1;
   o->cur.border.scale = 1.0;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.spread = EVAS_TEXTURE_REPEAT;
   o->cur.opaque_valid = 0;
   o->cur.source = NULL;
   o->prev = o->cur;
   o->tmpf_fd = -1;
   return o;
}

static void
evas_object_image_free(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   /* frees private object data. very simple here */
   o = (Evas_Object_Image *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   _cleanup_tmpf(obj);
   if (o->cur.file) eina_stringshare_del(o->cur.file);
   if (o->cur.key) eina_stringshare_del(o->cur.key);
   if (o->cur.source) _proxy_unset(obj);
   if (o->engine_data)
     {
        if (o->preloading)
          {
             o->preloading = 0;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                      o->engine_data,
                                                                      obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                                  o->engine_data);
     }
   if (o->video_surface)
     {
        o->video_surface = 0;
	obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, obj);
     }
   o->engine_data = NULL;
   o->magic = 0;
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
   EVAS_MEMPOOL_FREE(_mp_obj, o);
}

static void
evas_object_image_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Image *o;
   int imagew, imageh, uvw, uvh;
   void *pixels;

   /* render object to surface with context, and offset by x,y */
   o = (Evas_Object_Image *)(obj->object_data);

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     return; /* no error message, already printed in pre_render */

   /* Proxy sanity */
   if (o->proxyrendering)
     {
        _proxy_error(obj, context, output, surface, x, y);
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
                                                      obj->cur.geometry.x + x,
                                                      obj->cur.geometry.y + y,
                                                      obj->cur.geometry.w,
                                                      obj->cur.geometry.h);

        return ;
     }

   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);

   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     {
	obj->layer->evas->engine.func->context_multiplier_unset(output,
								context);
     }
   else
     obj->layer->evas->engine.func->context_multiplier_set(output,
							   context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);

   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);

   if (!o->cur.source)
     {
        pixels = o->engine_data;
        imagew = o->cur.image.w;
        imageh = o->cur.image.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (o->cur.source->proxy.surface && !o->cur.source->proxy.redraw)
     {
        pixels = o->cur.source->proxy.surface;
        imagew = o->cur.source->proxy.w;
        imageh = o->cur.source->proxy.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (o->cur.source->type == o_type &&
            ((Evas_Object_Image *)o->cur.source->object_data)->engine_data)
     {
        Evas_Object_Image *oi;
        oi = o->cur.source->object_data;
        pixels = oi->engine_data;
        imagew = oi->cur.image.w;
        imageh = oi->cur.image.h;
        uvw = o->cur.source->cur.geometry.w;
        uvh = o->cur.source->cur.geometry.h;
     }
   else
     {
        o->proxyrendering = 1;
        _proxy_subrender(obj->layer->evas, o->cur.source);
        pixels = o->cur.source->proxy.surface;
        imagew = o->cur.source->proxy.w;
        imageh = o->cur.source->proxy.h;
        uvw = imagew;
        uvh = imageh;
        o->proxyrendering = 0;
     }

#if 0 // filtering disabled
   /* Now check/update filter */
   if (obj->filter && obj->filter->filter)
     {
        Filtered_Image *fi = NULL;
        //printf("%p has filter: %s\n", obj,obj->filter->dirty?"dirty":"clean");
        if (obj->filter->dirty)
          {
             if (obj->filter->mode != EVAS_FILTER_MODE_BELOW)
               {
                  uint32_t len;
                  uint8_t *key;
                  
                  if (obj->filter->key) free(obj->filter->key);
                  obj->filter->key = NULL;
                  obj->filter->len = 0;
                  key = evas_filter_key_get(obj->filter, &len);
                  if (key)
                    {
                       obj->filter->key = key;
                       obj->filter->len = len;
                       fi = obj->layer->evas->engine.func->image_filtered_get
                          (o->engine_data, key, len);
                       if (obj->filter->cached && fi != obj->filter->cached)
                         {
                            obj->layer->evas->engine.func->image_filtered_free
                               (o->engine_data, obj->filter->cached);
                            obj->filter->cached = NULL;
                         }
                    }
               }
             else if (obj->filter->cached)
               {
                  obj->layer->evas->engine.func->image_filtered_free
                     (o->engine_data, obj->filter->cached);
               }
             if (!fi)
                fi = image_filter_update(obj->layer->evas, obj, pixels, 
                                         imagew, imageh, &imagew, &imageh);
             pixels = fi->image;
             obj->filter->dirty = 0;
             obj->filter->cached = fi;
          }
        else
          {
             fi = obj->filter->cached;
             pixels = fi->image;
          }
     }
#endif
   
   if (pixels)
     {
	Evas_Coord idw, idh, idx, idy;
	int ix, iy, iw, ih;

	if (o->dirty_pixels)
	  {
	     if (o->func.get_pixels)
	       {
		  o->func.get_pixels(o->func.get_pixels_data, obj);
		  if (o->engine_data != pixels)
		    pixels = o->engine_data;
		  o->engine_data = obj->layer->evas->engine.func->image_dirty_region
                     (obj->layer->evas->engine.data.output, o->engine_data,
                         0, 0, o->cur.image.w, o->cur.image.h);
	       }
	     o->dirty_pixels = 0;
	  }
        if ((obj->cur.map) && (obj->cur.map->count > 3) && (obj->cur.usemap))
          {
	     const Evas_Map_Point *p, *p_end;
             RGBA_Map_Point pts[obj->cur.map->count], *pt;

	     p = obj->cur.map->points;
	     p_end = p + obj->cur.map->count;
	     pt = pts;
             
             pts[0].px = obj->cur.map->persp.px << FP;
             pts[0].py = obj->cur.map->persp.py << FP;
             pts[0].foc = obj->cur.map->persp.foc << FP;
             pts[0].z0 = obj->cur.map->persp.z0 << FP;
             // draw geom +x +y
             for (; p < p_end; p++, pt++)
               {
                  pt->x = (lround(p->x) + x) * FP1;
                  pt->y = (lround(p->y) + y) * FP1;
                  pt->z = (lround(p->z)    ) * FP1;
                  pt->fx = p->px;
                  pt->fy = p->py;
                  pt->fz = p->z;
                  pt->u = ((lround(p->u) * imagew) / uvw) * FP1;
                  pt->v = ((lround(p->v) * imageh) / uvh) * FP1;
                  if      (pt->u < 0) pt->u = 0;
                  else if (pt->u > (imagew * FP1)) pt->u = (imagew * FP1);
                  if      (pt->v < 0) pt->v = 0;
                  else if (pt->v > (imageh * FP1)) pt->v = (imageh * FP1);
                  pt->col = ARGB_JOIN(p->a, p->r, p->g, p->b);
              }
	     if (obj->cur.map->count & 0x1)
	       {
		  pts[obj->cur.map->count] = pts[obj->cur.map->count -1];
	       }

             obj->layer->evas->engine.func->image_map_draw
               (output, context, surface, pixels, obj->cur.map->count,
		pts, o->cur.smooth_scale | obj->cur.map->smooth, 0);
          }
        else
          {
             obj->layer->evas->engine.func->image_scale_hint_set(output,
                                                                 pixels,
                                                                 o->scale_hint);
             /* This is technically a bug here: If the value is recreated
              * (which is returned)it may be a new object, however exactly 0
              * of all the evas engines do this. */
             obj->layer->evas->engine.func->image_border_set(output, pixels,
                                                             o->cur.border.l, o->cur.border.r,
                                                             o->cur.border.t, o->cur.border.b);
             idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
             idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
             if (idw < 1) idw = 1;
             if (idh < 1) idh = 1;
             if (idx > 0) idx -= idw;
             if (idy > 0) idy -= idh;
             while ((int)idx < obj->cur.geometry.w)
               {
                  Evas_Coord ydy;
                  int dobreak_w = 0;
                  
                  ydy = idy;
                  ix = idx;
                  if ((o->cur.fill.w == obj->cur.geometry.w) &&
                      (o->cur.fill.x == 0))
                    {
                       dobreak_w = 1;
                       iw = obj->cur.geometry.w;
                    }
                  else
                    iw = ((int)(idx + idw)) - ix;
                  while ((int)idy < obj->cur.geometry.h)
                    {
                       int dobreak_h = 0;
                       
                       iy = idy;
                       if ((o->cur.fill.h == obj->cur.geometry.h) &&
                           (o->cur.fill.y == 0))
                         {
                            ih = obj->cur.geometry.h;
                            dobreak_h = 1;
                         }
                       else
                         ih = ((int)(idy + idh)) - iy;
                       if ((o->cur.border.l == 0) &&
                           (o->cur.border.r == 0) &&
                           (o->cur.border.t == 0) &&
                           (o->cur.border.b == 0) &&
                           (o->cur.border.fill != 0))
                         obj->layer->evas->engine.func->image_draw(output,
                                                                   context,
                                                                   surface,
                                                                   pixels,
                                                                   0, 0,
                                                                   imagew,
                                                                   imageh,
                                                                   obj->cur.geometry.x + ix + x,
                                                                   obj->cur.geometry.y + iy + y,
                                                                   iw, ih,
                                                                   o->cur.smooth_scale);
                       else
                         {
                            int inx, iny, inw, inh, outx, outy, outw, outh;
                            int bl, br, bt, bb, bsl, bsr, bst, bsb;
                            int imw, imh, ox, oy;
                            
                            ox = obj->cur.geometry.x + ix + x;
                            oy = obj->cur.geometry.y + iy + y;
                            imw = imagew;
                            imh = imageh;
                            bl = o->cur.border.l;
                            br = o->cur.border.r;
                            bt = o->cur.border.t;
                            bb = o->cur.border.b;
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
                            if (o->cur.border.scale != 1.0)
                              {
                                 bsl = ((double)bl * o->cur.border.scale);
                                 bsr = ((double)br * o->cur.border.scale);
                                 bst = ((double)bt * o->cur.border.scale);
                                 bsb = ((double)bb * o->cur.border.scale);
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
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .##
                            // |
                            inx = bl; iny = 0;
                            inw = imw - bl - br; inh = bt;
                            outx = ox + bsl; outy = oy;
                            outw = iw - bsl - bsr; outh = bst;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // --#
                            //   |
                            inx = imw - br; iny = 0;
                            inw = br; inh = bt;
                            outx = ox + iw - bsr; outy = oy;
                            outw = bsr; outh = bst;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .--
                            // #  
                            inx = 0; iny = bt;
                            inw = bl; inh = imh - bt - bb;
                            outx = ox; outy = oy + bst;
                            outw = bsl; outh = ih - bst - bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // .--.
                            // |##|
                            if (o->cur.border.fill > EVAS_BORDER_FILL_NONE)
                              {
                                 inx = bl; iny = bt;
                                 inw = imw - bl - br; inh = imh - bt - bb;
                                 outx = ox + bsl; outy = oy + bst;
                                 outw = iw - bsl - bsr; outh = ih - bst - bsb;
                                 if ((o->cur.border.fill == EVAS_BORDER_FILL_SOLID) &&
                                     (obj->cur.cache.clip.a == 255) &&
                                     (obj->cur.render_op == EVAS_RENDER_BLEND))
                                   {
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           EVAS_RENDER_COPY);
                                      obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                                      obj->layer->evas->engine.func->context_render_op_set(output, context,
                                                                                           obj->cur.render_op);
                                   }
                                 else
                                   obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                              }
                            // --.
                            //   #
                            inx = imw - br; iny = bt;
                            inw = br; inh = imh - bt - bb;
                            outx = ox + iw - bsr; outy = oy + bst;
                            outw = bsr; outh = ih - bst - bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // |
                            // #--
                            inx = 0; iny = imh - bb;
                            inw = bl; inh = bb;
                            outx = ox; outy = oy + ih - bsb;
                            outw = bsl; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            // |
                            // .## 
                            inx = bl; iny = imh - bb;
                            inw = imw - bl - br; inh = bb;
                            outx = ox + bsl; outy = oy + ih - bsb;
                            outw = iw - bsl - bsr; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
                            //   |
                            // --#
                            inx = imw - br; iny = imh - bb;
                            inw = br; inh = bb;
                            outx = ox + iw - bsr; outy = oy + ih - bsb;
                            outw = bsr; outh = bsb;
                            obj->layer->evas->engine.func->image_draw(output, context, surface, pixels, inx, iny, inw, inh, outx, outy, outw, outh, o->cur.smooth_scale);
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
}

static void
evas_object_image_render_pre(Evas_Object *obj)
{
   Evas_Object_Image *o;
   int is_v = 0, was_v = 0;
   Evas *e;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Image *)(obj->object_data);
   e = obj->layer->evas;

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     {
        ERR("%p has invalid fill size: %dx%d. Ignored",
            obj, o->cur.fill.w, o->cur.fill.h);
	return;
     }

   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* Proxy: Do it early */
   if (o->cur.source && 
       (o->cur.source->proxy.redraw || o->cur.source->changed))
     {
        /* XXX: Do I need to sort out the map here? */
        evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
        goto done;
     }

   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&e->clip_changes, obj, is_v, was_v);
	if (!o->pixel_updates) goto done;
     }
   if ((obj->cur.map != obj->prev.map) ||
       (obj->cur.usemap != obj->prev.usemap))
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&e->clip_changes, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed render op */
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur.anti_alias != obj->prev.anti_alias)
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if (((o->cur.file) && (!o->prev.file)) ||
	    ((!o->cur.file) && (o->prev.file)) ||
	    ((o->cur.key) && (!o->prev.key)) ||
	    ((!o->cur.key) && (o->prev.key))
	    )
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.image.w != o->prev.image.w) ||
	    (o->cur.image.h != o->prev.image.h) ||
	    (o->cur.has_alpha != o->prev.has_alpha) ||
	    (o->cur.cspace != o->prev.cspace) ||
	    (o->cur.smooth_scale != o->prev.smooth_scale))
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if ((o->cur.border.l != o->prev.border.l) ||
	    (o->cur.border.r != o->prev.border.r) ||
	    (o->cur.border.t != o->prev.border.t) ||
	    (o->cur.border.b != o->prev.border.b) ||
            (o->cur.border.fill != o->prev.border.fill) ||
            (o->cur.border.scale != o->prev.border.scale))
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if (o->dirty_pixels)
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	if (o->cur.frame != o->prev.frame)
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
	
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
#if 0 // XXX: maybe buggy?
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h)) &&
       (o->cur.fill.w == o->prev.fill.w) &&
       (o->cur.fill.h == o->prev.fill.h) &&
       ((o->cur.fill.x + obj->cur.geometry.x) == (o->prev.fill.x + obj->prev.geometry.x)) &&
       ((o->cur.fill.y + obj->cur.geometry.y) == (o->prev.fill.y + obj->prev.geometry.y)) &&
       (!o->pixel_updates)
       )
     {
	evas_rects_return_difference_rects(&e->clip_changes,
					   obj->cur.geometry.x,
					   obj->cur.geometry.y,
					   obj->cur.geometry.w,
					   obj->cur.geometry.h,
					   obj->prev.geometry.x,
					   obj->prev.geometry.y,
					   obj->prev.geometry.w,
					   obj->prev.geometry.h);
	if (!o->pixel_updates) goto done;
     }
#endif   
   if (((obj->cur.geometry.x != obj->prev.geometry.x) ||
	(obj->cur.geometry.y != obj->prev.geometry.y) ||
	(obj->cur.geometry.w != obj->prev.geometry.w) ||
	(obj->cur.geometry.h != obj->prev.geometry.h))
       )
     {
	evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
	if ((o->cur.fill.x != o->prev.fill.x) ||
	    (o->cur.fill.y != o->prev.fill.y) ||
	    (o->cur.fill.w != o->prev.fill.w) ||
	    (o->cur.fill.h != o->prev.fill.h))
	  {
	     evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
	     if (!o->pixel_updates) goto done;
	  }
        if (o->pixel_updates)
          {
             if ((o->cur.border.l == 0) &&
                 (o->cur.border.r == 0) &&
                 (o->cur.border.t == 0) &&
                 (o->cur.border.b == 0) &&
                 (o->cur.image.w > 0) &&
                 (o->cur.image.h > 0) &&
                (!((obj->cur.map) && (obj->cur.usemap))))
               {
                  Eina_Rectangle *rr;
                  
                  EINA_LIST_FREE(o->pixel_updates, rr)
                    {
                       Evas_Coord idw, idh, idx, idy;
                       int x, y, w, h;
                       
                       e->engine.func->image_dirty_region(e->engine.data.output, o->engine_data, rr->x, rr->y, rr->w, rr->h);
                       
                       idx = evas_object_image_figure_x_fill(obj, o->cur.fill.x, o->cur.fill.w, &idw);
                       idy = evas_object_image_figure_y_fill(obj, o->cur.fill.y, o->cur.fill.h, &idh);
                       
                       if (idw < 1) idw = 1;
                       if (idh < 1) idh = 1;
                       if (idx > 0) idx -= idw;
                       if (idy > 0) idy -= idh;
                       while (idx < obj->cur.geometry.w)
                         {
                            Evas_Coord ydy;
                            
                            ydy = idy;
                            x = idx;
                            w = ((int)(idx + idw)) - x;
                            while (idy < obj->cur.geometry.h)
                              {
                                 Eina_Rectangle r;
                                 
                                 y = idy;
                                 h = ((int)(idy + idh)) - y;
                                 
                                 r.x = ((rr->x - 1) * w) / o->cur.image.w;
                                 r.y = ((rr->y - 1) * h) / o->cur.image.h;
                                 r.w = ((rr->w + 2) * w) / o->cur.image.w;
                                 r.h = ((rr->h + 2) * h) / o->cur.image.h;
                                 r.x += obj->cur.geometry.x + x;
                                 r.y += obj->cur.geometry.y + y;
                                 RECTS_CLIP_TO_RECT(r.x, r.y, r.w, r.h,
                                                    obj->cur.cache.clip.x, obj->cur.cache.clip.y,
                                                    obj->cur.cache.clip.w, obj->cur.cache.clip.h);
                                 evas_add_rect(&e->clip_changes, r.x, r.y, r.w, r.h);
                                 idy += h;
                              }
                            idx += idw;
                            idy = ydy;
                         }
                       eina_rectangle_free(rr);
                    }
                  goto done;
               }
             else
               {
		  Eina_Rectangle *r;
                  
		  EINA_LIST_FREE(o->pixel_updates, r)
                     eina_rectangle_free(r);
		  e->engine.func->image_dirty_region(e->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
		  evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
		  goto done;
	       }
	  }
     }
#if 0 // filtering disabled
   if (obj->filter && obj->filter->dirty)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, obj);
     }
#endif   
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* aren't fully opaque and we are visible */
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     {
         e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                                 obj->cur.cache.clip.x,
                                                 obj->cur.cache.clip.y,
                                                 obj->cur.cache.clip.w,
                                                 obj->cur.cache.clip.h);
     }
   done:
   evas_object_render_pre_effect_updates(&e->clip_changes, obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *obj)
{
   Evas_Object_Image *o;
   Eina_Rectangle *r;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Image *)(obj->object_data);
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
   /* FIXME: copy strings across */
}

static unsigned int evas_object_image_id_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static unsigned int evas_object_image_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static void *evas_object_image_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_image_is_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
/*  disable caching due tyo maps screwing with this
   o->cur.opaque_valid = 0;
   if (o->cur.opaque_valid)
     {
        if (!o->cur.opaque) return 0;
     }
   else
*/
     {
        o->cur.opaque = 0;
/* disable caching */
/*        o->cur.opaque_valid = 1; */
        if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
           return o->cur.opaque;
        if (((o->cur.border.l != 0) ||
             (o->cur.border.r != 0) ||
             (o->cur.border.t != 0) ||
             (o->cur.border.b != 0)) &&
            (!o->cur.border.fill)) return o->cur.opaque;
        if (!o->engine_data) return o->cur.opaque;
        o->cur.opaque = 1;
     }
   // FIXME: use proxy
   if (o->cur.source)
     {
        o->cur.opaque = evas_object_is_opaque(o->cur.source);
        return o->cur.opaque; /* FIXME: Should go poke at the object */
     }
   if (o->cur.has_alpha)
     {
        o->cur.opaque = 0;
        return o->cur.opaque;
     }
   if ((obj->cur.map) && (obj->cur.usemap))
     {
        Evas_Map *m = obj->cur.map;
        
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
               return o->cur.opaque;
          }
        o->cur.opaque = 0;
        return o->cur.opaque;
     }
   if (obj->cur.render_op == EVAS_RENDER_COPY) return o->cur.opaque;
   return o->cur.opaque;
}

static int
evas_object_image_was_opaque(Evas_Object *obj)
{
   Evas_Object_Image *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   o = (Evas_Object_Image *)(obj->object_data);
   if (o->prev.opaque_valid)
     {
        if (!o->prev.opaque) return 0;
     }
   else
     {
        o->prev.opaque = 0;
        o->prev.opaque_valid = 1;
        if ((o->prev.fill.w < 1) || (o->prev.fill.h < 1))
           return 0;
        if (((o->prev.border.l != 0) ||
             (o->prev.border.r != 0) ||
             (o->prev.border.t != 0) ||
             (o->prev.border.b != 0)) &&
            (!o->prev.border.fill)) return 0;
        if (!o->engine_data) return 0;
        o->prev.opaque = 1;
     }
   // FIXME: use proxy
   if (o->prev.source) return 0; /* FIXME: Should go poke at the object */
   if (obj->prev.usemap) return 0;
   if (obj->prev.render_op == EVAS_RENDER_COPY) return 1;
   if (o->prev.has_alpha) return 0;
   if (obj->prev.render_op != EVAS_RENDER_BLEND) return 0;
   return 1;
}

static int
evas_object_image_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Image *o;
   DATA32 *data;
   int w, h, stride, iw, ih;
   int a;

   o = (Evas_Object_Image *)(obj->object_data);

   x -= obj->cur.cache.clip.x;
   y -= obj->cur.cache.clip.y;
   w = obj->cur.cache.clip.w;
   h = obj->cur.cache.clip.h;
   iw = o->cur.image.w;
   ih = o->cur.image.h;

   if ((x < 0) || (y < 0) || (x >= w) || (y >= h)) return 0;
   if (!o->cur.has_alpha) return 1;

   // FIXME: proxy needs to be honored
   if (obj->cur.map)
     {
        x = obj->cur.map->mx;
        y = obj->cur.map->my;
     }
   else
     {
        int bl, br, bt, bb, bsl, bsr, bst, bsb;
        
        bl = o->cur.border.l;
        br = o->cur.border.r;
        bt = o->cur.border.t;
        bb = o->cur.border.b;
        if ((bl + br) > iw)
          {
             bl = iw / 2;
             br = iw - bl;
          }
        if ((bl + br) > iw)
          {
             bl = iw / 2;
             br = iw - bl;
          }
        if ((bt + bb) > ih)
          {
             bt = ih / 2;
             bb = ih - bt;
          }
        if ((bt + bb) > ih)
          {
             bt = ih / 2;
             bb = ih - bt;
          }
        if (o->cur.border.scale != 1.0)
          {
             bsl = ((double)bl * o->cur.border.scale);
             bsr = ((double)br * o->cur.border.scale);
             bst = ((double)bt * o->cur.border.scale);
             bsb = ((double)bb * o->cur.border.scale);
          }
        else
          {
             bsl = bl; bsr = br; bst = bt; bsb = bb;
          }
        
        w = o->cur.fill.w;
        h = o->cur.fill.h;
        x -= o->cur.fill.x;
        y -= o->cur.fill.y;
        x %= w;
        y %= h;
        
        if (x < 0) x += w;
        if (y < 0) y += h;
        
        if (o->cur.border.fill != EVAS_BORDER_FILL_DEFAULT)
          {
             if ((x > bsl) && (x < (w - bsr)) &&
                 (y > bst) && (y < (h - bsb)))
               {
                  if (o->cur.border.fill == EVAS_BORDER_FILL_SOLID) return 1;
                  return 0;
               }
          }
        
        if (x < bsl) x = (x * bl) / bsl;
        else if (x > (w - bsr)) x = iw - (((w - x) * br) / bsr);
        else if ((bsl + bsr) < w) x = bl + (((x - bsl) * (iw - bl - br)) / (w - bsl - bsr));
        else return 1;
        
        if (y < bst) y = (y * bt) / bst;
        else if (y > (h - bsb)) y = ih - (((h - y) * bb) / bsb);
        else if ((bst + bsb) < h) y = bt + (((y - bst) * (ih - bt - bb)) / (h - bst - bsb));
        else return 1;
     }
   
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if (x >= iw) x = iw - 1;
   if (y >= ih) y = ih - 1;
   
   stride = o->cur.image.stride;
   
   o->engine_data = obj->layer->evas->engine.func->image_data_get
      (obj->layer->evas->engine.data.output,
          o->engine_data,
          0,
          &data,
          &o->load_error);
   if (!data)
     return 0;

   switch (o->cur.cspace)
     {
     case EVAS_COLORSPACE_ARGB8888:
        data = ((DATA32*)(data) + ((y * (stride >> 2)) + x));
        a = (*((DATA32*)(data)) >> 24) & 0xff;
        break;
     case EVAS_COLORSPACE_RGB565_A5P:
        data = (void*) ((DATA16*)(data) + (h * (stride >> 1)));
        data = (void*) ((DATA8*)(data) + ((y * (stride >> 1)) + x));
        a = (*((DATA8*)(data))) & 0x1f;
        break;
     default:
        return 1;
        break;
     }

   return (a != 0);
}

static int
evas_object_image_has_opaque_rect(Evas_Object *obj)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if ((obj->cur.map) && (obj->cur.usemap)) return 0;
   if (((o->cur.border.l | o->cur.border.r | o->cur.border.t | o->cur.border.b) != 0) &&
       (o->cur.border.fill == EVAS_BORDER_FILL_SOLID) &&
       (obj->cur.render_op == EVAS_RENDER_BLEND) &&
       (obj->cur.cache.clip.a == 255) &&
       (o->cur.fill.x == 0) &&
       (o->cur.fill.y == 0) &&
       (o->cur.fill.w == obj->cur.geometry.w) &&
       (o->cur.fill.h == obj->cur.geometry.h)
       ) return 1;
   return 0;
}

static int
evas_object_image_get_opaque_rect(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o;

   o = (Evas_Object_Image *)(obj->object_data);
   if (o->cur.border.scale == 1.0)
     {
        *x = obj->cur.geometry.x + o->cur.border.l;
        *y = obj->cur.geometry.y + o->cur.border.t;
        *w = obj->cur.geometry.w - (o->cur.border.l + o->cur.border.r);
        if (*w < 0) *w = 0;
        *h = obj->cur.geometry.h - (o->cur.border.t + o->cur.border.b);
        if (*h < 0) *h = 0;
     }
   else
     {
        *x = obj->cur.geometry.x + (o->cur.border.l * o->cur.border.scale);
        *y = obj->cur.geometry.y + (o->cur.border.t * o->cur.border.scale);
        *w = obj->cur.geometry.w - ((o->cur.border.l * o->cur.border.scale) + (o->cur.border.r * o->cur.border.scale));
        if (*w < 0) *w = 0;
        *h = obj->cur.geometry.h - ((o->cur.border.t * o->cur.border.scale) + (o->cur.border.b * o->cur.border.scale));
        if (*h < 0) *h = 0;
     }
   return 1;
}

static int
evas_object_image_can_map(Evas_Object *obj __UNUSED__)
{
   return 1;
}

static void *
evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace)
{
   void *out = NULL;

   if (!data)
     return NULL;

   switch (o->cur.cspace)
     {
	case EVAS_COLORSPACE_ARGB8888:
	  out = evas_common_convert_argb8888_to(data,
						o->cur.image.w,
						o->cur.image.h,
						o->cur.image.stride >> 2,
						o->cur.has_alpha,
						to_cspace);
	  break;
	case EVAS_COLORSPACE_RGB565_A5P:
	  out = evas_common_convert_rgb565_a5p_to(data,
						  o->cur.image.w,
						  o->cur.image.h,
						  o->cur.image.stride >> 1,
						  o->cur.has_alpha,
						  to_cspace);
	  break;
        case EVAS_COLORSPACE_YCBCR422601_PL:
           fprintf(stderr, "EVAS_COLORSPACE_YCBCR422601_PL:\n");
          out = evas_common_convert_yuv_422_601_to(data,
                                                   o->cur.image.w,
                                                   o->cur.image.h,
                                                   to_cspace);
          break;
        case EVAS_COLORSPACE_YCBCR422P601_PL:
          out = evas_common_convert_yuv_422P_601_to(data,
                                                    o->cur.image.w,
                                                    o->cur.image.h,
                                                    to_cspace);
          break;
        case EVAS_COLORSPACE_YCBCR420NV12601_PL:
          out = evas_common_convert_yuv_420_601_to(data,
                                                   o->cur.image.w,
                                                   o->cur.image.h,
                                                   to_cspace);
          break;
        case EVAS_COLORSPACE_YCBCR420TM12601_PL:
          out = evas_common_convert_yuv_420T_601_to(data,
						    o->cur.image.w,
						    o->cur.image.h,
						    to_cspace);
          break;
	default:
           fprintf(stderr, "unknow colorspace: %i\n", o->cur.cspace);
	  break;
     }

   return out;
}

static void
evas_object_image_filled_resize_listener(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *einfo __UNUSED__)
{
   Evas_Coord w, h;
   Evas_Object_Image *o;

   o = obj->object_data;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_image_fill_set(obj, 0, 0, w, h);
}


Eina_Bool
_evas_object_image_preloading_get(const Evas_Object *obj)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);
   if (!o) return EINA_FALSE;
   MAGIC_CHECK(o, Evas_Object_Image, MAGIC_OBJ_IMAGE);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   return o->preloading;
}

void
_evas_object_image_preloading_set(Evas_Object *obj, Eina_Bool preloading)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);
   o->preloading = preloading;
}

void
_evas_object_image_preloading_check(Evas_Object *obj)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);
   if (obj->layer->evas->engine.func->image_load_error_get)
      o->load_error = obj->layer->evas->engine.func->image_load_error_get
      (obj->layer->evas->engine.data.output, o->engine_data);
}

Evas_Object *
_evas_object_image_video_parent_get(Evas_Object *obj)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);

   return o->video_surface ? o->video.parent : NULL;
}

void
_evas_object_image_video_overlay_show(Evas_Object *obj)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);

   if (obj->cur.cache.clip.x != obj->prev.cache.clip.x ||
       obj->cur.cache.clip.y != obj->prev.cache.clip.y ||
       o->created || !o->video_visible)
     o->video.move(o->video.data, obj, &o->video, obj->cur.cache.clip.x, obj->cur.cache.clip.y);
   if (obj->cur.cache.clip.w != obj->prev.cache.clip.w ||
       obj->cur.cache.clip.h != obj->prev.cache.clip.h ||
       o->created || !o->video_visible)
     o->video.resize(o->video.data, obj, &o->video, obj->cur.cache.clip.w, obj->cur.cache.clip.h);
   if (!o->video_visible || o->created)
     {
        o->video.show(o->video.data, obj, &o->video);
     }
   else
     {
        /* Cancel dirty on the image */
        Eina_Rectangle *r;

        o->dirty_pixels = 0;
        EINA_LIST_FREE(o->pixel_updates, r)
          eina_rectangle_free(r);
     }
   o->video_visible = EINA_TRUE;
   o->created = EINA_FALSE;
}

void
_evas_object_image_video_overlay_hide(Evas_Object *obj)
{
   Evas_Object_Image *o = (Evas_Object_Image *)(obj->object_data);

   if (o->video_visible || o->created)
     o->video.hide(o->video.data, obj, &o->video);
   if (evas_object_is_visible(obj))
     o->video.update_pixels(o->video.data, obj, &o->video);
   o->video_visible = EINA_FALSE;
   o->created = EINA_FALSE;
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
