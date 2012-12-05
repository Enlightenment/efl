#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <math.h>

#include "evas_common.h"
#include "evas_private.h"
#ifdef EVAS_CSERVE2
#include "../cserve2/evas_cs2_private.h"
#endif
#include "../common/evas_convert_color.h"
#include "../common/evas_convert_colorspace.h"
#include "../common/evas_convert_yuv.h"

#include <Eo.h>

EAPI Eo_Op EVAS_OBJ_IMAGE_BASE_ID = EO_NOOP;

#define MY_CLASS EVAS_OBJ_IMAGE_CLASS

#define VERBOSE_PROXY_ERROR 1

/* private magic number for image objects */
static const char o_type[] = "image";

/* private struct for rectangle object internal data */
typedef struct _Evas_Object_Image Evas_Object_Image;

struct _Evas_Object_Image
{
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

      Eina_Bool      smooth_scale : 1;
      Eina_Bool      has_alpha :1;
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
      struct {
         int src_x, src_y, src_w, src_h;
         int dst_w, dst_h;
         int smooth;
         int scale_hint;
      } scale_load;
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

   Eina_Bool         changed : 1;
   Eina_Bool         dirty_pixels : 1;
   Eina_Bool         filled : 1;
   Eina_Bool         proxyrendering : 1;
   Eina_Bool         preloading : 1;
   Eina_Bool         video_surface : 1;
   Eina_Bool         video_visible : 1;
   Eina_Bool         created : 1;
};

/* private methods for image objects */
static void evas_object_image_unload(Evas_Object *eo_obj, Eina_Bool dirty);
static void evas_object_image_load(Evas_Object *eo_obj);
static Evas_Coord evas_object_image_figure_x_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);
static Evas_Coord evas_object_image_figure_y_fill(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord start, Evas_Coord size, Evas_Coord *size_ret);

static void evas_object_image_init(Evas_Object *eo_obj);
static void evas_object_image_new(Evas_Object *eo_obj);
static void evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_image_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static void evas_object_image_render_pre(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static void evas_object_image_render_post(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);

static unsigned int evas_object_image_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_image_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_image_engine_data_get(Evas_Object *eo_obj);

static int evas_object_image_is_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static int evas_object_image_was_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static int evas_object_image_is_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y);
static int evas_object_image_has_opaque_rect(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);
static int evas_object_image_get_opaque_rect(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
static int evas_object_image_can_map(Evas_Object *eo_obj);

static void *evas_object_image_data_convert_internal(Evas_Object_Image *o, void *data, Evas_Colorspace to_cspace);
static void evas_object_image_filled_resize_listener(void *data, Evas *eo_e, Evas_Object *eo_obj, void *einfo);

static void _proxy_unset(Evas_Object *proxy);
static void _proxy_set(Evas_Object *proxy, Evas_Object *src);
static void _proxy_error(Evas_Object *proxy, void *context, void *output, void *surface, int x, int y);

static void _cleanup_tmpf(Evas_Object *eo_obj);

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

static void
_evas_object_image_cleanup(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object_Image *o)
{
   obj->cur.opaque_valid = 0;
   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if (o->tmpf) _cleanup_tmpf(eo_obj);
   if (o->cur.source) _proxy_unset(eo_obj);
}

static void
_constructor(Eo *eo_obj, void *class_data, va_list *list EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = class_data;
   Evas *eo_e = evas_object_evas_get(eo_parent_get(eo_obj));

   eo_do_super(eo_obj, eo_constructor());
   evas_object_image_init(eo_obj);
   evas_object_inject(eo_obj, obj, eo_e);
   o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output, o->engine_data);
}

EAPI Evas_Object *
evas_object_image_add(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->engine.func, NULL);
   Evas_Object *eo_obj = eo_add(EVAS_OBJ_IMAGE_CLASS, eo_e);
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

static void
_cleanup_tmpf(Evas_Object *eo_obj)
{
#ifdef HAVE_SYS_MMAN_H
   Evas_Object_Image *o;

   o = eo_data_get(eo_obj, MY_CLASS);
   if (!o->tmpf) return;
#ifdef __linux__
#else
   unlink(o->tmpf);
#endif
   if (o->tmpf_fd >= 0) close(o->tmpf_fd);
   eina_stringshare_del(o->tmpf);
   o->tmpf_fd = -1;
   o->tmpf = NULL;
#else
   (void) eo_obj;
#endif
}

static void
_create_tmpf(Evas_Object *eo_obj, void *data, int size, char *format EINA_UNUSED)
{
#ifdef HAVE_SYS_MMAN_H
   Evas_Object_Image *o;
   char buf[PATH_MAX];
   void *dst;
   int fd = -1;

   o = eo_data_get(eo_obj, MY_CLASS);
#ifdef __linux__
   snprintf(buf, sizeof(buf), "/dev/shm/.evas-tmpf-%i-%p-%i-XXXXXX", 
            (int)getpid(), data, (int)size);
   fd = mkstemp(buf);
#endif   
   if (fd < 0)
     {
        const char *tmpdir = getenv("TMPDIR");

        if (!tmpdir)
          {
             tmpdir = getenv("TMP");
             if (!tmpdir)
               {
                  tmpdir = getenv("TEMP");
                  if (!tmpdir) tmpdir = "/tmp";
               }
          }
        snprintf(buf, sizeof(buf), "%s/.evas-tmpf-%i-%p-%i-XXXXXX",
                 tmpdir, (int)getpid(), data, (int)size);
        fd = mkstemp(buf);
        if (fd < 0) return;
     }
   if (ftruncate(fd, size) < 0)
     {
        unlink(buf);
        close(fd);
        return;
     }
#ifdef __linux__
   unlink(buf);
#endif

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
#else
   (void) eo_obj;
   (void) data;
   (void) size;
   (void) format;
#endif
}

EAPI void
evas_object_image_memfile_set(Evas_Object *eo_obj, void *data, int size, char *format, char *key)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_memfile_set(data, size, format, key));
}

static void
_image_memfile_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;

   void *data = va_arg(*list, void *);
   int size = va_arg(*list, int);
   char *format = va_arg(*list, char*);
   char *key = va_arg(*list, char*);

   _cleanup_tmpf(eo_obj);
   evas_object_image_file_set(eo_obj, NULL, NULL);
   // invalidate the cache effectively
   evas_object_image_alpha_set(eo_obj, !o->cur.has_alpha);
   evas_object_image_alpha_set(eo_obj, o->cur.has_alpha);

   if ((size < 1) || (!data)) return;

   _create_tmpf(eo_obj, data, size, format);
   evas_object_image_file_set(eo_obj, o->tmpf, key);
   if (!o->engine_data)
     {
        ERR("unable to load '%s' from memory", o->tmpf);
        _cleanup_tmpf(eo_obj);
        return;
     }
}

EAPI void
evas_object_image_file_set(Evas_Object *eo_obj, const char *file, const char *key)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_file_set(file, key));
}

static void
_image_file_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   Evas_Image_Load_Opts lo;
   Eina_Bool resize_call = EINA_FALSE;

   const char *file = va_arg(*list, const char*);
   const char *key = va_arg(*list, const char*);

   if ((o->tmpf) && (file != o->tmpf)) _cleanup_tmpf(eo_obj);
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
							      eo_obj);
 */
   if (o->cur.source) _proxy_unset(eo_obj);
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
             o->preloading = EINA_FALSE;
             obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output, o->engine_data, eo_obj);
          }
        obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output, o->engine_data);
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
   lo.scale_load.src_x = o->load_opts.scale_load.src_x;
   lo.scale_load.src_y = o->load_opts.scale_load.src_y;
   lo.scale_load.src_w = o->load_opts.scale_load.src_w;
   lo.scale_load.src_h = o->load_opts.scale_load.src_h;
   lo.scale_load.dst_w = o->load_opts.scale_load.dst_w;
   lo.scale_load.dst_h = o->load_opts.scale_load.dst_h;
   lo.scale_load.smooth = o->load_opts.scale_load.smooth;
   lo.scale_load.scale_hint = o->load_opts.scale_load.scale_hint;
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

        obj->layer->evas->engine.func->image_size_get(obj->layer->evas->engine.data.output, o->engine_data, &w, &h);
        if (obj->layer->evas->engine.func->image_stride_get)
          obj->layer->evas->engine.func->image_stride_get(obj->layer->evas->engine.data.output, o->engine_data, &stride);
        else
          stride = w * 4;
        o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get(obj->layer->evas->engine.data.output, o->engine_data);
        o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get(obj->layer->evas->engine.data.output, o->engine_data);

        if ((o->cur.image.w != w) || (o->cur.image.h != h))
          resize_call = EINA_TRUE;

        o->cur.image.w = w;
        o->cur.image.h = h;
        o->cur.image.stride = stride;
     }
   else
     {
        if (o->load_error == EVAS_LOAD_ERROR_NONE)
          o->load_error = EVAS_LOAD_ERROR_GENERIC;
        o->cur.has_alpha = EINA_TRUE;
        o->cur.cspace = EVAS_COLORSPACE_ARGB8888;

        if ((o->cur.image.w != 0) || (o->cur.image.h != 0))
          resize_call = EINA_TRUE;

        o->cur.image.w = 0;
        o->cur.image.h = 0;
        o->cur.image.stride = 0;
     }

   o->changed = EINA_TRUE;
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_image_file_get(const Evas_Object *eo_obj, const char **file, const char **key)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (file) *file = NULL;
   if (key) *key = NULL;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_file_get(file, key));
}

static void
_image_file_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   const char **file = va_arg(*list, const char**);
   const char **key = va_arg(*list, const char**);
   if (file) *file = o->cur.file;
   if (key) *key = o->cur.key;
}

EAPI Eina_Bool
evas_object_image_source_set(Evas_Object *eo_obj, Evas_Object *src)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool result = EINA_FALSE;
   eo_do(eo_obj, evas_obj_image_source_set(src, &result));
   return result;
}

static void
_image_source_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   Evas_Object *eo_src = va_arg(*list, Evas_Object *);
   Eina_Bool *result = va_arg(*list, Eina_Bool *);
   if (result) *result = EINA_FALSE;

   if (obj->delete_me && eo_src)
     {
        WRN("Setting deleted object %p as image source %p", eo_src, eo_obj);
        return;
     }
   if (eo_src)
     {
        Evas_Object_Protected_Data *src = eo_data_get(eo_src, EVAS_OBJ_CLASS);
        if (src->delete_me)
          {
             WRN("Setting object %p to deleted image source %p", eo_src, eo_obj);
             return;
          }
        if (!src->layer)
          {
             CRIT("No evas surface associated with source object (%p)", eo_obj);
             return;
          }
        if ((obj->layer && src->layer) &&
            (obj->layer->evas != src->layer->evas))
          {
             CRIT("Setting object %p from Evas (%p) from another Evas (%p)", eo_src, src->layer->evas, obj->layer->evas);
             return;
          }
        if (eo_src == eo_obj)
          {
             CRIT("Setting object %p as a source for itself", obj);
             return;
          }
     }
   if (o->cur.source == eo_src)
     {
        if (result) *result = EINA_TRUE;
        return;
     }

   _evas_object_image_cleanup(eo_obj, obj, o);
   /* Kill the image if any */
   if (o->cur.file || o->cur.key)
      evas_object_image_file_set(eo_obj, NULL, NULL);

   if (eo_src) _proxy_set(eo_obj, eo_src);
   else _proxy_unset(eo_obj);

   if (result) *result = EINA_TRUE;
}


EAPI Evas_Object *
evas_object_image_source_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *source = NULL;
   eo_do((Eo *)eo_obj, evas_obj_image_source_get(&source));
   return source;
}

static void
_image_source_get(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Object **source = va_arg(*list, Evas_Object **);
   if (source) *source = o->cur.source;
}

EAPI Eina_Bool
evas_object_image_source_unset(Evas_Object *eo_obj)
{
   Eina_Bool result = EINA_FALSE;
   eo_do(eo_obj, evas_obj_image_source_set(NULL, &result));
   return result;
}

EAPI void
evas_object_image_source_events_set(Evas_Object *eo_obj, Eina_Bool source_events)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   eo_do(eo_obj, evas_obj_image_source_events_set(source_events));
}

static void
_image_source_events_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Eina_Bool source_events = va_arg(*list, int);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   source_events = !!source_events;
   if (obj->proxy.src_events == source_events) return;
   obj->proxy.src_events = source_events;
   if (!o->cur.source) return;
   if ((obj->proxy.src_invisible) || (!source_events)) return;
   //FIXME: Feed mouse events here.
}

EAPI Eina_Bool
evas_object_image_source_events_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   Eina_Bool source_events;
   eo_do((Eo*)eo_obj, evas_obj_image_source_events_get(&source_events));

   return source_events;
}

static void
_image_source_events_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Eina_Bool *source_events = va_arg(*list, Eina_Bool *);
   if (!source_events) return;
   *source_events = obj->proxy.src_events;
}

EAPI void
evas_object_image_source_visible_set(Evas_Object *eo_obj, Eina_Bool visible)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   eo_do(eo_obj, evas_obj_image_source_visible_set(visible));
}

static void
_image_source_visible_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *src_obj;
   Evas_Object_Image *o = _pd;
   Eina_Bool visible = va_arg(*list, int);

   if (!o->cur.source) return;

   visible = !!visible;
   src_obj = eo_data_get(o->cur.source, EVAS_OBJ_CLASS);
   if (src_obj->proxy.src_invisible == !visible) return;
   src_obj->proxy.src_invisible = !visible;
   src_obj->changed_src_visible = EINA_TRUE;
   evas_object_smart_member_cache_invalidate(o->cur.source, EINA_FALSE,
                                             EINA_FALSE, EINA_TRUE);
   evas_object_change(o->cur.source, src_obj);
   if ((!visible) || (!src_obj->proxy.src_events)) return;
   //FIXME: Feed mouse events here.
}

EAPI Eina_Bool
evas_object_image_source_visible_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   Eina_Bool visible;
   eo_do((Eo*)eo_obj, evas_obj_image_source_visible_get(&visible));

   return visible;
}

static void
_image_source_visible_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *src_obj;
   Evas_Object_Image *o = _pd;
   Eina_Bool *visible = va_arg(*list, Eina_Bool *);

   if (!visible) return;
   if (!o->cur.source) *visible = EINA_FALSE;
   src_obj = eo_data_get(o->cur.source, EVAS_OBJ_CLASS);
   if (src_obj) *visible = !src_obj->proxy.src_invisible;
   else *visible = EINA_FALSE;
}

EAPI void
evas_object_image_border_set(Evas_Object *eo_obj, int l, int r, int t, int b)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_border_set(l, r, t, b));
}

static void
_image_border_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   int l = va_arg(*list, int);
   int r = va_arg(*list, int);
   int t = va_arg(*list, int);
   int b = va_arg(*list, int);

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
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_image_border_get(const Evas_Object *eo_obj, int *l, int *r, int *t, int *b)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_border_get(l, r, t, b));
}

static void
_image_border_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   int *l = va_arg(*list, int *);
   int *r = va_arg(*list, int *);
   int *t = va_arg(*list, int *);
   int *b = va_arg(*list, int *);
   if (l) *l = o->cur.border.l;
   if (r) *r = o->cur.border.r;
   if (t) *t = o->cur.border.t;
   if (b) *b = o->cur.border.b;
}

EAPI void
evas_object_image_border_center_fill_set(Evas_Object *eo_obj, Evas_Border_Fill_Mode fill)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_border_center_fill_set(fill));
}

static void
_image_border_center_fill_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Border_Fill_Mode fill = va_arg(*list, Evas_Border_Fill_Mode);
   Evas_Object_Image *o = _pd;
   if (fill == o->cur.border.fill) return;
   o->cur.border.fill = fill;
   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EAPI Evas_Border_Fill_Mode
evas_object_image_border_center_fill_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Evas_Border_Fill_Mode fill = 0;
   eo_do((Eo *)eo_obj, evas_obj_image_border_center_fill_get(&fill));
   return fill;
}

static void
_image_border_center_fill_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Border_Fill_Mode *fill = va_arg(*list, Evas_Border_Fill_Mode *);
   const Evas_Object_Image *o = _pd;
   if (fill) *fill = o->cur.border.fill;
   return;
}

EAPI void
evas_object_image_filled_set(Evas_Object *eo_obj, Eina_Bool setting)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_filled_set(setting));
}

static void
_image_filled_set(Eo *eo_obj, void* _pd, va_list *list)
{
   Evas_Object_Image *o = _pd;

   Eina_Bool setting = va_arg(*list, int);
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

EAPI Eina_Bool
evas_object_image_filled_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool fill = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_filled_get(&fill));
   return fill;
}

static void
_image_filled_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Eina_Bool *fill = va_arg(*list, Eina_Bool *);
   if (fill) *fill = o->filled;
}

EAPI void
evas_object_image_border_scale_set(Evas_Object *eo_obj, double scale)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_border_scale_set(scale));
}

static void
_image_border_scale_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;

   double scale = va_arg(*list, double);
   if (scale == o->cur.border.scale) return;
   o->cur.border.scale = scale;
   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EAPI double
evas_object_image_border_scale_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 1.0;
   MAGIC_CHECK_END();
   double scale = 1.0;
   eo_do((Eo *)eo_obj, evas_obj_image_border_scale_get(&scale));
   return scale;
}

static void
_image_border_scale_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   double *scale = va_arg(*list, double *);
   if (scale) *scale = o->cur.border.scale;
}

EAPI void
evas_object_image_fill_set(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_fill_set(x, y, w, h));
}

static void
_image_fill_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj;
   Evas_Object_Image *o = _pd;

   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);

   if (w == 0) return;
   if (h == 0) return;
   if (w < 0) w = -w;
   if (h < 0) h = -h;

   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->changed = EINA_TRUE;
   obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   obj->cur.opaque_valid = 0;   
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_image_fill_get(const Evas_Object *eo_obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_fill_get(x, y, w, h));
}

static void
_image_fill_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;

   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
}


EAPI void
evas_object_image_fill_spread_set(Evas_Object *eo_obj, Evas_Fill_Spread spread)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_fill_spread_set(spread));
}

static void
_image_fill_spread_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Evas_Fill_Spread spread = va_arg(*list, Evas_Fill_Spread);
   if (spread == (Evas_Fill_Spread)o->cur.spread) return;
   o->cur.spread = spread;
   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EAPI Evas_Fill_Spread
evas_object_image_fill_spread_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   Evas_Fill_Spread spread = EVAS_TEXTURE_REPEAT;
   eo_do((Eo *)eo_obj, evas_obj_image_fill_spread_get(&spread));
   return spread;
}

static void
_image_fill_spread_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Fill_Spread *spread = va_arg(*list, Evas_Fill_Spread *);
   if (spread) *spread = (Evas_Fill_Spread)o->cur.spread;
}

EAPI void
evas_object_image_size_set(Evas_Object *eo_obj, int w, int h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_size_set(w, h));
}

static void
_image_size_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);

   Evas_Object_Image *o = _pd;
   int stride = 0;

   _evas_object_image_cleanup(eo_obj, obj, o);
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if (w > 32768) return;
   if (h > 32768) return;
   if ((w == o->cur.image.w) &&
       (h == o->cur.image.h)) return;
   o->cur.image.w = w;
   o->cur.image.h = h;

   if (o->engine_data)
      o->engine_data = obj->layer->evas->engine.func->image_size_set(obj->layer->evas->engine.data.output, o->engine_data, w, h);
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
   o->changed = EINA_TRUE;
   evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_image_size_get(const Evas_Object *eo_obj, int *w, int *h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_size_get(w, h));
}

static void
_image_size_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   if (w) *w = o->cur.image.w;
   if (h) *h = o->cur.image.h;
}

EAPI int
evas_object_image_stride_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   int stride = 0;
   eo_do((Eo *)eo_obj, evas_obj_image_stride_get(&stride));
   return stride;
}

static void
_image_stride_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *stride = va_arg(*list, int *);
   const Evas_Object_Image *o = _pd;
   if (stride) *stride = o->cur.image.stride;
}

EAPI Evas_Load_Error
evas_object_image_load_error_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Evas_Load_Error load_error = 0;
   eo_do((Eo *)eo_obj, evas_obj_image_load_error_get(&load_error));
   return load_error;
}

static void
_image_load_error_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Load_Error *load_error = va_arg(*list, Evas_Load_Error *);
   const Evas_Object_Image *o = _pd;
   if (load_error) *load_error = o->load_error;
}

EAPI void *
evas_object_image_data_convert(Evas_Object *eo_obj, Evas_Colorspace to_cspace)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   eo_do(eo_obj, evas_obj_image_data_convert(to_cspace, &data));
   return data;
}

static void
_image_data_convert(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   Evas_Colorspace to_cspace = va_arg(*list, Evas_Colorspace);
   void **ret_data = va_arg(*list, void **);
   if (ret_data) *ret_data = NULL;
   DATA32 *data;
   void* result = NULL;

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output, o->engine_data, eo_obj);
     }
   if (!o->engine_data) return;
   if (o->video_surface)
     o->video.update_pixels(o->video.data, eo_obj, &o->video);
   if (o->cur.cspace == to_cspace) return;
   data = NULL;
   o->engine_data = obj->layer->evas->engine.func->image_data_get(obj->layer->evas->engine.data.output, o->engine_data, 0, &data, &o->load_error);
   result = evas_object_image_data_convert_internal(o, data, to_cspace);
   if (o->engine_data)
     {
        o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output, o->engine_data, data);
     }

   if (ret_data) *ret_data = result;
}

EAPI void
evas_object_image_data_set(Evas_Object *eo_obj, void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_data_set(data));
}

static void
_image_data_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   void *p_data;
   Eina_Bool resize_call = EINA_FALSE;

   void *data = va_arg(*list, void *);

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
                                                                  o->cur.image.w,
                                                                  o->cur.image.h,
                                                                  data,
                                                                  o->cur.has_alpha,
                                                                  o->cur.cspace);
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
                stride = o->cur.image.w * 4;
             o->cur.image.stride = stride;
         }
     }
   else
     {
        if (o->engine_data)
          obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output, o->engine_data);
        o->load_error = EVAS_LOAD_ERROR_NONE;
        if ((o->cur.image.w != 0) || (o->cur.image.h != 0))
          resize_call = EINA_TRUE;
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
   o->changed = EINA_TRUE;
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
   evas_object_change(eo_obj, obj);
}

EAPI void *
evas_object_image_data_get(const Evas_Object *eo_obj, Eina_Bool for_writing)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   eo_do((Eo *)eo_obj, evas_obj_image_data_get(for_writing, &data));
   return data;
}

static void
_image_data_get(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = (Evas_Object_Image *) _pd;
   Eina_Bool for_writing = va_arg(*list, int);
   void **ret_data = va_arg(*list, void **);
   DATA32 *data;

   if (!o->engine_data)
     {
        if (ret_data) *ret_data = NULL;
        return;
     }

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
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
   if (!o->engine_data)
     {
        if (ret_data) *ret_data = NULL;
        return;
     }

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

   if (ret_data) *ret_data = data;
}

EAPI void
evas_object_image_preload(Evas_Object *eo_obj, Eina_Bool cancel)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return ;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_preload(cancel));
}

static void
_image_preload(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;

   if (!o->engine_data)
     {
        o->preloading = EINA_TRUE;
        evas_object_inform_call_image_preloaded(eo_obj);
        return;
     }
   // FIXME: if already busy preloading, then dont request again until
   // preload done
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Eina_Bool cancel = va_arg(*list, int);
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

EAPI void
evas_object_image_data_copy_set(Evas_Object *eo_obj, void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_data_copy_set(data));
}

static void
_image_data_copy_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   void *data = va_arg(*list, void *);

   if (!data) return;
   _evas_object_image_cleanup(eo_obj, obj, o);
   if ((o->cur.image.w <= 0) ||
       (o->cur.image.h <= 0)) return;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_free(obj->layer->evas->engine.data.output,
                                               o->engine_data);
   o->engine_data =
     obj->layer->evas->engine.func->image_new_from_copied_data(obj->layer->evas->engine.data.output,
                                                               o->cur.image.w,
                                                               o->cur.image.h,
                                                               data,
                                                               o->cur.has_alpha,
                                                               o->cur.cspace);
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data =
          obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
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
evas_object_image_data_update_add(Evas_Object *eo_obj, int x, int y, int w, int h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_data_update_add(x, y, w, h));
}

static void
_image_data_update_add(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Eina_Rectangle *r;

   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.image.w, o->cur.image.h);
   if ((w <= 0)  || (h <= 0)) return;
   NEW_RECT(r, x, y, w, h);
   if (r) o->pixel_updates = eina_list_append(o->pixel_updates, r);
   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_image_alpha_set(Evas_Object *eo_obj, Eina_Bool has_alpha)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_alpha_set(has_alpha));
}

static void
_image_alpha_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   Eina_Bool has_alpha = va_arg(*list, int);

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if (((has_alpha) && (o->cur.has_alpha)) ||
       ((!has_alpha) && (!o->cur.has_alpha)))
     return;
   o->cur.has_alpha = has_alpha;
   if (o->engine_data)
     {
        int stride = 0;

        o->engine_data =
          obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output,
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
   evas_object_image_data_update_add(eo_obj, 0, 0, o->cur.image.w, o->cur.image.h);
   EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o);
}


EAPI Eina_Bool
evas_object_image_alpha_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool has_alpha = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_alpha_get(&has_alpha));
   return has_alpha;
}

static void
_image_alpha_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *has_alpha = va_arg(*list, Eina_Bool *);
   const Evas_Object_Image *o = _pd;
   *has_alpha = o->cur.has_alpha;
}

EAPI void
evas_object_image_smooth_scale_set(Evas_Object *eo_obj, Eina_Bool smooth_scale)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_smooth_scale_set(smooth_scale));
}

static void
_image_smooth_scale_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Eina_Bool smooth_scale = va_arg(*list, int);
   Evas_Object_Image *o = _pd;

   if (((smooth_scale) && (o->cur.smooth_scale)) ||
       ((!smooth_scale) && (!o->cur.smooth_scale)))
     return;
   o->cur.smooth_scale = smooth_scale;
   o->changed = EINA_TRUE;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EAPI Eina_Bool
evas_object_image_smooth_scale_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool smooth_scale = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_smooth_scale_get(&smooth_scale));
   return smooth_scale;
}

static void
_image_smooth_scale_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *smooth_scale = va_arg(*list, Eina_Bool *);
   const Evas_Object_Image *o = _pd;
   if (smooth_scale) *smooth_scale = o->cur.smooth_scale;
}

EAPI void
evas_object_image_reload(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_reload());
}

static void
_image_reload(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   if ((o->preloading) && (o->engine_data))
     {
        o->preloading = EINA_FALSE;
        obj->layer->evas->engine.func->image_data_preload_cancel(obj->layer->evas->engine.data.output,
                                                                 o->engine_data,
                                                                 eo_obj);
     }
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;
   if (o->engine_data)
     o->engine_data = obj->layer->evas->engine.func->image_dirty_region(obj->layer->evas->engine.data.output, o->engine_data, 0, 0, o->cur.image.w, o->cur.image.h);
   evas_object_image_unload(eo_obj, 1);
   evas_object_inform_call_image_unloaded(eo_obj);
   evas_object_image_load(eo_obj);
   o->prev.file = NULL;
   o->prev.key = NULL;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EAPI Eina_Bool
evas_object_image_save(const Evas_Object *eo_obj, const char *file, const char *key, const char *flags)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool result = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_save(file, key, flags, &result));
   return result;
}

static void _image_save(Eo *eo_obj, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *key = va_arg(*list, const char *);
   const char *flags = va_arg(*list, const char *);
   Eina_Bool *result = va_arg(*list, Eina_Bool *);
   if (result) *result = 0;

   DATA32 *data = NULL;
   int quality = 80, compress = 9, ok = 0;
   RGBA_Image *im;
   Evas_Object_Image *o = _pd;
   if (!o->engine_data) return;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
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
   o->engine_data = obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output,
                                                                  o->engine_data,
                                                                  data);
   if (result) *result = ok;
}

EAPI Eina_Bool
evas_object_image_pixels_import(Evas_Object *eo_obj, Evas_Pixel_Import_Source *pixels)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool result = EINA_FALSE;
   eo_do(eo_obj, evas_obj_image_pixels_import(pixels, &result));
   return result;
}

static void
_image_pixels_import(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   Evas_Pixel_Import_Source *pixels = va_arg(*list, Evas_Pixel_Import_Source *);
   Eina_Bool *result = va_arg(*list, Eina_Bool *);
   if (result) *result = 0;

   _evas_object_image_cleanup(eo_obj, obj, o);
   if ((pixels->w != o->cur.image.w) || (pixels->h != o->cur.image.h)) return;

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
                    evas_common_convert_yuv_420p_601_rgba((DATA8 **) pixels->rows, (DATA8 *) image_pixels, o->cur.image.w, o->cur.image.h);
                  if (o->engine_data)
                    o->engine_data =
                       obj->layer->evas->engine.func->image_data_put(obj->layer->evas->engine.data.output, o->engine_data, image_pixels);
                  if (o->engine_data)
                    o->engine_data =
                       obj->layer->evas->engine.func->image_alpha_set(obj->layer->evas->engine.data.output, o->engine_data, o->cur.has_alpha);
                  o->changed = EINA_TRUE;
                  evas_object_change(eo_obj, obj);
               }
          }
        break;
      default:
        return;
        break;
     }
   if (result) *result = 1;
   return;
}

EAPI void
evas_object_image_pixels_get_callback_set(Evas_Object *eo_obj, Evas_Object_Image_Pixels_Get_Cb func, void *data)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_pixels_get_callback_set(func, data));
}

static void
_image_pixels_get_callback_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object_Image_Pixels_Get_Cb func = va_arg(*list, Evas_Object_Image_Pixels_Get_Cb);
   void *data = va_arg(*list, void *);
   Evas_Object_Image *o = _pd;

   o->func.get_pixels = func;
   o->func.get_pixels_data = data;
}

EAPI void
evas_object_image_pixels_dirty_set(Evas_Object *eo_obj, Eina_Bool dirty)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_pixels_dirty_set(dirty));
}

static void
_image_pixels_dirty_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Eina_Bool dirty = va_arg(*list, int);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   if (dirty) o->dirty_pixels = EINA_TRUE;
   else o->dirty_pixels = EINA_FALSE;
   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EAPI Eina_Bool
evas_object_image_pixels_dirty_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool dirty = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_pixels_dirty_get(&dirty));
   return dirty;
}

static void
_image_pixels_dirty_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *dirty = va_arg(*list, Eina_Bool *);
   const Evas_Object_Image *o = _pd;

   *dirty = (o->dirty_pixels ? 1 : 0);
}

EAPI void
evas_object_image_load_dpi_set(Evas_Object *eo_obj, double dpi)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_load_dpi_set(dpi));
}

static void
_image_load_dpi_set(Eo *eo_obj, void *_pd, va_list *list)
{
   double dpi = va_arg(*list, double);

   Evas_Object_Image *o = _pd;

   if (dpi == o->load_opts.dpi) return;
   o->load_opts.dpi = dpi;
   if (o->cur.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EAPI double
evas_object_image_load_dpi_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   double dpi = 0.0;
   eo_do((Eo *)eo_obj, evas_obj_image_load_dpi_get(&dpi));
   return dpi;
}

static void
_image_load_dpi_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   double *dpi = va_arg(*list, double *);
   *dpi = o->load_opts.dpi;
}

EAPI void
evas_object_image_load_size_set(Evas_Object *eo_obj, int w, int h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_load_size_set(w, h));
}

static void
_image_load_size_set(Eo *eo_obj, void *_pd, va_list *list)
{
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);

   Evas_Object_Image *o = _pd;

   if ((o->load_opts.w == w) && (o->load_opts.h == h)) return;
   o->load_opts.w = w;
   o->load_opts.h = h;
   if (o->cur.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EAPI void
evas_object_image_load_size_get(const Evas_Object *eo_obj, int *w, int *h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_load_size_get(w, h));
}

static void
_image_load_size_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   const Evas_Object_Image *o = _pd;

   if (w) *w = o->load_opts.w;
   if (h) *h = o->load_opts.h;
}

EAPI void
evas_object_image_load_scale_down_set(Evas_Object *eo_obj, int scale_down)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_load_scale_down_set(scale_down));
}

static void
_image_load_scale_down_set(Eo *eo_obj, void *_pd, va_list *list)
{
   int scale_down = va_arg(*list, int);

   Evas_Object_Image *o = _pd;

   if (o->load_opts.scale_down_by == scale_down) return;
   o->load_opts.scale_down_by = scale_down;
   if (o->cur.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EAPI int
evas_object_image_load_scale_down_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   int scale_down = 0;
   eo_do((Eo *)eo_obj, evas_obj_image_load_scale_down_get(&scale_down));
   return scale_down;
}

static void
_image_load_scale_down_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *scale_down = va_arg(*list, int *);
   const Evas_Object_Image *o = _pd;
   *scale_down = o->load_opts.scale_down_by;
}

EAPI void
evas_object_image_load_region_set(Evas_Object *eo_obj, int x, int y, int w, int h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_load_region_set(x, y, w, h));
}

static void
_image_load_region_set(Eo *eo_obj, void *_pd, va_list *list)
{
   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);

   Evas_Object_Image *o = _pd;

   if ((o->load_opts.region.x == x) && (o->load_opts.region.y == y) &&
       (o->load_opts.region.w == w) && (o->load_opts.region.h == h)) return;
   o->load_opts.region.x = x;
   o->load_opts.region.y = y;
   o->load_opts.region.w = w;
   o->load_opts.region.h = h;
   if (o->cur.file)
     {
        Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
        evas_object_image_unload(eo_obj, 0);
        evas_object_inform_call_image_unloaded(eo_obj);
        evas_object_image_load(eo_obj);
        o->changed = EINA_TRUE;
	evas_object_change(eo_obj, obj);
     }
}

EAPI void
evas_object_image_load_region_get(const Evas_Object *eo_obj, int *x, int *y, int *w, int *h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_obj, evas_obj_image_load_region_get(x, y, w, h));
}

static void
_image_load_region_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   const Evas_Object_Image *o = _pd;

   if (x) *x = o->load_opts.region.x;
   if (y) *y = o->load_opts.region.y;
   if (w) *w = o->load_opts.region.w;
   if (h) *h = o->load_opts.region.h;
}

EAPI void
evas_object_image_load_orientation_set(Evas_Object *eo_obj, Eina_Bool enable)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_load_orientation_set(enable));
}

static void
_image_load_orientation_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool enable = va_arg(*list, int);
   Evas_Object_Image *o = _pd;
   o->load_opts.orientation = !!enable;
}

EAPI Eina_Bool
evas_object_image_load_orientation_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool enable = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_load_orientation_get(&enable));
   return enable;
}

static void
_image_load_orientation_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *enable = va_arg(*list, Eina_Bool *);
   const Evas_Object_Image *o = _pd;
   *enable = o->load_opts.orientation;
}

EAPI void
evas_object_image_colorspace_set(Evas_Object *eo_obj, Evas_Colorspace cspace)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_colorspace_set(cspace));
}

static void
_image_colorspace_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Colorspace cspace = va_arg(*list, Evas_Colorspace);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   _evas_object_image_cleanup(eo_obj, obj, o);

   o->cur.cspace = cspace;
   if (o->engine_data)
     obj->layer->evas->engine.func->image_colorspace_set(obj->layer->evas->engine.data.output, o->engine_data, cspace);
}

EAPI Evas_Colorspace
evas_object_image_colorspace_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EVAS_COLORSPACE_ARGB8888;
   MAGIC_CHECK_END();
   Evas_Colorspace cspace = EVAS_COLORSPACE_ARGB8888;
   eo_do((Eo *)eo_obj, evas_obj_image_colorspace_get(&cspace));
   return cspace;
}

static void
_image_colorspace_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Colorspace *cspace = va_arg(*list, Evas_Colorspace *);
   const Evas_Object_Image *o = _pd;
   *cspace = o->cur.cspace;
}

EAPI void
evas_object_image_video_surface_set(Evas_Object *eo_obj, Evas_Video_Surface *surf)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_video_surface_set(surf));
}

static void
_image_video_surface_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Video_Surface *surf = va_arg(*list, Evas_Video_Surface *);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   _evas_object_image_cleanup(eo_obj, obj, o);
   if (o->video_surface)
     {
        o->video_surface = EINA_FALSE;
        obj->layer->evas->video_objects = eina_list_remove(obj->layer->evas->video_objects, eo_obj);
     }

   if (surf)
     {
        if (surf->version != EVAS_VIDEO_SURFACE_VERSION) return ;

        if (!surf->update_pixels ||
            !surf->move ||
            !surf->resize ||
            !surf->hide ||
            !surf->show)
          return ;

        o->created = EINA_TRUE;
        o->video_surface = EINA_TRUE;
        o->video = *surf;

        obj->layer->evas->video_objects = eina_list_append(obj->layer->evas->video_objects, eo_obj);
     }
   else
     {
        o->video_surface = EINA_FALSE;
        o->video.update_pixels = NULL;
        o->video.move = NULL;
        o->video.resize = NULL;
        o->video.hide = NULL;
        o->video.show = NULL;
        o->video.data = NULL;
     }
}

EAPI const Evas_Video_Surface *
evas_object_image_video_surface_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   const Evas_Video_Surface *surf = NULL;
   eo_do((Eo *)eo_obj, evas_obj_image_video_surface_get(&surf));
   return surf;
}

static void
_image_video_surface_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Video_Surface **surf = va_arg(*list, const Evas_Video_Surface **);
   const Evas_Object_Image *o = _pd;

   *surf = (!o->video_surface ? NULL : &o->video);
}

EAPI void
evas_object_image_native_surface_set(Evas_Object *eo_obj, Evas_Native_Surface *surf)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_native_surface_set(surf));
}

static void
_image_native_surface_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Native_Surface *surf = va_arg(*list, Evas_Native_Surface *);

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;

   _evas_object_image_cleanup(eo_obj, obj, o);
   if (!obj->layer->evas->engine.func->image_native_set) return;
   if ((surf) &&
       ((surf->version < 2) ||
        (surf->version > EVAS_NATIVE_SURFACE_VERSION))) return;
   o->engine_data = obj->layer->evas->engine.func->image_native_set(obj->layer->evas->engine.data.output, o->engine_data, surf);
}

EAPI Evas_Native_Surface *
evas_object_image_native_surface_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Native_Surface *surf = NULL;
   eo_do((Eo *)eo_obj, evas_obj_image_native_surface_get(&surf));
   return surf;
}

static void
_image_native_surface_get(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Native_Surface **surf = va_arg(*list, Evas_Native_Surface **);

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   const Evas_Object_Image *o = _pd;

   *surf =
      (!obj->layer->evas->engine.func->image_native_get ? NULL :
         obj->layer->evas->engine.func->image_native_get(obj->layer->evas->engine.data.output, o->engine_data));
}

EAPI void
evas_object_image_scale_hint_set(Evas_Object *eo_obj, Evas_Image_Scale_Hint hint)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_scale_hint_set(hint));
}

static void
_image_scale_hint_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Image_Scale_Hint hint = va_arg(*list, Evas_Image_Scale_Hint);
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
     }
}

EAPI Evas_Image_Scale_Hint
evas_object_image_scale_hint_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_SCALE_HINT_NONE;
   MAGIC_CHECK_END();
   Evas_Image_Scale_Hint hint = EVAS_IMAGE_SCALE_HINT_NONE;
   eo_do((Eo *)eo_obj, evas_obj_image_scale_hint_get(&hint));
   return hint;
}

static void
_image_scale_hint_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Image_Scale_Hint *hint = va_arg(*list, Evas_Image_Scale_Hint *);
   *hint = o->scale_hint;
}

EAPI void
evas_object_image_content_hint_set(Evas_Object *eo_obj, Evas_Image_Content_Hint hint)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_content_hint_set(hint));
}

static void
_image_content_hint_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Image *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Image_Content_Hint hint = va_arg(*list, Evas_Image_Content_Hint);
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
           stride = o->cur.image.w * 4;
        o->cur.image.stride = stride;
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
EAPI Evas_Image_Content_Hint
evas_object_image_content_hint_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_CONTENT_HINT_NONE;
   MAGIC_CHECK_END();
   Evas_Image_Content_Hint hint = EVAS_IMAGE_CONTENT_HINT_NONE;
   eo_do((Eo *)eo_obj, evas_obj_image_content_hint_get(&hint));
   return hint;
}

static void
_image_content_hint_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Image_Content_Hint *hint = va_arg(*list, Evas_Image_Content_Hint *);

   *hint = o->content_hint;
}

EAPI Eina_Bool
evas_object_image_region_support_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool region = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_region_support_get(&region));
   return region;
}

static void
_image_region_support_get(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   const Evas_Object_Image *o = _pd;

   Eina_Bool *region = va_arg(*list, Eina_Bool *);
   *region =  obj->layer->evas->engine.func->image_can_region_get(
      obj->layer->evas->engine.data.output,
      o->engine_data);
}

/* animated feature */
EAPI Eina_Bool
evas_object_image_animated_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool animated = EINA_FALSE;
   eo_do((Eo *)eo_obj, evas_obj_image_animated_get(&animated));
   return animated;
}

static void
_image_animated_get(Eo *eo_obj, void *_pd, va_list *list)
{
   Eina_Bool *animated = va_arg(*list, Eina_Bool *);
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   const Evas_Object_Image *o = _pd;

   *animated =
      obj->layer->evas->engine.func->image_animated_get ?
      obj->layer->evas->engine.func->image_animated_get(obj->layer->evas->engine.data.output, o->engine_data) :
      EINA_FALSE;
}

EAPI int
evas_object_image_animated_frame_count_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   int frame_count = -1;
   eo_do((Eo *)eo_obj, evas_obj_image_animated_frame_count_get(&frame_count));
   return frame_count;
}

static void
_image_animated_frame_count_get(Eo *eo_obj, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   int *frame_count = va_arg(*list, int *);
   *frame_count = -1;

   if (!evas_object_image_animated_get(eo_obj)) return;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   if (obj->layer->evas->engine.func->image_animated_frame_count_get)
     *frame_count = obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);
}

EAPI Evas_Image_Animated_Loop_Hint
evas_object_image_animated_loop_type_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EVAS_IMAGE_ANIMATED_HINT_NONE;
   MAGIC_CHECK_END();
   Evas_Image_Animated_Loop_Hint hint = EVAS_IMAGE_ANIMATED_HINT_NONE;
   eo_do((Eo *)eo_obj, evas_obj_image_animated_loop_type_get(&hint));
   return hint;
}

static void
_image_animated_loop_type_get(Eo *eo_obj, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   Evas_Image_Animated_Loop_Hint *hint = va_arg(*list, Evas_Image_Animated_Loop_Hint *);
   *hint = EVAS_IMAGE_ANIMATED_HINT_NONE;
   if (!evas_object_image_animated_get(eo_obj)) return;

   if (obj->layer->evas->engine.func->image_animated_loop_type_get)
      *hint = obj->layer->evas->engine.func->image_animated_loop_type_get(obj->layer->evas->engine.data.output, o->engine_data);
}

EAPI int
evas_object_image_animated_loop_count_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   int loop_count = -1;
   eo_do((Eo *)eo_obj, evas_obj_image_animated_loop_count_get(&loop_count));
   return loop_count;
}

static void
_image_animated_loop_count_get(Eo *eo_obj, void *_pd, va_list *list)
{
   const Evas_Object_Image *o = _pd;
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);

   int *loop_count = va_arg(*list, int *);
   *loop_count = -1;
   if (!evas_object_image_animated_get(eo_obj)) return;

   *loop_count =
      obj->layer->evas->engine.func->image_animated_loop_count_get ?
      obj->layer->evas->engine.func->image_animated_loop_count_get(obj->layer->evas->engine.data.output, o->engine_data) :
      -1;
}

EAPI double
evas_object_image_animated_frame_duration_get(const Evas_Object *eo_obj, int start_frame, int frame_num)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   double frame_duration = -1;
   eo_do((Eo *)eo_obj, evas_obj_image_animated_frame_duration_get(start_frame, frame_num, &frame_duration));
   return frame_duration;
}

static void
_image_animated_frame_duration_get(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   const Evas_Object_Image *o = _pd;
   int frame_count = 0;

   int start_frame = va_arg(*list, int);
   int frame_num = va_arg(*list, int);
   double *frame_duration = va_arg(*list, double *);
   *frame_duration = -1;

   if (!obj->layer->evas->engine.func->image_animated_frame_count_get) return;

   frame_count = obj->layer->evas->engine.func->image_animated_frame_count_get(obj->layer->evas->engine.data.output, o->engine_data);

   if ((start_frame + frame_num) > frame_count) return;
   if (obj->layer->evas->engine.func->image_animated_frame_duration_get)
     *frame_duration = obj->layer->evas->engine.func->image_animated_frame_duration_get(obj->layer->evas->engine.data.output, o->engine_data, start_frame, frame_num);
}

EAPI void
evas_object_image_animated_frame_set(Evas_Object *eo_obj, int frame_index)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_image_animated_frame_set(frame_index));
}

static void
_image_animated_frame_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = _pd;
   int frame_count = 0;

   if (!o->cur.file) return;
   int frame_index = va_arg(*list, int);
   if (o->cur.frame == frame_index) return;

   if (!evas_object_image_animated_get(eo_obj)) return;

   frame_count = evas_object_image_animated_frame_count_get(eo_obj);

   /* limit the size of frame to FRAME_MAX */
   if ((frame_count > FRAME_MAX) || (frame_count < 0) || (frame_index > frame_count))
     return;

   if (!obj->layer->evas->engine.func->image_animated_frame_set) return;
   if (!obj->layer->evas->engine.func->image_animated_frame_set(obj->layer->evas->engine.data.output, o->engine_data, frame_index))
     return;

   o->prev.frame = o->cur.frame;
   o->cur.frame = frame_index;

   o->changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);

}

EAPI void
evas_image_cache_flush(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_image_cache_flush());
}

void
_canvas_image_cache_flush(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
   e->engine.func->image_cache_flush(e->engine.data.output);
}

EAPI void
evas_image_cache_reload(Evas *eo_e)
{
   eo_do(eo_e, evas_canvas_image_cache_reload());
}

void
_canvas_image_cache_reload(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
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
                  Evas_Object_Image *o = eo_data_get(obj->object, MY_CLASS);
                  evas_object_image_load(obj->object);
                  o->changed = EINA_TRUE;
                  evas_object_change(obj->object, obj);
               }
	  }
     }
   evas_image_cache_flush(eo_e);
}

EAPI void
evas_image_cache_set(Evas *eo_e, int size)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_image_cache_set(size));
}

void
_canvas_image_cache_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int size = va_arg(*list, int);
   Evas_Public_Data *e = _pd;
   if (size < 0) size = 0;
   e->engine.func->image_cache_set(e->engine.data.output, size);
}

EAPI int
evas_image_cache_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   int ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_image_cache_get(&ret));
   return ret;
}

void
_canvas_image_cache_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   const Evas_Public_Data *e = _pd;
   *ret = e->engine.func->image_cache_get(e->engine.data.output);
}

EAPI Eina_Bool
evas_image_max_size_get(const Evas *eo_e, int *maxw, int *maxh)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)eo_e, evas_canvas_image_max_size_get(maxw, maxh, &ret));
   return ret;
}

void
_canvas_image_max_size_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *maxw = va_arg(*list, int *);
   int *maxh = va_arg(*list, int *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   int w = 0, h = 0;

   const Evas_Public_Data *e = _pd;
   if (maxw) *maxw = 0xffff;
   if (maxh) *maxh = 0xffff;
   if (!e->engine.func->image_max_size_get) return;
   e->engine.func->image_max_size_get(e->engine.data.output, &w, &h);
   if (maxw) *maxw = w;
   if (maxh) *maxh = h;
   if (ret) *ret = EINA_TRUE;
}

/* all nice and private */
static void
_proxy_unset(Evas_Object *proxy)
{
   Evas_Object_Image *o = eo_data_get(proxy, MY_CLASS);

   if (!o->cur.source) return;

   Evas_Object_Protected_Data *cur_source = eo_data_get(o->cur.source, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *cur_proxy = eo_data_get(proxy, EVAS_OBJ_CLASS);

   cur_source->proxy.proxies = eina_list_remove(cur_source->proxy.proxies, proxy);
   cur_proxy->proxy.is_proxy = EINA_FALSE;

   if (cur_source->proxy.src_invisible)
     {
        cur_source->proxy.src_invisible = EINA_FALSE;
        cur_source->changed_src_visible = EINA_TRUE;
        evas_object_change(o->cur.source, cur_source);
        evas_object_smart_member_cache_invalidate(o->cur.source, EINA_FALSE,
                                                  EINA_FALSE, EINA_TRUE);
     }
   o->cur.source = NULL;
   if (o->cur.defmap)
     {
        evas_map_free(o->cur.defmap);
        o->cur.defmap = NULL;
     }
}

static void
_proxy_set(Evas_Object *eo_proxy, Evas_Object *eo_src)
{
   Evas_Object_Protected_Data *src = eo_data_get(eo_src, EVAS_OBJ_CLASS);
   Evas_Object_Protected_Data *proxy = eo_data_get(eo_proxy, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = eo_data_get(eo_proxy, MY_CLASS);

   evas_object_image_file_set(eo_proxy, NULL, NULL);

   proxy->proxy.is_proxy = EINA_TRUE;
   o->cur.source = eo_src;
   o->load_error = EVAS_LOAD_ERROR_NONE;

   src->proxy.proxies = eina_list_append(src->proxy.proxies, eo_proxy);
   src->proxy.redraw = EINA_TRUE;
}

/* Some moron just set a proxy on a proxy.
 * Give them some pixels.  A random color
 */
static void
_proxy_error(Evas_Object *eo_proxy, void *context, void *output, void *surface,
             int x, int y)
{
   Evas_Func *func;
   int r = rand() % 255;
   int g = rand() % 255;
   int b = rand() % 255;

   /* XXX: Eina log error or something I'm sure
    * If it bugs you, just fix it.  Don't tell me */
   if (VERBOSE_PROXY_ERROR) printf("Err: Argh! Recursive proxies.\n");

   Evas_Object_Protected_Data *proxy = eo_data_get(eo_proxy, EVAS_OBJ_CLASS);
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
_proxy_subrender_recurse(Evas_Object *eo_obj, Evas_Object *clip, void *output, void *surface, void *ctx, int x, int y)
{
   Evas_Object *eo_obj2;
   Evas *eo_e = obj->layer->evas;
   
   if (obj->clip.clipees) return;
   if (!obj->cur.visible) return;
   if ((!clip) || (clip != obj->cur.clipper))
     {
        if (!obj->cur.cache.clip.visible) return;
        if ((obj->cur.cache.clip.a == 0) &&
            (obj->cur.render_op == EVAS_RENDER_BLEND)) return;
     }
   if ((obj->func->is_visible) && (!obj->func->is_visible(eo_obj))) return;
   
   if (!obj->pre_render_done)
      obj->func->render_pre(eo_obj);
   ctx = e->engine.func->context_new(output);
   if (obj->is_smart)
     {
        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), obj2)
          {
             _proxy_subrender_recurse(obj2, clip, output, surface, ctx, x, y);
          }
     }
   else
     {
        obj->func->render(eo_obj, output, ctx, surface, x, y);
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
_proxy_subrender(Evas *eo_e, Evas_Object *eo_source)
{
   Evas_Public_Data *e = eo_data_get(eo_e, EVAS_CLASS);
   void *ctx;
/*   Evas_Object *eo_obj2, *clip;*/
   int w, h;

   if (!eo_source) return;
   Evas_Object_Protected_Data *source = eo_data_get(eo_source, EVAS_OBJ_CLASS);

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

   if (!source->proxy.surface) return;

   ctx = e->engine.func->context_new(e->engine.data.output);
   e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0, 0, 0);
   e->engine.func->context_render_op_set(e->engine.data.output, ctx, EVAS_RENDER_COPY);
   e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                  source->proxy.surface, 0, 0, w, h);
   e->engine.func->context_free(e->engine.data.output, ctx);

   ctx = e->engine.func->context_new(e->engine.data.output);
   evas_render_mapped(e, eo_source, source, ctx, source->proxy.surface,
                      -source->cur.geometry.x,
                      -source->cur.geometry.y,
                      1, 0, 0, e->output.w, e->output.h, EINA_TRUE
#ifdef REND_DBG
                      , 1
#endif
                      );

   e->engine.func->context_free(e->engine.data.output, ctx);
   source->proxy.surface = e->engine.func->image_dirty_region
      (e->engine.data.output, source->proxy.surface, 0, 0, w, h);
/*   
   ctx = e->engine.func->context_new(e->engine.data.output);
   if (eo_isa(source, EVAS_OBJ_SMART_CLASS))
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

static void
evas_object_image_unload(Evas_Object *eo_obj, Eina_Bool dirty)
{
   Evas_Object_Image *o;
   Eina_Bool resize_call = EINA_FALSE;

   o = eo_data_get(eo_obj, MY_CLASS);
   if ((!o->cur.file) ||
       (o->pixels_checked_out > 0)) return;

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
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
   o->cur.has_alpha = EINA_TRUE;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   if ((o->cur.image.w != 0) || (o->cur.image.h != 0)) resize_call = EINA_TRUE;
   o->cur.image.w = 0;
   o->cur.image.h = 0;
   o->cur.image.stride = 0;
   if (resize_call) evas_object_inform_call_image_resize(eo_obj);
}

static void
evas_object_image_load(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   Evas_Image_Load_Opts lo;

   if (o->engine_data) return;

   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   lo.scale_down_by = o->load_opts.scale_down_by;
   lo.dpi = o->load_opts.dpi;
   lo.w = o->load_opts.w;
   lo.h = o->load_opts.h;
   lo.region.x = o->load_opts.region.x;
   lo.region.y = o->load_opts.region.y;
   lo.region.w = o->load_opts.region.w;
   lo.region.h = o->load_opts.region.h;
   lo.scale_load.src_x = o->load_opts.scale_load.src_x;
   lo.scale_load.src_y = o->load_opts.scale_load.src_y;
   lo.scale_load.src_w = o->load_opts.scale_load.src_w;
   lo.scale_load.src_h = o->load_opts.scale_load.src_h;
   lo.scale_load.dst_w = o->load_opts.scale_load.dst_w;
   lo.scale_load.dst_h = o->load_opts.scale_load.dst_h;
   lo.scale_load.smooth = o->load_opts.scale_load.smooth;
   lo.scale_load.scale_hint = o->load_opts.scale_load.scale_hint;
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
        o->cur.has_alpha = obj->layer->evas->engine.func->image_alpha_get
           (obj->layer->evas->engine.data.output,
            o->engine_data);
        o->cur.cspace = obj->layer->evas->engine.func->image_colorspace_get
           (obj->layer->evas->engine.data.output,
            o->engine_data);
        if ((o->cur.image.w != w) || (o->cur.image.h != h))
          resize_call = EINA_TRUE;
        o->cur.image.w = w;
        o->cur.image.h = h;
        o->cur.image.stride = stride;
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
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_image_new(eo_obj);
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
   obj->cur.opaque_valid = 0;
}

static void
evas_object_image_new(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, EVAS_OBJ_IMAGE_CLASS);
   o->cur.fill.w = 0;
   o->cur.fill.h = 0;
   o->cur.smooth_scale = EINA_TRUE;
   o->cur.border.fill = 1;
   o->cur.border.scale = 1.0;
   o->cur.cspace = EVAS_COLORSPACE_ARGB8888;
   o->cur.spread = EVAS_TEXTURE_REPEAT;
   o->cur.source = NULL;
   o->prev = o->cur;
   o->tmpf_fd = -1;
}

static void
_destructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_image_free(eo_obj, obj);
   eo_do_super(eo_obj, eo_destructor());
}

static void
evas_object_image_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;

   /* free obj */
   _cleanup_tmpf(eo_obj);
   if (o->cur.file) eina_stringshare_del(o->cur.file);
   if (o->cur.key) eina_stringshare_del(o->cur.key);
   if (o->cur.source) _proxy_unset(eo_obj);
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
   o->engine_data = NULL;
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
}

static void
evas_object_image_render(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   int imagew, imageh, uvw, uvh;
   void *pixels;

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     return; /* no error message, already printed in pre_render */

   /* Proxy sanity */
   if (o->proxyrendering)
     {
        _proxy_error(eo_obj, context, output, surface, x, y);
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

   Evas_Object_Protected_Data *source =
      (o->cur.source ?
       eo_data_get(o->cur.source, EVAS_OBJ_CLASS):
       NULL);
   if (!o->cur.source)
     {
        pixels = o->engine_data;
        imagew = o->cur.image.w;
        imageh = o->cur.image.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->proxy.surface && !source->proxy.redraw)
     {
        pixels = source->proxy.surface;
        imagew = source->proxy.w;
        imageh = source->proxy.h;
        uvw = imagew;
        uvh = imageh;
     }
   else if (source->type == o_type &&
            ((Evas_Object_Image *)eo_data_get(o->cur.source, MY_CLASS))->engine_data)
     {
        Evas_Object_Image *oi;
        oi = eo_data_get(o->cur.source, MY_CLASS);
        pixels = oi->engine_data;
        imagew = oi->cur.image.w;
        imageh = oi->cur.image.h;
        uvw = source->cur.geometry.w;
        uvh = source->cur.geometry.h;
     }
   else
     {
        o->proxyrendering = EINA_TRUE;
        _proxy_subrender(obj->layer->evas->evas, o->cur.source);
        pixels = source->proxy.surface;
        imagew = source->proxy.w;
        imageh = source->proxy.h;
        uvw = imagew;
        uvh = imageh;
        o->proxyrendering = EINA_FALSE;
     }

   if (pixels)
     {
        Evas_Coord idw, idh, idx, idy;
        int ix, iy, iw, ih;
        int img_set = 0;

        if (o->dirty_pixels)
          {
             if (o->func.get_pixels)
               {
                  // Set img object for direct rendering optimization
                  // Check for image w/h against image geometry w/h
                  // Check for image color r,g,b,a = {255,255,255,255}
                  // Check and make sure that there are no maps.
                  if ( (obj->cur.geometry.w == o->cur.image.w) &&
                       (obj->cur.geometry.h == o->cur.image.h) &&
                       (obj->cur.color.r == 255) &&
                       (obj->cur.color.g == 255) &&
                       (obj->cur.color.b == 255) &&
                       (obj->cur.color.a == 255) &&
                       (!obj->cur.map) )
                    {
                       if (obj->layer->evas->engine.func->gl_img_obj_set)
                         {
                            obj->layer->evas->engine.func->gl_img_obj_set(output, eo_obj, o->cur.has_alpha);
                            img_set = 1;
                         }
                    }

                  o->func.get_pixels(o->func.get_pixels_data, eo_obj);
                  if (o->engine_data != pixels)
                    pixels = o->engine_data;
                  o->engine_data = obj->layer->evas->engine.func->image_dirty_region
                     (obj->layer->evas->engine.data.output, o->engine_data,
                      0, 0, o->cur.image.w, o->cur.image.h);
               }
             o->dirty_pixels = EINA_FALSE;
          }
        if ((obj->cur.map) && (obj->cur.map->count > 3) && (obj->cur.usemap))
          {
             evas_object_map_update(eo_obj, x, y, imagew, imageh, uvw, uvh);

             obj->layer->evas->engine.func->image_map_draw
                (output, context, surface, pixels, obj->spans,
                 o->cur.smooth_scale | obj->cur.map->smooth, 0);
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
             idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur.fill.x, o->cur.fill.w, &idw);
             idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur.fill.y, o->cur.fill.h, &idh);
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
                         {
#ifdef EVAS_CSERVE2
                            if (evas_cserve2_use_get())
                              {
                                 Image_Entry *ie;
                                 void *data = pixels;
                                 int w = imagew, h = imageh;

                                 ie = evas_cache2_image_scale_load
                                   ((Image_Entry *)pixels,
                                    0, 0,
                                    imagew, imageh,
                                    iw, ih, o->cur.smooth_scale);
                                 if (ie != &((RGBA_Image *)pixels)->cache_entry)
                                   {
                                      data = ie;
                                      w = iw;
                                      h = ih;
                                   }

                                 obj->layer->evas->engine.func->image_draw
                                   (output, context, surface, data,
                                    0, 0,
                                    w, h,
                                    obj->cur.geometry.x + ix + x,
                                    obj->cur.geometry.y + iy + y,
                                    iw, ih,
                                    o->cur.smooth_scale);
                              }
                            else
#endif
                              {
                                 obj->layer->evas->engine.func->image_draw
                                   (output, context, surface, pixels,
                                    0, 0,
                                    imagew, imageh,
                                    obj->cur.geometry.x + ix + x,
                                    obj->cur.geometry.y + iy + y,
                                    iw, ih,
                                    o->cur.smooth_scale);
                              }
                         }
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

        // Unset img object 
        if (img_set)
          {
             if (obj->layer->evas->engine.func->gl_img_obj_set)
               {
                  obj->layer->evas->engine.func->gl_img_obj_set(output, NULL, 0);
                  img_set = 0;
               }
          }
     }
}

static void
evas_object_image_render_pre(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   int is_v = 0, was_v = 0;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   Evas_Public_Data *e = obj->layer->evas;

   if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
     {
        ERR("%p has invalid fill size: %dx%d. Ignored",
            eo_obj, o->cur.fill.w, o->cur.fill.h);
        return;
     }

   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.eo_clipper, obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.eo_clipper, obj->cur.clipper);
     }
   /* Proxy: Do it early */
   if (o->cur.source)
     {
        Evas_Object_Protected_Data *source = eo_data_get(o->cur.source, EVAS_OBJ_CLASS);
        if (source->proxy.redraw || source->changed)
          {
             /* XXX: Do I need to sort out the map here? */
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
        if (!o->pixel_updates) goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&e->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixel_updates) goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixel_updates) goto done;
     }
   /* if it changed render op */
   if (obj->cur.render_op != obj->prev.render_op)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixel_updates) goto done;
     }
   /* if it changed anti_alias */
   if (obj->cur.anti_alias != obj->prev.anti_alias)
     {
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
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
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixel_updates) goto done;
          }
        if ((o->cur.image.w != o->prev.image.w) ||
            (o->cur.image.h != o->prev.image.h) ||
            (o->cur.has_alpha != o->prev.has_alpha) ||
            (o->cur.cspace != o->prev.cspace) ||
            (o->cur.smooth_scale != o->prev.smooth_scale))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixel_updates) goto done;
          }
        if ((o->cur.border.l != o->prev.border.l) ||
            (o->cur.border.r != o->prev.border.r) ||
            (o->cur.border.t != o->prev.border.t) ||
            (o->cur.border.b != o->prev.border.b) ||
            (o->cur.border.fill != o->prev.border.fill) ||
            (o->cur.border.scale != o->prev.border.scale))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixel_updates) goto done;
          }
        if (o->dirty_pixels)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
             if (!o->pixel_updates) goto done;
          }
        if (o->cur.frame != o->prev.frame)
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
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
        evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
        if (!o->pixel_updates) goto done;
     }
   if (o->changed)
     {
        if ((o->cur.fill.x != o->prev.fill.x) ||
            (o->cur.fill.y != o->prev.fill.y) ||
            (o->cur.fill.w != o->prev.fill.w) ||
            (o->cur.fill.h != o->prev.fill.h))
          {
             evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
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

                       idx = evas_object_image_figure_x_fill(eo_obj, obj, o->cur.fill.x, o->cur.fill.w, &idw);
                       idy = evas_object_image_figure_y_fill(eo_obj, obj, o->cur.fill.y, o->cur.fill.h, &idh);

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

                                 r.x = (rr->x * w) / o->cur.image.w;
                                 r.y = (rr->y * h) / o->cur.image.h;
                                 r.w = ((rr->w * w) + (o->cur.image.w * 2) - 1) / o->cur.image.w;
                                 r.h = ((rr->h * h) + (o->cur.image.h * 2) - 1) / o->cur.image.h;
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
		  evas_object_render_pre_prev_cur_add(&e->clip_changes, eo_obj, obj);
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

        x = obj->cur.cache.clip.x;
        y = obj->cur.cache.clip.y;
        w = obj->cur.cache.clip.w;
        h = obj->cur.cache.clip.h;
        if (obj->cur.clipper)
          {
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur.clipper->cur.cache.clip.x,
                                obj->cur.clipper->cur.cache.clip.y,
                                obj->cur.clipper->cur.cache.clip.w,
                                obj->cur.clipper->cur.cache.clip.h);
          }
        e->engine.func->output_redraws_rect_del(e->engine.data.output,
                                                x, y, w, h);
     }
   done:
   evas_object_render_pre_effect_updates(&e->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_image_render_post(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);
   EINA_LIST_FREE(o->pixel_updates, r)
     eina_rectangle_free(r);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(eo_obj);
   o->prev = o->cur;
   o->changed = EINA_FALSE;
   /* FIXME: copy strings across */
}

static unsigned int evas_object_image_id_get(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static unsigned int evas_object_image_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_IMAGE;
}

static void *evas_object_image_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_image_is_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire rectangle it occupies */
/*  disable caching due tyo maps screwing with this
    o->cur.opaque_valid = 0;*/
   if (obj->cur.opaque_valid)
     {
        if (!obj->cur.opaque) return 0;
     }
   else
     {
        Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

        obj->cur.opaque = 0;
        obj->cur.opaque_valid = 1;
        if ((o->cur.fill.w < 1) || (o->cur.fill.h < 1))
          return obj->cur.opaque;
        if (((o->cur.border.l != 0) ||
             (o->cur.border.r != 0) ||
             (o->cur.border.t != 0) ||
             (o->cur.border.b != 0)) &&
            (!o->cur.border.fill))
          return obj->cur.opaque;
        if (!o->engine_data)
          return obj->cur.opaque;

        // FIXME: use proxy
        if (o->cur.source)
          {
             Evas_Object_Protected_Data *cur_source = eo_data_get(o->cur.source, EVAS_OBJ_CLASS);
             obj->cur.opaque = evas_object_is_opaque(o->cur.source, cur_source);
             return obj->cur.opaque; /* FIXME: Should go poke at the object */
          }
        if (o->cur.has_alpha)
          return obj->cur.opaque;

        obj->cur.opaque = 1;
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
               {
                  if ((m->points[0].x == obj->cur.geometry.x) &&
                      (m->points[0].y == obj->cur.geometry.y) &&
                      (m->points[2].x == (obj->cur.geometry.x + obj->cur.geometry.w)) &&
                      (m->points[2].y == (obj->cur.geometry.y + obj->cur.geometry.h)))
                    return obj->cur.opaque;
               }
          }
        obj->cur.opaque = 0;
        return obj->cur.opaque;
     }
   if (obj->cur.render_op == EVAS_RENDER_COPY)
     {
        obj->cur.opaque = 1;
        return obj->cur.opaque;
     }
   return obj->cur.opaque;
}

static int
evas_object_image_was_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire rectangle it occupies */
   if (obj->prev.opaque_valid)
     {
        if (!obj->prev.opaque) return 0;
     }
   else
     {
        Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

        obj->prev.opaque = 0;
        obj->prev.opaque_valid = 1;
        if ((o->prev.fill.w < 1) || (o->prev.fill.h < 1))
          return obj->prev.opaque;
        if (((o->prev.border.l != 0) ||
             (o->prev.border.r != 0) ||
             (o->prev.border.t != 0) ||
             (o->prev.border.b != 0)) &&
            (!o->prev.border.fill))
          return obj->prev.opaque;
        if (!o->engine_data)
          return obj->prev.opaque;

        // FIXME: use proxy
        if (o->prev.source)
          return obj->prev.opaque; /* FIXME: Should go poke at the object */
        if (o->prev.has_alpha)
          return obj->prev.opaque;

        obj->prev.opaque = 1;
     }
   if (obj->prev.usemap)
     {
        Evas_Map *m = obj->prev.map;

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
                  if ((m->points[0].x == obj->prev.geometry.x) &&
                      (m->points[0].y == obj->prev.geometry.y) &&
                      (m->points[2].x == (obj->prev.geometry.x + obj->prev.geometry.w)) &&
                      (m->points[2].y == (obj->prev.geometry.y + obj->prev.geometry.h)))
                    return obj->prev.opaque;
               }
          }

        obj->prev.opaque = 0;
        return obj->prev.opaque;
     }
   if (obj->prev.render_op == EVAS_RENDER_COPY)
     {
        obj->prev.opaque = 1;
        return obj->prev.opaque;
     }
   if (obj->prev.render_op != EVAS_RENDER_BLEND)
     {
        obj->prev.opaque = 0;
        return obj->prev.opaque;
     }
   return obj->prev.opaque;
}

static int
evas_object_image_is_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   DATA32 *data;
   int w, h, stride, iw, ih;
   int a;
   int return_value;

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
     {
        return_value = 0;
        goto finish;
     }

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
        return_value = 1;
        goto finish;
        break;
     }

   return_value = (a != 0);

finish:
   return return_value;
}

static int
evas_object_image_has_opaque_rect(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

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
evas_object_image_get_opaque_rect(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

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
evas_object_image_can_map(Evas_Object *obj EINA_UNUSED)
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
          WRN("unknow colorspace: %i\n", o->cur.cspace);
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
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   return o->cur.source;
}

Eina_Bool
_evas_object_image_preloading_get(const Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   return o->preloading;
}

void
_evas_object_image_preloading_set(Evas_Object *eo_obj, Eina_Bool preloading)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   o->preloading = preloading;
}

void
_evas_object_image_preloading_check(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   if (obj->layer->evas->engine.func->image_load_error_get)
      o->load_error = obj->layer->evas->engine.func->image_load_error_get
      (obj->layer->evas->engine.data.output, o->engine_data);
}

Evas_Object *
_evas_object_image_video_parent_get(Evas_Object *eo_obj)
{
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);
   return o->video_surface ? o->video.parent : NULL;
}

void
_evas_object_image_video_overlay_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

   if (obj->cur.cache.clip.x != obj->prev.cache.clip.x ||
       obj->cur.cache.clip.y != obj->prev.cache.clip.y ||
       o->created || !o->video_visible)
     o->video.move(o->video.data, eo_obj, &o->video, obj->cur.cache.clip.x, obj->cur.cache.clip.y);
   if (obj->cur.cache.clip.w != obj->prev.cache.clip.w ||
       obj->cur.cache.clip.h != obj->prev.cache.clip.h ||
       o->created || !o->video_visible)
     o->video.resize(o->video.data, eo_obj, &o->video, obj->cur.cache.clip.w, obj->cur.cache.clip.h);
   if (!o->video_visible || o->created)
     {
        o->video.show(o->video.data, eo_obj, &o->video);
     }
   else
     {
        /* Cancel dirty on the image */
        Eina_Rectangle *r;

        o->dirty_pixels = EINA_FALSE;
        EINA_LIST_FREE(o->pixel_updates, r)
          eina_rectangle_free(r);
     }
   o->video_visible = EINA_TRUE;
   o->created = EINA_FALSE;
}

void
_evas_object_image_video_overlay_hide(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Image *o = eo_data_get(eo_obj, MY_CLASS);

   if (o->video_visible || o->created)
     o->video.hide(o->video.data, eo_obj, &o->video);
   if (evas_object_is_visible(eo_obj, obj))
     o->video.update_pixels(o->video.data, eo_obj, &o->video);
   o->video_visible = EINA_FALSE;
   o->created = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_MEMFILE_SET), _image_memfile_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_SET), _image_file_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILE_GET), _image_file_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET), _image_source_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET), _image_source_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET), _image_border_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET), _image_border_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET), _image_border_center_fill_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET), _image_border_center_fill_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET), _image_filled_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET), _image_filled_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET), _image_border_scale_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET), _image_border_scale_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SET), _image_fill_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_GET), _image_fill_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET), _image_fill_spread_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET), _image_fill_spread_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET), _image_size_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET), _image_size_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET), _image_stride_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET), _image_load_error_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT), _image_data_convert),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_SET), _image_data_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_GET), _image_data_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PRELOAD), _image_preload),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET), _image_data_copy_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD), _image_data_update_add),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET), _image_alpha_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET), _image_alpha_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET), _image_smooth_scale_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET), _image_smooth_scale_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_RELOAD), _image_reload),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SAVE), _image_save),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT), _image_pixels_import),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET), _image_pixels_get_callback_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET), _image_pixels_dirty_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET), _image_pixels_dirty_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET), _image_load_dpi_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET), _image_load_dpi_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET), _image_load_size_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET), _image_load_size_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET), _image_load_scale_down_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET), _image_load_scale_down_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET), _image_load_region_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET), _image_load_region_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET), _image_load_orientation_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET), _image_load_orientation_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET), _image_colorspace_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET), _image_colorspace_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET), _image_video_surface_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET), _image_video_surface_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET), _image_native_surface_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET), _image_native_surface_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET), _image_scale_hint_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET), _image_scale_hint_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET), _image_content_hint_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET), _image_content_hint_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET), _image_region_support_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET), _image_animated_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET), _image_animated_frame_count_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET), _image_animated_loop_type_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET), _image_animated_loop_count_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET), _image_animated_frame_duration_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET), _image_animated_frame_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET), _image_source_visible_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET), _image_source_visible_get),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET), _image_source_events_set),
        EO_OP_FUNC(EVAS_OBJ_IMAGE_ID(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET), _image_source_events_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_MEMFILE_SET, "Sets the data for an image from memory to be loaded"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILE_SET, "Set the source file from where an image object must fetch the real"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILE_GET, "Retrieve the source file from where an image object is to fetch the"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_SET, "Set the source object on an image object to used as a @b proxy."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_GET, "Get the current source object of an image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SET, "Set the dimensions for an image object's border."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_GET, "Retrieve the dimensions for an image object's border, a region."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_SET, "Sets how the center part of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_CENTER_FILL_GET, "Retrieves how the center part of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILLED_SET, "Set whether the image object's fill property should track the object's size."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILLED_GET, "Retrieve whether the image object's fill property should track the object's size."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_SET, "Sets the scaling factor (multiplier) for the borders of an image object"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_BORDER_SCALE_GET, "Retrieves the scaling factor (multiplier) for the borders of an image object"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILL_SET, "Set how to fill an image object's drawing rectangle given the (real) image bound to it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILL_GET, "Retrieve how an image object is to fill its drawing rectangle, given the (real) image bound to it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_SET, "Sets the tiling mode for the given evas image object's fill."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_FILL_SPREAD_GET, "Retrieves the spread (tiling mode) for the given image object's fill."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SIZE_SET, "Sets the size of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SIZE_GET, "Retrieves the size of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_STRIDE_GET, "Retrieves the row stride of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ERROR_GET, "Retrieves a number representing any error."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_DATA_CONVERT, "Converts the raw image data of the given image object to the specified colorspace."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_DATA_SET, "Sets the raw image data of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_DATA_GET, "Get a pointer to the raw image data of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_PRELOAD, "Preload an image object's image data in the background."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_DATA_COPY_SET, "Replaces the raw image data of the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_DATA_UPDATE_ADD, "Mark a sub-region of the given image object to be redrawn."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_SET, "Enable or disable alpha channel usage on the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ALPHA_GET, "Retrieve whether alpha channel data is being used on the given image object"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET, "Sets whether to use high-quality image scaling algorithm on the given image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET, "Retrieves whether the given image object is using high-quality image scaling algorithm."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_RELOAD, "Reload an image object's image data."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SAVE, "Save the given image object's contents to an (image) file."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_IMPORT, "Import pixels from given source to a given canvas image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_GET_CALLBACK_SET, "Set the callback function to get pixels from a canvas' image."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_SET, "Mark whether the given image object is dirty and needs to request its pixels."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_PIXELS_DIRTY_GET, "Retrieves whether the given image object is dirty (needs to be redrawn)."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_SET, "Set the DPI resolution of an image object's source image."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_DPI_GET, "Get the DPI resolution of a loaded image object in the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET, "Set the size of a given image object's source image, when loading it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET, "Get the size of a given image object's source image, when loading it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_SET, "Set the scale down factor of a given image object's source image, when loading it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_SCALE_DOWN_GET, "get the scale down factor of a given image object's source image, when loading it."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_SET, "Inform a given image object to load a selective region of its source image."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_REGION_GET, "Retrieve the coordinates of a given image object's selective load region."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_SET, "Define if the orientation information in the image file should be honored."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_LOAD_ORIENTATION_GET, "Get if the orientation information in the image file should be honored."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_SET, "Set the colorspace of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_COLORSPACE_GET, "Get the colorspace of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_SET, "Set the video surface linked to a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_VIDEO_SURFACE_GET, "Get the video surface linekd to a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_SET, "Set the native surface of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_NATIVE_SURFACE_GET, "Get the native surface of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_SET, "Set the scale hint of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SCALE_HINT_GET, "Get the scale hint of a given image of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_SET, "Set the content hint setting of a given image object of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_CONTENT_HINT_GET, "Get the content hint setting of a given image object of the canvas."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_REGION_SUPPORT_GET, "Get the support state of a given image"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_GET, "Check if an image object can be animated (have multiple frames)"),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_COUNT_GET, "Get the total number of frames of the image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_TYPE_GET, "Get the kind of looping the image object does."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_LOOP_COUNT_GET, "Get the number times the animation of the object loops."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_DURATION_GET, "Get the duration of a sequence of frames."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_ANIMATED_FRAME_SET, "Set the frame to current frame of an image object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_SET, "Set the source object visibility of a given image object being used as a proxy."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_VISIBLE_GET, "Get the source object visibility of a given image object being used as a proxy."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_SET, "Set the events to be repeated to the source object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_IMAGE_SUB_ID_SOURCE_EVENTS_GET, "Get the state of the source events."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas_Object_Image",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_IMAGE_BASE_ID, op_desc, EVAS_OBJ_IMAGE_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Object_Image),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_image_class_get, &class_desc, EVAS_OBJ_CLASS, NULL);

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
