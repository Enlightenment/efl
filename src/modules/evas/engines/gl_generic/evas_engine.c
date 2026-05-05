#include "evas_common_private.h"
#include "evas_gl_core_private.h"

#include "software/Ector_Software.h"
#include "evas_ector_gl.h"
#include "filters/gl_engine_filter.h"

#ifdef _WIN32
# include <evil_private.h> /* dlsym */
#elif defined HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_x11 should not get compiled if dlsym is not found on the system!
#endif

#include "../gl_common/evas_gl_common.h"

#include "Evas_Engine_GL_Generic.h"

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

int _evas_engine_GL_log_dom = -1;

#undef ERR
#undef DBG
#undef INF
#undef WRN
#undef CRI
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_log_dom, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_log_dom, __VA_ARGS__)

#ifdef GL_GLES
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
#endif

static int eng_gl_image_direct_get(void *data, void *image);
static int eng_gl_surface_destroy(void *data, void *surface);
static Eina_Bool eng_gl_surface_lock(void *data, void *surface);
static Eina_Bool eng_gl_surface_unlock(void *data, void *surface);
static Eina_Bool eng_gl_surface_read_pixels(void *data, void *surface, int x, int y, int w, int h, Evas_Colorspace cspace, void *pixels);

Eina_Bool _need_context_restore = EINA_FALSE;

static Render_Output_GL_Generic *
_evgl_output_find(Render_Engine_GL_Generic *engine)
{
   Render_Output_GL_Generic *output = NULL;
   EVGL_Resource *rsc;
   Eina_List *l;

   if (engine->current)
     {
        output = engine->current;
        goto picked;
     }

   rsc = _evgl_tls_resource_get();
   if (rsc &&
       rsc->stored.data)
     {
        EINA_LIST_FOREACH(engine->software.outputs, l, output)
          if (output == rsc->stored.data) goto picked;
     }

   EINA_LIST_FOREACH(engine->software.outputs, l, output)
     {
        if (output->software.ob) goto picked;
     }
   return NULL;

 picked:
   return output;
}

static Evas_Func func, pfunc;

void
_context_restore(void)
{
   EVGL_Resource *rsc = _evgl_tls_resource_get();
   if (rsc)
     {
        if (rsc->id == evgl_engine->main_tid)
          {
             if (rsc->stored.data)
               evgl_make_current(rsc->stored.data, rsc->stored.surface, rsc->stored.context);
             _need_context_restore = EINA_FALSE;
          }
     }
}

static inline void
_context_store(void *data, void *surface, void *context)
{
   EVGL_Resource *rsc = _evgl_tls_resource_get();
   if (rsc)
     {
        if (rsc->id == evgl_engine->main_tid)
          {
             _need_context_restore = EINA_FALSE;
             rsc->stored.data = data;
             rsc->stored.surface = surface;
             rsc->stored.context = context;
          }
     }
}

static inline void
_context_stored_reset(void *data EINA_UNUSED, void *surface)
{
   EVGL_Resource *rsc = _evgl_tls_resource_get();
   if (rsc && rsc->stored.surface == surface)
     {
        _need_context_restore = EINA_FALSE;
        rsc->stored.data = NULL;
        rsc->stored.surface = NULL;
        rsc->stored.context = NULL;
     }
}

#define CONTEXT_STORE(data, surface, context) _context_store(data, surface, context)
#define CONTEXT_STORED_RESET(data, surface) _context_stored_reset(data, surface)

#ifdef GL_GLES
static void *
egl_display_get(Render_Engine_GL_Generic *engine)
{
   Render_Output_GL_Generic *output;
   Eina_List *l;

   EINA_LIST_FOREACH(engine->software.outputs, l, output)
     if (output->software.ob)
       return output->window_egl_display_get(output->software.ob);

   return NULL;
}
#endif

void eng_image_free(void *engine, void *image);

static void *
eng_engine_new(void)
{
   Render_Engine_GL_Generic *engine;

   engine = calloc(1, sizeof (Render_Engine_GL_Generic));
   if (!engine) return NULL;
   engine->software.surface_cache = generic_cache_new(engine, eng_image_free);

   /* Gradient ramp atlas: NULL return means atlas unavailable — gradient
    * shapes will be skipped per the spec error table (no-op, non-fatal). */
   engine->grad_atlas = span_grad_atlas_new();

   return engine;
}

static void
eng_engine_free(void *engine)
{
   Render_Engine_GL_Generic *e = engine;
   Render_Output_GL_Generic *output;

   generic_cache_destroy(e->software.surface_cache);

   if (e->grad_atlas) span_grad_atlas_free(e->grad_atlas);

   EINA_LIST_FREE(e->software.outputs, output)
     ERR("Output %p not properly cleaned before engine destruction.", output);
   free(e);
}

static void
eng_rectangle_draw(void *engine EINA_UNUSED, void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Output_GL_Generic *re = data;

   gl_context = gl_generic_context_get(re, 1);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_rect_draw(gl_context, x, y, w, h);
}

static void
eng_line_draw(void *engine EINA_UNUSED, void *data, void *context, void *surface, int p1x, int p1y, int p2x, int p2y, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Output_GL_Generic *re = data;

   gl_context = gl_generic_context_get(re, 1);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_line_draw(gl_context, p1x, p1y, p2x, p2y);
}

static void *
eng_polygon_point_add(void *engine EINA_UNUSED, void *polygon, int x, int y)
{
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *engine EINA_UNUSED, void *polygon)
{
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *engine EINA_UNUSED, void *data, void *context, void *surface EINA_UNUSED, void *polygon, int x, int y, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Output_GL_Generic *re = data;

   gl_context = gl_generic_context_get(re, 1);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_poly_draw(gl_context, polygon, x, y);
}

static int
eng_image_alpha_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   if (!im) return 1;
   return im->alpha;
}

static Evas_Colorspace
eng_image_colorspace_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   if (!im) return EVAS_COLORSPACE_ARGB8888;
   return im->cs.space;
}

static void *
eng_image_alpha_set(void *engine, void *image, int has_alpha)
{
   Evas_GL_Image *im;

   if (!image) return NULL;
   im = image;
   if (im->alpha == has_alpha) return image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   gl_generic_window_find(engine);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        im->alpha = has_alpha;
        im->tex->alpha = im->alpha;
        return image;
     }
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;

        if (!im->im->image.data)
          evas_cache_image_load_data(&im->im->cache_entry);
        evas_gl_common_image_alloc_ensure(im);
        im_new = evas_gl_common_image_new_from_copied_data
           (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
               im->im->image.data,
               eng_image_alpha_get(engine, image),
               eng_image_colorspace_get(engine, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return evas_gl_common_image_alpha_set(im, has_alpha ? 1 : 0);
}

static Evas_Colorspace
eng_image_file_colorspace_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   if (!im || !im->im) return EVAS_COLORSPACE_ARGB8888;
   if (im->im->cache_entry.cspaces)
     return im->im->cache_entry.cspaces[0];
   return im->im->cache_entry.space;
}

static Eina_Bool
eng_image_data_direct_get(void *engine EINA_UNUSED, void *image, int plane,
                          Eina_Slice *slice, Evas_Colorspace *cspace,
                          Eina_Bool load, Eina_Bool *tofree)
{
   Eina_Bool ret = EINA_FALSE;
   Evas_GL_Image *im = image;
   int bpp = 0;

   if (!slice || !im) return ret;

   /* If content hint is DYNAMIC, the im->im could be NULL. If the im->im does
      not exist, eng_image_data_direct_get needs to return copied dyn.data to
      make functions including efl_file_save work. */
   if ((im->content_hint == EVAS_IMAGE_CONTENT_HINT_DYNAMIC) &&
       tofree &&
       (im->tex_only) && (!im->im) &&
       (im->tex) && (im->tex->pt) && (im->tex->pt->dyn.data))
     {
        *tofree = EINA_FALSE;
        switch ( im->cs.space)
          {
           case EFL_GFX_COLORSPACE_ARGB8888:
             bpp = 4;
             EINA_FALLTHROUGH;
             // falltrhough is intended
           case EFL_GFX_COLORSPACE_AGRY88:
             if (!bpp) bpp = 2;
             EINA_FALLTHROUGH;
             // falltrhough is intended
           case EFL_GFX_COLORSPACE_GRY8:
             if (!bpp) bpp = 1;
             *tofree = EINA_TRUE;
             im->im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
             im->im->cache_entry.flags.alpha = im->alpha;
             im->im->cache_entry.space = im->cs.space;
             evas_cache_image_colorspace(&im->im->cache_entry, im->cs.space);
             im->im = (RGBA_Image *)evas_cache_image_size_set(&im->im->cache_entry, im->w, im->h);

             DATA8 *pixels = (DATA8 *)im->tex->pt->dyn.data;
             for (int i = 0; i < im->tex->pt->dyn.h; i++)
               {
                  memcpy(im->im->image.data + (im->w * i),
                         pixels + (im->tex->pt->dyn.stride * i),
                         im->w * bpp);
               }
             break;
           default: break;
          }
     }

   if (!im->im) return ret;

   if (cspace) *cspace = im->im->cache_entry.space;
   if (load)
     {
        if (evas_cache_image_load_data(&im->im->cache_entry) != 0)
          {
             /* Only valid when content hint is DYNAMIC */
             if (tofree && *tofree)
               {
                  evas_cache_image_drop(&im->im->cache_entry);
                  im->im = NULL;
               }

             return ret;
          }
     }

   ret = _evas_common_rgba_image_plane_get(im->im, plane, slice);

   /* The im->im is not necessary, because it is created temporal purpose to
      get the slice used by out side of this function. */
   if (tofree && *tofree)
     {
        if (ret)
          *slice = eina_rw_slice_slice_get(eina_slice_dup(*slice));

        evas_cache_image_drop(&im->im->cache_entry);
        im->im = NULL;
     }

   return ret;
}

static void
eng_image_colorspace_set(void *engine, void *image, Evas_Colorspace cspace)
{
   Evas_GL_Image *im;

   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   gl_generic_window_find(engine);
   evas_gl_common_image_alloc_ensure(im);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         evas_cache_image_colorspace(&im->im->cache_entry, cspace);
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
              im->cs.data = NULL;
              im->cs.no_free = 0;
           }
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         evas_cache_image_colorspace(&im->im->cache_entry, cspace);
         if (im->tex) evas_gl_common_texture_free(im->tex, EINA_TRUE);
         im->tex = NULL;
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
           }
         if (im->im->cache_entry.h > 0)
           im->cs.data =
              calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
         else
           im->cs.data = NULL;
         im->cs.no_free = 0;
         break;
      default:
         ERR("colorspace %d is not supported here", im->cs.space);
         return;
     }
   im->cs.space = cspace;
}

static void
_native_bind_cb(void *image)
{
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n = im->native.data;

   if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, n->data.opengl.texture_id);
}

static void
_native_unbind_cb(void *image)
{
  Evas_GL_Image *im = image;
  Evas_Native_Surface *n = im->native.data;

  if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void
_native_free_cb(void *image)
{
  Evas_GL_Image *im = image;
  Evas_Native_Surface *n = im->native.data;
  uint32_t texid;

  if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = n->data.opengl.texture_id;
       eina_hash_del(im->native.shared->native_tex_hash, &texid, im);
    }
  im->native.data        = NULL;
  im->native.func.bind   = NULL;
  im->native.func.unbind = NULL;
  im->native.func.free   = NULL;
  free(n);
}

static int
eng_image_native_init(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
      case EVAS_NATIVE_SURFACE_OPENGL:
        return 1;
      default:
        ERR("Native surface type %d not supported!", type);
        return 0;
     }
}

static void
eng_image_native_shutdown(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
      case EVAS_NATIVE_SURFACE_OPENGL:
        return;
      default:
        ERR("Native surface type %d not supported!", type);
        return;
     }
}

static void *
eng_image_native_set(void *engine, void *image, void *native)
{
  Evas_Engine_GL_Context *gl_context;
  Evas_Native_Surface *ns = native;
  Evas_GL_Image *im = image, *im2 = NULL;
  uint32_t texid;
  Evas_Native_Surface *n;
  unsigned int tex = 0;
  unsigned int fbo = 0;

  gl_context = gl_generic_context_find(engine, 1);

  if (!im)
    {
       if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = evas_gl_common_image_new_from_data(gl_context,
                                                    ns->data.opengl.w,
                                                    ns->data.opengl.h,
                                                    NULL, 1,
                                                    EVAS_COLORSPACE_ARGB8888);
         }
       else
         return NULL;
    }

  if (ns)
    {
      if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
        {
          tex = ns->data.opengl.texture_id;
          fbo = ns->data.opengl.framebuffer_id;
          if (im->native.data)
            {
              Evas_Native_Surface *ens = im->native.data;
              if ((ens->data.opengl.texture_id == tex) &&
                  (ens->data.opengl.framebuffer_id == fbo))
                return im;
            }
        }
    }
  gl_generic_window_find(engine);

   if (!ns)
     {
        evas_gl_common_image_free(im);
        return NULL;
     }

  if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = tex;
       im2 = eina_hash_find(gl_context->shared->native_tex_hash, &texid);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 evas_gl_common_image_ref(im2);
                 evas_gl_common_image_free(im);
                 return im2;
              }
         }

    }
  im2 = evas_gl_common_image_new_from_data(gl_context,
                                           im->w, im->h, NULL, im->alpha,
                                           EVAS_COLORSPACE_ARGB8888);
  evas_gl_common_image_free(im);
  im = im2;
  if (!im) return NULL;
  if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      if (native)
        {
          n = calloc(1, sizeof(Evas_Native_Surface));
          if (n)
            {
              memcpy(n, ns, sizeof(Evas_Native_Surface));

              eina_hash_add(gl_context->shared->native_tex_hash, &texid, im);

              im->native.yinvert     = 0;
              im->native.loose       = 0;
              im->native.shared      = gl_context->shared;
              im->native.data        = n;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;
              im->native.target      = GL_TEXTURE_2D;
              im->native.mipmap      = 0;

              // FIXME: need to implement mapping sub texture regions
              // x, y, w, h for possible texture atlasing

              evas_gl_common_image_native_enable(im);
            }
        }

    }
   return im;
}

static void *
eng_image_native_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n;

   if (!im) return NULL;
   n = im->native.data;
   if (!n) return NULL;
   return n;
}

static void *
eng_image_load(void *engine, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Evas_Engine_GL_Context *gl_context;

   *error = EVAS_LOAD_ERROR_NONE;
   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_load(gl_context, file, key, lo, error);
}

static void *
eng_image_mmap(void *engine, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Evas_Engine_GL_Context *gl_context;

   *error = EVAS_LOAD_ERROR_NONE;
   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_mmap(gl_context, f, key, lo, error);
}

static void *
eng_image_new_from_data(void *engine, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_new_from_data(gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *engine, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_new_from_copied_data(gl_context, w, h, image_data, alpha, cspace);
}

void
eng_image_free(void *engine, void *image)
{
   if (!image) return;
   gl_generic_window_find(engine);
   evas_gl_common_image_free(image);
}

static void *
eng_image_ref(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;
   if (!im) return NULL;
   im->references++;
   return im;
}

static void
eng_image_size_get(void *engine EINA_UNUSED, void *image, int *w, int *h)
{
   Evas_GL_Image *im;
   if (!image)
     {
        *w = 0;
        *h = 0;
        return;
     }
   im = image;
   if (im->orient == EVAS_IMAGE_ORIENT_90 ||
       im->orient == EVAS_IMAGE_ORIENT_270 ||
       im->orient == EVAS_IMAGE_FLIP_TRANSPOSE ||
       im->orient == EVAS_IMAGE_FLIP_TRANSVERSE)
     {
        *w = im->h;
        *h = im->w;
     }
   else
     {
        *w = im->w;
        *h = im->h;
     }
}

static void *
eng_image_size_set(void *engine, void *image, int w, int h)
{
   Evas_Engine_GL_Context *gl_context;
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;

   if (!im) return NULL;
   gl_context = gl_generic_context_find(engine, 1);
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        evas_gl_common_image_native_enable(im);
        return image;
     }
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        evas_gl_common_texture_free(im->tex, EINA_TRUE);
        im->tex = NULL;
        im->w = w;
        im->h = h;
        im->tex = evas_gl_common_texture_dynamic_new(im->gc, im);
        return image;
     }
   im_old = image;

   switch (eng_image_colorspace_get(engine, image))
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         w &= ~0x1;
         break;
      default: break;
     }

   evas_gl_common_image_alloc_ensure(im_old);
   if ((im_old->im) &&
       ((int)im_old->im->cache_entry.w == w) &&
       ((int)im_old->im->cache_entry.h == h))
     return image;
   im = evas_gl_common_image_new(gl_context, w, h,
                                 eng_image_alpha_get(engine, image),
                                 eng_image_colorspace_get(engine, image));
   evas_gl_common_image_free(im_old);
   return im;
}

static void *
eng_image_dirty_region(void *engine, void *image, int x, int y, int w, int h)
{
   Evas_GL_Image *im = image;

   if (!image) return NULL;
   if (im->native.data) return image;
   gl_generic_window_find(engine);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static Evas_GL_Image *
_rotate_image_data(Render_Engine_GL_Generic *re, Evas_GL_Image *im1)
{
   int alpha;
   Evas_GL_Image *im2;
   Evas_Engine_GL_Context *gl_context;
   RGBA_Draw_Context *dc;
   int w, h;


   w = im1->w;
   h = im1->h;

   if (im1->orient == EVAS_IMAGE_ORIENT_90 ||
       im1->orient == EVAS_IMAGE_ORIENT_270 ||
       im1->orient == EVAS_IMAGE_FLIP_TRANSPOSE ||
       im1->orient == EVAS_IMAGE_FLIP_TRANSVERSE)
     {
        w = im1->h;
        h = im1->w;
     }

   if ((w * h) <= 0) return NULL;

   alpha = eng_image_alpha_get(re, im1);
   gl_context = gl_generic_context_find(re, 1);
   im2 = evas_gl_common_image_surface_new(gl_context, w, h, alpha, EINA_FALSE);

   evas_gl_common_context_target_surface_set(gl_context, im2);

   // Create a new and temporary context
   dc = evas_common_draw_context_new();
   evas_common_draw_context_cutout_max_set
     (dc, gl_context->shared->info.tune.cutout.max);
   evas_common_draw_context_cutout_size_min_set
     (dc, gl_context->shared->info.tune.cutout_size.min);
   evas_common_draw_context_set_clip(dc, 0, 0, im2->w, im2->h);
   gl_context->dc = dc;

   // Image draw handle the rotation magically for us
   evas_gl_common_image_draw(gl_context, im1,
                             0, 0, w, h,
                             0, 0, im2->w, im2->h,
                             0);

   gl_context->dc = NULL;
   evas_common_draw_context_free(dc);

   // flush everything
   eng_gl_surface_lock(re, im2);

   // Rely on Evas_GL_Image infrastructure to allocate pixels
   im2->im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
   if (!im2->im) return NULL;
   im2->im->cache_entry.flags.alpha = !!alpha;
   evas_gl_common_image_alloc_ensure(im2);

   eng_gl_surface_read_pixels(re, im2, 0, 0, im2->w, im2->h,
                              EVAS_COLORSPACE_ARGB8888, im2->im->image.data);

   eng_gl_surface_unlock(re, im2);
   return im2;
}

void *
eng_image_data_get(void *engine, void *image, int to_write, DATA32 **image_data, int *err, Eina_Bool *tofree)
{
   Evas_GL_Image *im_new = NULL;
   Evas_GL_Image *im = image;
   int error;

   *image_data = NULL;
   if (tofree) *tofree = EINA_FALSE;
   if (err) *err = EVAS_LOAD_ERROR_NONE;

   if (!im)
     {
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
        ERR("No image provided.");
        return NULL;
     }

   if (im->native.data)
     return im;

   if ((tofree != NULL) && im->im && (im->orient != EVAS_IMAGE_ORIENT_NONE))
     goto rotate_image;

#ifdef GL_GLES
   gl_generic_window_find(engine);

   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.img) &&
       (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        if (im->tex->pt->dyn.checked_out > 0)
          {
             im->tex->pt->dyn.checked_out++;
             *image_data = im->tex->pt->dyn.data;
             return im;
          }
        if ((im->gc->shared->info.sec_tbm_surface) && (secsym_tbm_surface_map))
          {
             tbm_surface_info_s info;
             if (secsym_tbm_surface_map(im->tex->pt->dyn.buffer,
                                    TBM_SURF_OPTION_READ|TBM_SURF_OPTION_WRITE,
                                    &info))
               {
                  ERR("tbm_surface_map failed!");
                  *image_data = im->tex->pt->dyn.data = NULL;
               }
             else
               *image_data = im->tex->pt->dyn.data = (DATA32 *) info.planes[0].ptr;
          }
        else if ((im->gc->shared->info.sec_image_map) && (secsym_eglMapImageSEC))
          {
             void *disp = egl_display_get(engine);
             *image_data = im->tex->pt->dyn.data = secsym_eglMapImageSEC(disp,
                                                                         im->tex->pt->dyn.img,
                                                                         EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC,
                                                                         EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC);
          }

        if (!im->tex->pt->dyn.data)
          {
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             ERR("Ressource allocation failed.");
             return im;
          }
        im->tex->pt->dyn.checked_out++;

        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }
#else
   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.data))
     {
        *image_data = im->tex->pt->dyn.data;
        return im;
     }

   gl_generic_window_find(engine);
#endif

   /* use glReadPixels for FBOs (assume fbo > 0) */
   if (!im->im && im->tex && im->tex->pt && im->tex->pt->fb)
     {
        Eina_Bool ok;

        if (to_write)
          {
             // This could be implemented, but can't be efficient at all.
             // Apps should avoid this situation.
             ERR("Can not retrieve image data from FBO to write it back.");
             if (err) *err = EVAS_LOAD_ERROR_GENERIC;
             return NULL;
          }

        if (!tofree)
          {
             ERR("FBO image must be freed after image_data_get.");
             if (err) *err = EVAS_LOAD_ERROR_GENERIC;
             return NULL;
          }

        ok = eng_gl_surface_lock(engine, im);
        if (!ok)
          {
             if (err) *err = EVAS_LOAD_ERROR_GENERIC;
             ERR("Lock failed.");
             return NULL;
          }

        im_new = evas_gl_common_image_new_from_copied_data
              (im->gc, im->tex->w, im->tex->h, NULL,
               eng_image_alpha_get(engine, image), EVAS_COLORSPACE_ARGB8888);
        if (!im_new)
          {
             eng_gl_surface_unlock(engine, im);
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             ERR("Allocation failed.");
             return NULL;
          }

        ok = eng_gl_surface_read_pixels
              (engine, im, 0, 0, im_new->w, im_new->h,
               EVAS_COLORSPACE_ARGB8888, im_new->im->image.data);
        eng_gl_surface_unlock(engine, im);
        if (!ok)
          {
             evas_gl_common_image_free(im_new);
             if (err) *err = EVAS_LOAD_ERROR_GENERIC;
             ERR("ReadPixels failed.");
             return NULL;
          }
        *image_data = im_new->im->image.data;
        *tofree = EINA_TRUE;
        return im_new;
     }

   /* Engine can be fail to create texture after cache drop like eng_image_content_hint_set function,
        so it is need to add code which check im->im's NULL value*/

   if (!im->im)
     {
        if (tofree)
          goto rotate_image;
        else
          {
             ERR("GL image has no source data, failed to get pixel data");
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             return NULL;
          }
     }

   error = evas_cache_image_load_data(&im->im->cache_entry);

   if (err) *err = error;
   if (error != EVAS_LOAD_ERROR_NONE)
     {
        if (!im->im->image.data ||
            (im->im->cache_entry.allocated.w != (unsigned) im->w) ||
            (im->im->cache_entry.allocated.h != (unsigned) im->h))
          {
             ERR("GL image has no source data, failed to get pixel data");
             *image_data = NULL;
             return im;
          }

        if (tofree && !to_write)
          goto rotate_image;
     }

   evas_gl_common_image_alloc_ensure(im);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
      case EVAS_COLORSPACE_AGRY88:
      case EVAS_COLORSPACE_GRY8:
         if (to_write)
           {
              if (im->references > 1)
                {
                   im_new = evas_gl_common_image_new_from_copied_data
                      (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
                       im->im->image.data,
                       eng_image_alpha_get(engine, image),
                       eng_image_colorspace_get(engine, image));
                   if (!im_new)
                     {
                        if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                        return NULL;
                     }
                   evas_gl_common_image_free(im);
                   im = im_new;
                }
              else
                evas_gl_common_image_dirty(im, 0, 0, 0, 0);
           }
         *image_data = im->im->image.data;
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         *image_data = im->cs.data;
         break;
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_ETC1_ALPHA:
         ERR("This image is encoded in ETC1 or ETC2, not returning any data");
         error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         break;
      default:
         ERR("colorspace %d is not supported here", im->cs.space);
         error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
         break;
     }
   if (err) *err = error;
   return im;

rotate_image:
   // rotate data for image save
   im_new = _rotate_image_data(engine, image);
   if (!im_new)
     {
        if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        ERR("Image rotation failed.");
        return im;
     }
   *tofree = EINA_TRUE;
   *image_data = im_new->im->image.data;
   return im_new;
}

void *
eng_image_data_put(void *engine, void *image, DATA32 *image_data)
{
   Evas_GL_Image *im, *im2;

   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   gl_generic_window_find(engine);
   evas_gl_common_image_alloc_ensure(im);
   if ((im->tex) && (im->tex->pt)
       && (im->tex->pt->dyn.data)
       && (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        if (im->tex->pt->dyn.data == image_data)
          {
             if (im->tex->pt->dyn.checked_out > 0)
               {
                 im->tex->pt->dyn.checked_out--;
#ifdef GL_GLES
                 if (im->tex->pt->dyn.checked_out == 0)
                   {
                      if (im->gc->shared->info.sec_tbm_surface)
                        {
                           if (secsym_tbm_surface_unmap(im->tex->pt->dyn.buffer))
                             ERR("tbm_surface_unmap failed!");
                        }
                      else if (im->gc->shared->info.sec_image_map)
                        {
                           void *disp = egl_display_get(engine);
                           secsym_eglUnmapImageSEC(disp, im->tex->pt->dyn.img, EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
                        }
                   }
#endif
               }

             return image;
          }
        im2 = eng_image_new_from_data(engine, im->w, im->h, image_data,
                                      eng_image_alpha_get(engine, image),
                                      eng_image_colorspace_get(engine, image));
        if (!im2) return im;
        evas_gl_common_image_free(im);
        im = im2;
        evas_gl_common_image_dirty(im, 0, 0, 0, 0);
        return im;
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
      case EVAS_COLORSPACE_AGRY88:
      case EVAS_COLORSPACE_GRY8:
         if ((!im->im) || (image_data != im->im->image.data))
           {
              im2 = eng_image_new_from_data(engine, im->w, im->h, image_data,
                                            eng_image_alpha_get(engine, image),
                                            eng_image_colorspace_get(engine, image));
              if (!im2) return im;
              evas_gl_common_image_free(im);
              im = im2;
           }
         evas_gl_common_image_dirty(im, 0, 0, 0, 0);
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         if (image_data != im->cs.data)
           {
              if (im->cs.data)
                {
                   if (!im->cs.no_free) free(im->cs.data);
                }
              im->cs.data = image_data;
           }
         evas_gl_common_image_dirty(im, 0, 0, 0, 0);
         evas_gl_common_image_update(im->gc, im);
         break;
      default:
         ERR("colorspace %d is not supported here", im->cs.space);
         break;
     }
   return im;
}

static void *
eng_image_orient_set(void *engine, void *image, Evas_Image_Orient orient)
{
   Evas_GL_Image *im;
   Evas_GL_Image *im_new;

   if (!image) return NULL;
   im = image;
   if (im->orient == orient) return image;

   gl_generic_window_find(engine);

   evas_gl_common_image_update(im->gc, im);

   im_new = evas_gl_common_image_new_from_rgbaimage(im->gc, im->im, &im->load_opts, NULL);
   if (!im_new) return im;

   im_new->load_opts = im->load_opts;
   im_new->scaled = im->scaled;
   im_new->scale_hint = im->scale_hint;
   im_new->content_hint = im->content_hint;
   im_new->csize = im->csize;
   im_new->alpha = im->alpha;
   im_new->tex_only = im->tex_only;
   im_new->locked = im->locked;
   im_new->direct = im->direct;
   im_new->cached = EINA_FALSE;

   im_new->orient = orient;
   if (im->tex)
     {
        im_new->tex = im->tex;
        im_new->tex->references++;
     }

   evas_gl_common_image_free(im);
   return im_new;
}

static Evas_Image_Orient
eng_image_orient_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   if (!im) return EVAS_IMAGE_ORIENT_NONE;
   return im->orient;
}

static void
eng_image_data_preload_request(void *engine EINA_UNUSED, void *image, const Eo *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;

   evas_cache_image_preload_data(&im->cache_entry, target, evas_gl_common_image_preload_done, gim);
}

static void
eng_image_data_preload_cancel(void *engine EINA_UNUSED, void *image, const Eo *target, Eina_Bool force)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;

   evas_gl_common_image_preload_unwatch(gim);
   evas_cache_image_preload_cancel(&im->cache_entry, target, force);
//   if (gim->tex) evas_gl_preload_target_unregister(gim->tex, (Eo*) target);
}

static Eina_Bool
eng_image_draw(void *eng, void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async EINA_UNUSED)
{
   Render_Engine_GL_Generic *engine = eng;
   Evas_Engine_GL_Context *gl_context;
   Render_Output_GL_Generic *re = data;
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n;

   if (!im) return EINA_FALSE;

   n = im->native.data;

   gl_context = gl_generic_context_get(re, 1);

   if (eng_gl_image_direct_get(re, image))
     {
        void *direct_surface = NULL;

        gl_context->dc = context;
        if ((gl_context->master_clip.enabled) &&
            (gl_context->master_clip.w > 0) &&
            (gl_context->master_clip.h > 0))
          {
             // Pass the preserve flag info the evas_gl
             evgl_direct_partial_info_set(gl_context->preserve_bit);
          }

        if (n->type == EVAS_NATIVE_SURFACE_EVASGL)
          direct_surface = n->data.evasgl.surface;
        else
          {
             ERR("This native surface type is not supported for direct rendering");
             return EINA_FALSE;
          }

        // Set necessary info for direct rendering
        evgl_direct_info_set(gl_context->w,
                             gl_context->h,
                             gl_context->rot,
                             dst_x, dst_y, dst_w, dst_h,
                             gl_context->dc->clip.x,
                             gl_context->dc->clip.y,
                             gl_context->dc->clip.w,
                             gl_context->dc->clip.h,
                             gl_context->dc->render_op,
                             direct_surface);

        // Call pixel get function
        evgl_get_pixels_pre();
        engine->func.get_pixels(engine->func.get_pixels_data, engine->func.obj);
        evgl_get_pixels_post();

        // Call end tile if it's being used
        if ((gl_context->master_clip.enabled) &&
            (gl_context->master_clip.w > 0) &&
            (gl_context->master_clip.h > 0))
          {
             evgl_direct_partial_render_end();
             evgl_direct_partial_info_clear();
             gl_context->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM;
          }

        // Reset direct rendering info
        evgl_direct_info_clear();
     }
   else
     {
        evas_gl_common_context_target_surface_set(gl_context, surface);
        gl_context->dc = context;
        evas_gl_common_image_draw(gl_context, image,
                                  src_x, src_y, src_w, src_h,
                                  dst_x, dst_y, dst_w, dst_h,
                                  smooth);
     }

   return EINA_FALSE;
}

static void
eng_image_scale_hint_set(void *engine EINA_UNUSED, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static int
eng_image_scale_hint_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static Eina_Bool
eng_image_map_draw(void *engine EINA_UNUSED, void *data, void *context, void *surface, void *image,
                   RGBA_Map *m, int smooth, int level, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Evas_GL_Image *gim = image;

   if (!image) return EINA_FALSE;
   gl_context = gl_generic_context_get(data, 1);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;

   if (!((RGBA_Draw_Context*) context)->anti_alias &&
       fabsf(m->pts[0].fx - m->pts[3].fx) < FLT_EPSILON &&
       fabsf(m->pts[1].fx - m->pts[2].fx) < FLT_EPSILON &&
       fabsf(m->pts[0].fy - m->pts[1].fy) < FLT_EPSILON &&
       fabsf(m->pts[3].fy - m->pts[2].fy) < FLT_EPSILON &&
       (m->pts[0].fx <= m->pts[1].fx) &&
       (m->pts[0].fy <= m->pts[2].fy) &&
       (m->pts[0].u == 0) &&
       (m->pts[0].v == 0) &&
       (m->pts[1].u == (gim->w << FP)) &&
       (m->pts[1].v == 0) &&
       (m->pts[2].u == (gim->w << FP)) &&
       (m->pts[2].v == (gim->h << FP)) &&
       (m->pts[3].u == 0) &&
       (m->pts[3].v == (gim->h << FP)) &&
       (m->pts[0].col == 0xffffffff) &&
       (m->pts[1].col == 0xffffffff) &&
       (m->pts[2].col == 0xffffffff) &&
       (m->pts[3].col == 0xffffffff))
     {
        int dx, dy, dw, dh;
        dx = m->pts[0].x >> FP;
        dy = m->pts[0].y >> FP;
        dw = (m->pts[2].x >> FP) - dx;
        dh = (m->pts[2].y >> FP) - dy;
        eng_image_draw(engine, data, context, surface, image,
                       0, 0, gim->w, gim->h, dx, dy, dw, dh, smooth, do_async);
     }
   else
     {
        evas_gl_common_image_map_draw(gl_context, image, m->count, &m->pts[0],
                                      smooth, level);
     }

   return EINA_FALSE;
}

static void
eng_image_map_clean(void *engine EINA_UNUSED, RGBA_Map *m EINA_UNUSED)
{
}

static void *
eng_image_map_surface_new(void *engine, int w, int h, int alpha)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_surface_new(gl_context, w, h, alpha, EINA_FALSE);
}

void *
eng_image_scaled_update(void *engine EINA_UNUSED, void *scaled, void *image,
                        int dst_w, int dst_h, Eina_Bool smooth,
                        Evas_Colorspace cspace EINA_UNUSED)
{
   return evas_gl_common_image_virtual_scaled_get(scaled, image, dst_w, dst_h, smooth);
}

static void
eng_image_content_hint_set(void *engine, void *image, int hint)
{
   gl_generic_window_find(engine);
   evas_gl_common_image_content_hint_set(image, hint);
}

static void
eng_image_cache_flush(void *engine)
{
   Evas_Engine_GL_Context *gl_context;
   int tmp_size;

   gl_context = gl_generic_context_find(engine, 1);
   if (!gl_context) return;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_gl_common_image_cache_flush(gl_context);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *engine, int bytes)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);

   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
   if (gl_context) evas_gl_common_image_cache_flush(gl_context);
}

static int
eng_image_cache_get(void *engine EINA_UNUSED)
{
   return evas_common_image_get_cache();
}

static void
eng_font_cache_flush(void *engine)
{
   int tmp_size;

   gl_generic_window_find(engine);
   tmp_size = evas_common_font_cache_get();
   evas_common_font_cache_set(0);
   evas_common_font_flush();
   evas_common_font_cache_set(tmp_size);
}

static void
eng_font_cache_set(void *engine, int bytes)
{
   gl_generic_window_find(engine);
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *engine)
{
   gl_generic_window_find(engine);
   return evas_common_font_cache_get();
}

static void
eng_image_stride_get(void *engine EINA_UNUSED, void *image, int *stride)
{
   Evas_GL_Image *im = image;

   if ((im->tex) && (im->tex->pt->dyn.img))
     *stride = im->tex->pt->dyn.stride;
   else
     {
        switch (im->cs.space)
          {
           case EVAS_COLORSPACE_ARGB8888:
             *stride = im->w * 4;
             return;
           case EVAS_COLORSPACE_AGRY88:
             *stride = im->w * 2;
             return;
           case EVAS_COLORSPACE_GRY8:
             *stride = im->w * 1;
             return;
           case EVAS_COLORSPACE_YCBCR422P601_PL:
           case EVAS_COLORSPACE_YCBCR422P709_PL:
           case EVAS_COLORSPACE_YCBCR422601_PL:
           case EVAS_COLORSPACE_YCBCR420NV12601_PL:
           case EVAS_COLORSPACE_YCBCR420TM12601_PL:
             *stride = im->w * 1;
             return;
             /* the strides below are approximations, since stride doesn't
              * really make sense for ETC & S3TC */
           case EVAS_COLORSPACE_ETC1:
           case EVAS_COLORSPACE_RGB8_ETC2:
           case EVAS_COLORSPACE_RGB_S3TC_DXT1:
           case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
             *stride = (im->w + 2 + 3) / 4 * (8 / 4);
             return;
           case EVAS_COLORSPACE_ETC1_ALPHA:
           case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
           case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
           case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
           case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
           case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
             *stride = (im->w + 2 + 3) / 4 * (16 / 4);
             return;
           default:
             ERR("Requested stride on an invalid format %d", im->cs.space);
             *stride = 0;
             return;
          }
     }
}

static Eina_Bool
eng_font_draw(void *engine EINA_UNUSED, void *data, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Text_Props *intl_props, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_get(data, 1);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
     {
        if (!gl_context->font_surface)
          gl_context->font_surface = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        gl_context->font_surface->cache_entry.w = gl_context->shared->w;
        gl_context->font_surface->cache_entry.h = gl_context->shared->h;

        evas_common_draw_context_font_ext_set(context,
                                              gl_context,
                                              evas_gl_font_texture_new,
                                              evas_gl_font_texture_free,
                                              evas_gl_font_texture_draw,
                                              evas_gl_font_image_new,
                                              evas_gl_font_image_free,
                                              evas_gl_font_image_draw);
        evas_common_font_draw_prepare(intl_props);
        evas_common_font_draw(gl_context->font_surface, context, x, y, intl_props->glyphs);
        evas_common_draw_context_font_ext_set(context,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL);
     }

   return EINA_FALSE;
}

//--------------------------------//
// Evas GL Related Code
static inline Eina_Bool
evgl_init_do(Render_Engine_GL_Generic *engine,
             Render_Output_GL_Generic *output)
{
   if (engine->evgl_initted) return EINA_TRUE;
   if (!evgl_engine_init(output, output->evgl_funcs))
     return EINA_FALSE;
   engine->current = output;
   engine->evgl_initted = EINA_TRUE;
   return EINA_TRUE;
}

static Render_Output_GL_Generic *
evgl_init(Render_Engine_GL_Generic *engine)
{
   Render_Output_GL_Generic *output = NULL;
   Eina_List *l;

   if (engine->evgl_initted)
     {
        if (engine->current) return engine->current;

        EINA_LIST_FOREACH(engine->software.outputs, l, output)
          if (output->software.ob) return output;

        ERR("Evas_GL backend initializeod, but no window found !");
        return NULL;
     }

   EINA_LIST_FOREACH(engine->software.outputs, l, output)
     {
        if (!output->software.ob) continue;
        if (evgl_init_do(engine, output))
          return output;
     }

   return NULL;
}

#define EVGLINIT(_ret) Render_Output_GL_Generic *re; if ((re = evgl_init(engine)) == NULL) return _ret

static Eina_Bool
eng_gl_supports_evas_gl(void *engine EINA_UNUSED)
{
   // Evas GL should always work... But let's do a full init anyway.
   EVGLINIT(EINA_FALSE);
   return EINA_TRUE;
}

static void *
eng_gl_output_set(void *eng, void *output)
{
   Render_Engine_GL_Generic *engine = eng;
   Render_Output_GL_Generic *previous = engine->current;

   engine->current = output;

   return previous;
}

static void *
eng_gl_surface_create(void *engine, void *config, int w, int h)
{
   Evas_GL_Config *cfg = (Evas_GL_Config *)config;

   EVGLINIT(NULL);
   return evgl_surface_create(re, cfg, w, h);
}

static void *
eng_gl_pbuffer_surface_create(void *engine, void *config, int w, int h, const int *attrib_list)
{
   Evas_GL_Config *cfg = (Evas_GL_Config *)config;

   EVGLINIT(NULL);
   return evgl_pbuffer_surface_create(re, cfg, w, h, attrib_list);
}

static int
eng_gl_surface_destroy(void *engine, void *surface)
{
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;
   Render_Engine_GL_Generic *e = engine;

   EVGLINIT(0);
   if (e->current == re) e->current = NULL;
   CONTEXT_STORED_RESET(re, surface);
   return evgl_surface_destroy(re, sfc);
}

static void *
eng_gl_context_create(void *engine, void *share_context, int version,
                      void *(*native_context_get)(void *),
                      void *(*engine_data_get)(void *))
{
   EVGL_Context  *sctx = (EVGL_Context *)share_context;

   EVGLINIT(NULL);
   return evgl_context_create(re, sctx, version, native_context_get, engine_data_get);
}

static int
eng_gl_context_destroy(void *engine, void *context)
{
   EVGL_Context  *ctx = (EVGL_Context *)context;

   EVGLINIT(0);
   return evgl_context_destroy(re, ctx);
}

static int
eng_gl_make_current(void *eng, void *surface, void *context)
{
   Render_Engine_GL_Generic *engine = eng;
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;
   EVGL_Context  *ctx = (EVGL_Context *)context;
   Render_Output_GL_Generic *output;
   int ret = 0;

   if (sfc && ctx && eina_main_loop_is())
     {
        Evas_Engine_GL_Context *gl_context;

        gl_context = gl_generic_context_find(engine, 0);
        if ((gl_context->havestuff) ||
            (gl_context->master_clip.used))
          {
             gl_context = gl_generic_context_find(engine, 1);
             evas_gl_common_context_flush(gl_context);
             if (gl_context->master_clip.used)
                evas_gl_common_context_done(gl_context);
          }
     }

   output = _evgl_output_find(engine);
   if (!output) return ret;

   ret = evgl_make_current(output, sfc, ctx);
   CONTEXT_STORE(output, surface, context);

   return ret;
}

static void *
eng_gl_current_surface_get(void *engine EINA_UNUSED)
{
   EVGL_Context *ctx;

   ctx = evas_gl_common_current_context_get();
   if (!ctx)
     return NULL;

   // Note: We could verify with a call to eglGetCurrentSurface

   return ctx->current_sfc;
}

static int
eng_gl_rotation_angle_get(void *eng)
{
   Render_Engine_GL_Generic *engine = eng;
   Render_Output_GL_Generic *output;

   if (!evgl_engine->funcs->rotation_angle_get) return 0;
   if (!_evgl_direct_enabled()) return 0;

   // It would be better if that this API was called Evas Output
   output = _evgl_output_find(engine);
   if (!output) return 0;

   return evgl_engine->funcs->rotation_angle_get(output);
}

static const char *
eng_gl_string_query(void *engine, int name)
{
   EVGLINIT(NULL);
   return evgl_string_query(name);
}

static void *
eng_gl_proc_address_get(void *engine, const char *name)
{
   EVGLINIT(NULL);
   void *fun = NULL;

   if (!evgl_safe_extension_get(name, &fun))
     {
        DBG("The extension '%s' is not safe to use with Evas GL or is not "
            "supported on this platform.", name);
        return NULL;
     }

   if (fun)
     return fun;

   if (re->evgl_funcs && re->evgl_funcs->proc_address_get)
     return re->evgl_funcs->proc_address_get(name);

   return NULL;
}

static int
eng_gl_native_surface_get(void *engine EINA_UNUSED, void *surface, void *native_surface)
{
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;
   Evas_Native_Surface *ns = (Evas_Native_Surface *)native_surface;

   return evgl_native_surface_get(sfc, ns);
}

static void *
eng_gl_api_get(void *engine, int version)
{
   Render_Output_GL_Generic *output;
   Evas_Engine_GL_Context *gl_context;
   void *ret;
   EVGLINIT(NULL);

   gl_context = gl_generic_context_find(engine, 0);
   if (!gl_context)
     {
        ERR("Invalid context!");
        return NULL;
     }
   if ((version == EVAS_GL_GLES_3_X) && (gl_context->gles_version != EVAS_GL_GLES_3_X))
     {
        ERR("Version not supported!");
        return NULL;
     }

   output = _evgl_output_find(engine);
   ret = evgl_api_get(output, version, EINA_TRUE);

   //Disable GLES3 support if symbols not present
   if ((!ret) && (version == EVAS_GL_GLES_3_X))
     gl_context->gles_version--;

   return ret;
}


static void
eng_gl_direct_override_get(void *engine, Eina_Bool *override, Eina_Bool *force_off)
{
   EVGLINIT();
   evgl_direct_override_get(override, force_off);
}

static Eina_Bool
eng_gl_surface_direct_renderable_get(void *eng, void *output, Evas_Native_Surface *ns, Eina_Bool *override, void *surface)
{
   Render_Engine_GL_Generic *engine = eng;
   Render_Output_GL_Generic *re = output;
   Eina_Bool direct_render, client_side_rotation;
   Evas_Engine_GL_Context *gl_context;
   Evas_GL_Image *sfc = surface;

   if (!re) return EINA_FALSE;
   if (!evgl_init_do(engine, re))
     return EINA_FALSE;
   if (!ns) return EINA_FALSE;
   if (!evgl_native_surface_direct_opts_get(ns, &direct_render, &client_side_rotation, override))
     return EINA_FALSE;

   if (!direct_render)
     return EINA_FALSE;

   if ((re->software.outbuf_get_rot(re->software.ob) != 0) && (!client_side_rotation))
     return EINA_FALSE;

   gl_context = gl_generic_context_get(re, 0);
   if (gl_context->def_surface != sfc)
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
eng_gl_get_pixels_set(void *eng, void *get_pixels, void *get_pixels_data, void *obj)
{
   Render_Engine_GL_Generic *engine = eng;

   engine->func.get_pixels = get_pixels;
   engine->func.get_pixels_data = get_pixels_data;
   engine->func.obj = (Evas_Object*)obj;
}

static void
eng_gl_get_pixels_pre(void *e, void *o)
{
   Render_Engine_GL_Generic *engine = e;
   Render_Output_GL_Generic *output = o;

   if (!evgl_init_do(engine, output))
     return ;
   evgl_get_pixels_pre();
}

static void
eng_gl_get_pixels_post(void *e EINA_UNUSED, void *o EINA_UNUSED)
{
   evgl_get_pixels_post();
}

static Eina_Bool
eng_gl_surface_lock(void *engine EINA_UNUSED, void *surface)
{
   Evas_GL_Image *im = surface;

   if (!im || !im->tex || !im->tex->pt)
     {
        ERR("Can not lock image that is not a surface!");
        return EINA_FALSE;
     }

   evas_gl_common_context_flush(im->gc);
   im->locked = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
eng_gl_surface_unlock(void *engine EINA_UNUSED, void *surface)
{
   Evas_GL_Image *im = surface;

   im->locked = EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
eng_gl_surface_read_pixels(void *engine EINA_UNUSED, void *surface,
                           int x, int y, int w, int h,
                           Evas_Colorspace cspace, void *pixels)
{
   Evas_GL_Image *im = surface;
   GLint fmt = GL_BGRA, fbo = 0;
   int done = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pixels, EINA_FALSE);

   if (!im->locked)
     {
        // For now, this is useless, but let's force clients to lock :)
        CRI("The surface must be locked before reading its pixels!");
        return EINA_FALSE;
     }

   if (cspace != EVAS_COLORSPACE_ARGB8888)
     {
        ERR("Conversion to colorspace %d is not supported!", (int) cspace);
        return EINA_FALSE;
     }

   /* Since this is an FBO, the pixels are already in the right Y order.
    * But some devices don't support GL_BGRA, so we still need to convert.
    */

   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
   if (fbo != (GLint) im->tex->pt->fb)
     glsym_glBindFramebuffer(GL_FRAMEBUFFER, im->tex->pt->fb);
   glPixelStorei(GL_PACK_ALIGNMENT, 4);

   // With GLX we will try to read BGRA even if the driver reports RGBA
#if defined(GL_GLES) && defined(GL_IMPLEMENTATION_COLOR_READ_FORMAT)
   glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &fmt);
#endif

   if ((im->tex->pt->format == GL_BGRA) && (fmt == GL_BGRA))
     {
        glReadPixels(x, y, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        done = (glGetError() == GL_NO_ERROR);
#ifdef WORDS_BIGENDIAN
        if (done)
          {
             DATA32 *ptr = pixels;
             int k;
             for (k = w * h; k; --k)
               {
                  const DATA32 v = *ptr;
                  *ptr++ = eina_swap32(v);
               }
          }
#endif
     }

   if (!done)
     {
        DATA32 *ptr = pixels;
        int k;

        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        for (k = w * h; k; --k)
          {
             const DATA32 v = *ptr;
#ifdef WORDS_BIGENDIAN
             *ptr++ = (v << 24) | (v >> 8);
#else
             *ptr++ = (v & 0xFF00FF00)
                   | ((v & 0x00FF0000) >> 16)
                   | ((v & 0x000000FF) << 16);
#endif
          }
     }

   if (fbo != (GLint) im->tex->pt->fb)
     glsym_glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   return EINA_TRUE;
}

static Eina_Bool
eng_gl_surface_query(void *eng, void *surface, int attr, void *value)
{
   Render_Engine_GL_Generic *engine = eng;
   Render_Output_GL_Generic *re;
   EVGL_Surface  *sfc = surface;

   re = _evgl_output_find(engine);
   if (!re) return EINA_FALSE;

#ifdef GL_GLES
   if (sfc->pbuffer.is_pbuffer)
     {
        // This is a real EGL surface, let's just call EGL directly
        int val;
        Eina_Bool ok;
        void *disp;

        disp = egl_display_get(engine);
        ok = eglQuerySurface(disp, sfc->pbuffer.native_surface, attr, &val);
        if (!ok) return EINA_FALSE;
        switch (attr)
          {
           case EVAS_GL_TEXTURE_FORMAT:
             if (val == EGL_TEXTURE_RGB)
               *((int *) value) = EVAS_GL_RGB_888;
             else if (val == EGL_TEXTURE_RGBA)
               *((int *) value) = EVAS_GL_RGBA_8888;
             else // if (val == EGL_NO_TEXTURE)
               *((int *) value) = EVAS_GL_NO_FBO;
             break;
           case EVAS_GL_TEXTURE_TARGET:
             if (val == EGL_TEXTURE_2D)
               *((int *) value) = val;
             else
               *((int *) value) = 0;
             break;
           default:
             *((int *) value) = val;
             break;
          }
        return EINA_TRUE;
     }
   else
     {
        // Since this is a fake surface (shared with evas), we must filter the
        // queries...
        switch (attr)
          {
           // TODO: Add support for whole config get
           /*
           case EVAS_GL_CONFIG_ID:
             *((int *) value) = sfc->cfg_index;
             return EINA_TRUE;
             */
           case EVAS_GL_WIDTH:
             *((int *) value) = sfc->w;
             return EINA_TRUE;
           case EVAS_GL_HEIGHT:
             *((int *) value) = sfc->h;
             return EINA_TRUE;
           case EVAS_GL_TEXTURE_FORMAT:
             // FIXME: Check the possible color formats
             if (sfc->color_buf)
               {
                  if ((sfc->color_fmt == GL_RGBA) || (sfc->color_fmt == GL_BGRA))
                    {
                       *((Evas_GL_Color_Format *) value) = EVAS_GL_RGBA_8888;
                       return EINA_TRUE;
                    }
                  else if (sfc->color_fmt == GL_RGB)
                    {
                       *((Evas_GL_Color_Format *) value) = EVAS_GL_RGB_888;
                       return EINA_TRUE;
                    }
               }
             *((Evas_GL_Color_Format *) value) = EVAS_GL_NO_FBO;
             return EINA_TRUE;
           case EVAS_GL_TEXTURE_TARGET:
             if (sfc->color_buf)
               *((int *) value) = EVAS_GL_TEXTURE_2D;
             else
               *((int *) value) = 0;
             return EINA_TRUE;
           // TODO: Add support for this:
           /*
           case EVAS_GL_MULTISAMPLE_RESOLVE:
             *((int *) value) = sfc->msaa_samples;
             return EINA_TRUE;
             */
           // TODO: Add support for mipmaps
           /*
           case EVAS_GL_MIPMAP_TEXTURE:
           case EVAS_GL_MIPMAP_LEVEL:
             return eglQuerySurface(re->win->egl_disp, re->win->egl_surface,
                                    attr, (int *) value);
             */
           default: break;
          }
        evas_gl_common_error_set(EVAS_GL_BAD_ATTRIBUTE);
        return EINA_FALSE;
     }
#else
   (void) re; (void) sfc; (void) attr; (void) value;
   ERR("GLX support for surface_query is not implemented!");
   return EINA_FALSE;
#endif
}

static int
eng_gl_image_direct_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;
   if (!im) return EINA_FALSE;
   return im->direct;
}

static void
eng_gl_image_direct_set(void *eng, void *image, Eina_Bool direct)
{
   Render_Engine_GL_Generic *engine = eng;
   Evas_GL_Image *im = image;

   if (!im) return;
   if (im->native.data && direct && engine->func.get_pixels)
     im->direct = EINA_TRUE;
   else
     im->direct = EINA_FALSE;
}

//--------------------------------//

static int
eng_image_load_error_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->im->cache_entry.load_error;
}

static Eina_Bool
eng_image_animated_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   return im->animated.animated;
}

static int
eng_image_animated_frame_count_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->animated.animated) return -1;
   return im->animated.frame_count;
}

static Evas_Image_Animated_Loop_Hint
eng_image_animated_loop_type_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   im = (Image_Entry *)gim->im;
   if (!im) return EVAS_IMAGE_ANIMATED_HINT_NONE;

   if (!im->animated.animated) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   return im->animated.loop_hint;
}

static int
eng_image_animated_loop_count_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->animated.animated) return -1;
   return im->animated.loop_count;
}

static double
eng_image_animated_frame_duration_get(void *engine EINA_UNUSED, void *image, int start_frame, int frame_num)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->animated.animated) return -1;
   return evas_common_load_rgba_image_frame_duration_from_file(im, start_frame, frame_num);
}

static Eina_Bool
eng_image_animated_frame_set(void *engine EINA_UNUSED, void *image, int frame_index)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   if (!im->animated.animated) return EINA_FALSE;
   if (im->animated.cur_frame == frame_index) return EINA_FALSE;

   im->animated.cur_frame = frame_index;
   return EINA_TRUE;
}

static int
eng_image_animated_frame_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   if (!im->animated.animated) return EINA_FALSE;

   return im->animated.cur_frame;
}

static Eina_Bool
eng_image_can_region_get(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;
   return ((Evas_Image_Load_Func*) im->info.loader)->do_region;
}

static Eina_Bool
eng_image_stretch_region_get(void *engine EINA_UNUSED, void *image,
                             uint8_t **horizontal, uint8_t **vertical)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim || !gim->im) return EINA_FALSE;

   im = (RGBA_Image *)gim->im;

   if (!im->cache_entry.need_data) return EINA_FALSE;

   if (!im->image.data) evas_cache_image_load_data(&im->cache_entry);

   if (!im->cache_entry.stretch.horizontal.region ||
       !im->cache_entry.stretch.vertical.region)
     return EINA_FALSE;

   *horizontal = im->cache_entry.stretch.horizontal.region;
   *vertical = im->cache_entry.stretch.vertical.region;
   return EINA_TRUE;
}

static void
eng_image_max_size_get(void *engine, int *maxw, int *maxh)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 0);
   if (maxw) *maxw = gl_context->shared->info.max_texture_size;
   if (maxh) *maxh = gl_context->shared->info.max_texture_size;
}

static Eina_Bool
eng_pixel_alpha_get(void *image, int x, int y, DATA8 *alpha, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h)
{
   Evas_GL_Image *im = image;
   int px, py, dx, dy, sx, sy, src_w, src_h;
   double scale_w, scale_h;

   if (!im) return EINA_FALSE;

   if ((dst_region_x > x) || (x >= (dst_region_x + dst_region_w)) ||
       (dst_region_y > y) || (y >= (dst_region_y + dst_region_h)))
     {
        *alpha = 0;
        return EINA_FALSE;
     }

   evas_gl_common_image_alloc_ensure(im);
   if (!im->im) return EINA_FALSE;

   src_w = im->im->cache_entry.w;
   src_h = im->im->cache_entry.h;
   if ((src_w == 0) || (src_h == 0))
     {
        *alpha = 0;
        return EINA_TRUE;
     }

   EINA_SAFETY_ON_TRUE_GOTO(src_region_x < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_x + src_region_w > src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y + src_region_h > src_h, error_oob);

   scale_w = (double)dst_region_w / (double)src_region_w;
   scale_h = (double)dst_region_h / (double)src_region_h;

   /* point at destination */
   dx = x - dst_region_x;
   dy = y - dst_region_y;

   /* point at source */
   sx = dx / scale_w;
   sy = dy / scale_h;

   /* pixel point (translated) */
   px = src_region_x + sx;
   py = src_region_y + sy;
   EINA_SAFETY_ON_TRUE_GOTO(px >= src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(py >= src_h, error_oob);

   switch (im->im->cache_entry.space)
     {
     case EVAS_COLORSPACE_ARGB8888:
       {
          DATA32 *pixel;

          evas_cache_image_load_data(&im->im->cache_entry);
          if (!im->im->cache_entry.flags.loaded)
            {
               ERR("im %p has no pixels loaded yet", im);
               return EINA_FALSE;
            }

          pixel = im->im->image.data;
          pixel += ((py * src_w) + px);
          *alpha = ((*pixel) >> 24) & 0xff;
       }
       break;

     default:
        ERR("Colorspace %d not supported.", im->im->cache_entry.space);
        *alpha = 0;
     }

   return EINA_TRUE;

 error_oob:
   ERR("Invalid region src=(%d, %d, %d, %d), dst=(%d, %d, %d, %d), image=%dx%d",
       src_region_x, src_region_y, src_region_w, src_region_h,
       dst_region_x, dst_region_y, dst_region_w, dst_region_h,
       src_w, src_h);
   *alpha = 0;
   return EINA_TRUE;
}

static void *
eng_context_new(void *engine)
{
   Render_Engine_GL_Generic *re = engine;
   Evas_Engine_GL_Context *gl_context;
   void *ctx;

   gl_context = gl_generic_context_find(re, 1);

   ctx = pfunc.context_new(&re->software);
   evas_common_draw_context_cutout_max_set
     (ctx, gl_context->shared->info.tune.cutout.max);
   evas_common_draw_context_cutout_size_min_set
     (ctx, gl_context->shared->info.tune.cutout_size.min);
   return ctx;
}

static void
eng_context_flush(void *engine)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);

   if ((gl_context->havestuff) ||
     (gl_context->master_clip.used))
   {
      evas_gl_common_context_flush(gl_context);
      if (gl_context->master_clip.used)
         evas_gl_common_context_done(gl_context);
   }
}

static void
eng_context_clip_image_unset(void *engine EINA_UNUSED, void *context)
{
   RGBA_Draw_Context *ctx = context;
   Evas_GL_Image *im = ctx->clip.mask;

   evas_gl_common_image_free(im);

   ctx->clip.mask = NULL;
}

static void
eng_context_clip_image_set(void *engine, void *context, void *surface, int x, int y,
                           Evas_Public_Data *evas, Eina_Bool do_async)
{
   RGBA_Draw_Context *ctx = context;
   Evas_GL_Image *im = surface;
   Eina_Bool noinc = EINA_FALSE;

   if (ctx->clip.mask)
     {
        if (ctx->clip.mask != surface)
          eng_context_clip_image_unset(engine, context);
        else
          noinc = EINA_TRUE;
     }

   ctx->clip.mask = surface;
   ctx->clip.mask_x = x;
   ctx->clip.mask_y = y;

   // useless in gl since the engines are sync only
   ctx->clip.evas = evas;
   ctx->clip.async = do_async;

   if (im)
     {
        if (!noinc) evas_gl_common_image_ref(im);
        RECTS_CLIP_TO_RECT(ctx->clip.x, ctx->clip.y, ctx->clip.w, ctx->clip.h,
                           x, y, im->w, im->h);
     }
}

static void
eng_context_clip_image_get(void *engine EINA_UNUSED, void *context, void **ie, int *x, int *y)
{
   RGBA_Draw_Context *ctx = context;

   if (ie)
     {
        Evas_GL_Image *im = ctx->clip.mask;

        *ie = im;
        if (im) evas_gl_common_image_ref(im);
     }
   if (x) *x = ctx->clip.mask_x;
   if (y) *y = ctx->clip.mask_y;
}

static void
eng_context_free(void *engine, void *context)
{
   RGBA_Draw_Context *ctx = context;

   if (!ctx) return;
   if (ctx->clip.mask)
     eng_context_clip_image_unset(engine, context);
   evas_common_draw_context_free(context);
}

static void *
eng_context_dup(void *engine EINA_UNUSED, void *context)
{
   RGBA_Draw_Context *ctx;

   ctx = evas_common_draw_context_dup(context);
   if (ctx->clip.mask)
     evas_gl_common_image_ref(ctx->clip.mask);

   return ctx;
}

static Ector_Surface *
eng_ector_create(void *engine EINA_UNUSED)
{
   Ector_Surface *ector;

   efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
   ector = efl_add_ref(ECTOR_SOFTWARE_SURFACE_CLASS, NULL);
   efl_domain_current_pop();
   return ector;
}

static Ector_Buffer *
eng_ector_buffer_wrap(void *engine EINA_UNUSED, Evas *evas, void *engine_image)
{
   Evas_GL_Image *im = engine_image;
   Render_Output_GL_Generic *output;

   EINA_SAFETY_ON_NULL_RETURN_VAL(engine_image, NULL);
   output = _evgl_output_find(engine);
   if (!output) return NULL;

   return efl_add(EVAS_ECTOR_GL_IMAGE_BUFFER_CLASS, evas,
                  evas_ector_buffer_engine_image_set(efl_added, output, im));
}

//FIXME: Currently Ector GL doens't work properly. Use software instead.
#include "../software_generic/evas_ector_software.h"
/* ector_software_private.h is on the include path via engine_include_dir in
 * meson.build; include it before evas_ector_gl_span.h so the span header
 * sees the full Span_Data definition (not just the forward declaration). */
#include "ector_software_private.h"
#include "ector_software_gradient.h"
#include "evas_ector_log_restore.h"
#include "evas_ector_gl_span.h"

static void
eng_ector_destroy(void *engine EINA_UNUSED, Ector_Surface *ector)
{
   if (ector)
     {
        /* Free all per-shape fill and stroke collectors. */
        Ector_Software_Surface_Data *pd = efl_data_scope_get(ector, ECTOR_SOFTWARE_SURFACE_CLASS);
        if (pd)
          {
             int ci;
             for (ci = 0; ci < pd->span_collectors_fill_alloc; ci++)
               {
                  Span_Collector *sc = (Span_Collector *)pd->span_collectors_fill[ci];
                  if (sc)
                    {
                       span_collector_delete_textures(sc);
                       span_collector_free(sc);
                    }
               }
             free(pd->span_collectors_fill);
             pd->span_collectors_fill       = NULL;
             pd->span_collectors_fill_count  = 0;
             pd->span_collectors_fill_alloc  = 0;

             for (ci = 0; ci < pd->span_collectors_stroke_alloc; ci++)
               {
                  Span_Collector *sc = (Span_Collector *)pd->span_collectors_stroke[ci];
                  if (sc)
                    {
                       span_collector_delete_textures(sc);
                       span_collector_free(sc);
                    }
               }
             free(pd->span_collectors_stroke);
             pd->span_collectors_stroke       = NULL;
             pd->span_collectors_stroke_count  = 0;
             pd->span_collectors_stroke_alloc  = 0;
          }
        efl_unref(ector);
     }
}

static Ector_Buffer *
eng_ector_buffer_new(void *engine, Evas *evas, int w, int h,
                     Efl_Gfx_Colorspace cspace, Ector_Buffer_Flag flags)
{
   /* FIXME: This condition is tricky, this buffer could be used for masking
    * buffer by vector, Require to use software drawing.
    */
   if (flags != (ECTOR_BUFFER_FLAG_DRAWABLE | ECTOR_BUFFER_FLAG_CPU_READABLE | ECTOR_BUFFER_FLAG_CPU_WRITABLE))
      {
         return efl_add(EVAS_ECTOR_GL_BUFFER_CLASS, evas,
                        evas_ector_gl_buffer_prepare(efl_added, engine, w, h, cspace, flags));
      }
   else
     {
        Ector_Buffer *buf;
        Image_Entry *ie;
        void *pixels;
        int pxs;

        if (cspace == EFL_GFX_COLORSPACE_ARGB8888)
          pxs = 4;
        else if (cspace == EFL_GFX_COLORSPACE_GRY8)
          pxs = 1;
        else
          {
             ERR("Unsupported colorspace: %d", (int) cspace);
             return NULL;
          }

        // alloc buffer
        ie = evas_cache_image_copied_data(evas_common_image_cache_get(), w, h,
                                          NULL, EINA_TRUE,
                                          (Evas_Colorspace)cspace);
        if (!ie) return NULL;
        pixels = ((RGBA_Image *) ie)->image.data;
        memset(pixels, 0, w * h * pxs);

        if (!efl_domain_current_push(EFL_ID_DOMAIN_SHARED))
          {
             evas_cache_image_drop(ie);
             return NULL;
          }

        buf = efl_add_ref(EVAS_ECTOR_SOFTWARE_BUFFER_CLASS, NULL,
                          evas_ector_buffer_engine_image_set(efl_added, engine, ie));
        efl_domain_current_pop();

        evas_cache_image_drop(ie);

        return buf;
     }
}

static void
eng_ector_renderer_draw(void *engine EINA_UNUSED, void *surface,
                        void *context EINA_UNUSED, Ector_Renderer *renderer,
                        Eina_Array *clips EINA_UNUSED, Eina_Bool do_async EINA_UNUSED)
{
   int w, h;
   Eina_Rectangle *r;
   Eina_Array *c = eina_array_new(4);
   Evas_GL_Image *glimg = surface;

   eng_image_size_get(engine, glimg, &w, &h);
   eina_array_push(c, eina_rectangle_new(0, 0, w, h));

   ector_renderer_draw(renderer, EFL_GFX_RENDER_OP_BLEND, c, 0xffffffff);

   while ((r = eina_array_pop(c)))
     eina_rectangle_free(r);
   eina_array_free(c);
}

// Ector functions start
static void*
eng_ector_surface_create(void *engine, int width, int height, int *error)
{
   void *surface;

   *error = EINA_FALSE;

   /* FBO-backed render surface.  The span shader draws directly into the
    * texture attached to this image's FBO.  Use the atlas pool so multiple
    * VG objects share a single GL FBO texture, eliminating per-frame
    * glGenFramebuffers/glDeleteFramebuffers overhead when VG object sizes
    * change.  The image's tex->x/y carry the atlas sub-region offset. */
   {
      Evas_Engine_GL_Context *ctx = gl_generic_context_find(engine, EINA_TRUE);
      surface = evas_gl_common_image_surface_noscale_new(ctx, width, height, EINA_TRUE);
      if (!surface)
        *error = EINA_TRUE;
   }

   return surface;
}

static void *
eng_ector_mask_surface_create(void *engine, int width, int height, int *error)
{
   *error = EINA_FALSE;

   /* Mask FBO must use a dedicated texture — not shared with the atlas.
    * The main VG FBO and the mask FBO would otherwise map to the same GL
    * texture object, creating a read/write feedback loop when the span
    * shader samples the mask while rendering into the main FBO. */
   {
      Evas_Engine_GL_Context *ctx = gl_generic_context_find(engine, EINA_TRUE);
      void *surface = evas_gl_common_image_surface_noscale_noatlas_new(ctx, width, height, EINA_TRUE);
      if (!surface) *error = EINA_TRUE;
      return surface;
   }
}

static void
eng_ector_surface_destroy(void *engine, void *surface)
{
   if (!surface) return;
   eng_image_free(engine, surface);
}

static void
eng_ector_surface_cache_set(void *engine, void *key , void *surface)
{
   Render_Engine_GL_Generic *e = engine;

   generic_cache_data_set(e->software.surface_cache, key, surface);

}

static void *
eng_ector_surface_cache_get(void *engine, void *key)
{
   Render_Engine_GL_Generic *e = engine;

   return generic_cache_data_get(e->software.surface_cache, key);
}

static void
eng_ector_surface_cache_drop(void *engine, void *key)
{
   Render_Engine_GL_Generic *e = engine;

   generic_cache_data_drop(e->software.surface_cache, key);
}

/* ------------------------------------------------------------------ */
/* Per-shape span collector allocation callback                       */
/* ------------------------------------------------------------------ */

/**
 * Allocate or reuse a Span_Collector for one shape.
 *
 * Called from draw_rle_data() once per draw_rle_data call (i.e., once per
 * shape pass).  Lives in evas_engine.c so that span_collector_* functions
 * are only called from the module that includes evas_ector_gl_span.h.
 *
 * @param data       Ector_Software_Surface_Data* owning the arrays.
 * @param height     Canvas height (from raster_buffer->generic->h).
 * @param type       Fill type for this shape (Solid, LinearGradient, etc.).
 * @param is_stroke  EINA_TRUE if this is the stroke pass.
 * @return           Span_Collector* for this shape, or NULL on failure.
 */
static void *
_span_collector_alloc(void *data, int height,
                      Span_Data_Type type, Eina_Bool is_stroke)
{
   Ector_Software_Surface_Data *pd = (Ector_Software_Surface_Data *)data;
   void ***arr_ptr;
   int    *count_ptr, *alloc_ptr;
   int     idx;

   if (!pd) return NULL;

   if (is_stroke)
     {
        arr_ptr   = (void ***)&pd->span_collectors_stroke;
        count_ptr = &pd->span_collectors_stroke_count;
        alloc_ptr = &pd->span_collectors_stroke_alloc;
     }
   else
     {
        arr_ptr   = (void ***)&pd->span_collectors_fill;
        count_ptr = &pd->span_collectors_fill_count;
        alloc_ptr = &pd->span_collectors_fill_alloc;
     }

   idx = *count_ptr;

   /* Grow the pointer array if needed (high-water mark doubling). */
   if (idx >= *alloc_ptr)
     {
        int    new_alloc = *alloc_ptr ? *alloc_ptr * 2 : 4;
        void **new_arr   = realloc(*arr_ptr, (size_t)new_alloc * sizeof(void *));
        if (!new_arr) return NULL;
        memset(new_arr + *alloc_ptr, 0,
               (size_t)(new_alloc - *alloc_ptr) * sizeof(void *));
        *arr_ptr   = new_arr;
        *alloc_ptr = new_alloc;
     }

   /* Create collector for an empty slot; reuse+clear an existing one. */
   if (!(*arr_ptr)[idx])
     {
        (*arr_ptr)[idx] = span_collector_new(height,
                                             SPAN_COLLECTOR_DEFAULT_MAX_SPANS,
                                             type);
        if (!(*arr_ptr)[idx]) return NULL;
     }
   else
     {
        Span_Collector *sc = (Span_Collector *)(*arr_ptr)[idx];
        span_collector_resize(sc, height);
        span_collector_clear(sc);
     }

   (*count_ptr)++;
   return (*arr_ptr)[idx];
}

static Eina_Bool
eng_ector_begin(void *engine, void *surface,
                void *context EINA_UNUSED, Ector_Surface *ector,
                int x, int y, Eina_Bool do_async EINA_UNUSED)
{
   {
      Evas_GL_Image *glim = surface;
      int w, h;

        eng_image_size_get(engine, glim, &w, &h);
        if (w <= 0 || h <= 0) return EINA_FALSE;

        /* Set ector surface dimensions for rasterizer clipping.
         *
         * On first call, pixels_set initialises struct fields (stride,
         * pixel_size, cspace).  After that, just update w/h and grow
         * the pixel buffer with realloc if needed (high-water mark)
         * to avoid the per-frame free+calloc cycle in pixels_set. */
        {
           Ector_Software_Buffer_Base_Data *bbd =
              efl_data_scope_get(ector, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
           if (!bbd || !bbd->pixels.u8)
             {
                ector_buffer_pixels_set(ector, NULL, w, h, 0,
                                        EFL_GFX_COLORSPACE_ARGB8888, EINA_TRUE);
             }
           else if (bbd->generic && (bbd->generic->w != (unsigned)w || bbd->generic->h != (unsigned)h))
             {
                size_t needed = (size_t)bbd->stride * h;
                size_t have = (size_t)bbd->stride * bbd->generic->h;
                if (needed > have)
                  {
                     uint8_t *p = realloc(bbd->pixels.u8, needed);
                     if (p)
                       {
                          bbd->pixels.u8 = p;
                          memset(p + have, 0, needed - have);
                       }
                  }
                bbd->generic->w = w;
                bbd->generic->h = h;
             }
        }

        /* Per-shape collector model.
         *
         * Rather than two fixed collectors (one fill, one stroke), we now
         * allocate one collector per shape per pass via the _span_collector_alloc
         * callback.  The alloc callback is installed on Span_Data here; it is
         * called from draw_rle_data() for every shape.
         *
         * Existing collector objects are retained across frames (high-water
         * mark) and reused after a clear.  Only the count is reset to 0 here;
         * the underlying Span_Collector* slots are reused/cleared on demand.
         *
         * Skip-collection is not yet implemented for the multi-collector
         * model — it would require per-collector hash storage. */
        {
           Ector_Software_Surface_Data *pd = efl_data_scope_get(ector, ECTOR_SOFTWARE_SURFACE_CLASS);
           if (!pd) return EINA_FALSE;

           /* Reset counts — existing collectors are reused by the alloc cb. */
           pd->span_collectors_fill_count   = 0;
           pd->span_collectors_stroke_count = 0;

           if (pd->rasterizer)
             {
                Span_Data *sd = &pd->rasterizer->fill_data;
                sd->span_collector               = NULL;
                sd->span_is_stroke               = EINA_FALSE;
                sd->collector_solid              = _collect_spans_solid;
                sd->collector_gradient           = _collect_spans_gradient;
                sd->collector_composite          = _collect_spans_composite;
                /* Install per-shape alloc callback. */
                sd->span_collector_alloc         = _span_collector_alloc;
                sd->span_collector_alloc_data    = pd;
             }
        }

        ector_surface_reference_point_set(ector, x, y);
        return EINA_TRUE;
   }
}

/* ------------------------------------------------------------------ */
/* Gradient t-coefficient helpers for eng_ector_end()                 */
/* ------------------------------------------------------------------ */

/**
 * Compute linear gradient t-coefficients for the GL shader.
 *
 * The software computes per-pixel:
 *   rx = inv.xx*(x+0.5) + inv.xy*(y+0.5) + inv.xz
 *   ry = inv.yx*(x+0.5) + inv.yy*(y+0.5) + inv.yz
 *   t  = dx*rx + dy*ry + off
 *
 * The shader's px/py = gl_FragCoord.xy - fbo_offset already equal (x+0.5)
 * per the OpenGL spec (FragCoord centers at half-integer), so:
 *   t = a*px + b*py + c
 *   a = dx*inv.xx + dy*inv.yx
 *   b = dx*inv.xy + dy*inv.yy
 *   c = dx*inv.xz + dy*inv.yz + off
 */
static void
_span_gradient_linear_coeffs(Ector_Renderer_Software_Gradient_Data *gd,
                              const Eina_Matrix3 *inv,
                              int offx EINA_UNUSED, int offy EINA_UNUSED,
                              float atlas_off_x EINA_UNUSED,
                              float atlas_off_y EINA_UNUSED,
                              float *out_a, float *out_b, float *out_c)
{
   double dx  = gd->linear.dx;
   double dy  = gd->linear.dy;
   double off = gd->linear.off;

   /* The software computes t = dx*rx + dy*ry + off where:
    *   rx = inv.xx*(x+0.5) + inv.xy*(y+0.5) + inv.xz
    *   ry = inv.yx*(x+0.5) + inv.yy*(y+0.5) + inv.yz
    *
    * The shader receives px = gl_FragCoord.x - fbo_offset which already
    * equals (x + 0.5) per the OpenGL spec (FragCoord centers at half-integer).
    * So t = a*px + b*py + c decomposes as:
    *   a = dx*inv.xx + dy*inv.yx
    *   b = dx*inv.xy + dy*inv.yy
    *   c = dx*inv.xz + dy*inv.yz + off   (no extra half-pixel term needed)
    */
   *out_a = (float)(dx * inv->xx + dy * inv->yx);
   *out_b = (float)(dx * inv->xy + dy * inv->yy);
   *out_c = (float)(dx * inv->xz + dy * inv->yz + off);
}

/**
 * Precompute radial gradient coefficients for the GPU shader.
 *
 * The software path (ector_software_gradient.c:fetch_radial_gradient)
 * transforms each pixel to gradient space then solves a quadratic for t.
 * We precompute two affine rows that map FragCoord -> gradient space minus
 * focal, plus the quadratic parameters (a, dx, dy).
 *
 * With fradius = 0 (always in current EFL), sqrfr = 0 and dr*fradius = 0,
 * so the shader simplifies to:
 *   b = 2*(rx*dx + ry*dy)
 *   b_s = b * inv2a           (pre-scaled, avoids per-fragment division)
 *   det = b_s^2 + rr * 2 * inv2a
 *   t = sqrt(det) - b_s
 */
static void
_span_gradient_radial_coeffs(Ector_Renderer_Software_Gradient_Data *gd,
                              const Eina_Matrix3 *inv,
                              int offx EINA_UNUSED, int offy EINA_UNUSED,
                              float atlas_off_x EINA_UNUSED,
                              float atlas_off_y EINA_UNUSED,
                              float *out_a, float *out_b, float *out_c,
                              float *out_d, float *out_e, float *out_f,
                              float *out_ra, float *out_rdx, float *out_rdy)
{
   /* The software computes:
    *   rx = inv.xx*(x+0.5) + inv.xy*(y+0.5) + inv.xz - fx
    *   ry = inv.yx*(x+0.5) + inv.yy*(y+0.5) + inv.yz - fy
    *
    * The shader's px/py already include the +0.5 (gl_FragCoord centering),
    * so the constant terms are just inv.xz - fx and inv.yz - fy. */
   *out_a = (float)inv->xx;
   *out_b = (float)inv->xy;
   *out_c = (float)(inv->xz - gd->radial.fx);

   *out_d = (float)inv->yx;
   *out_e = (float)inv->yy;
   *out_f = (float)(inv->yz - gd->radial.fy);

   /* Quadratic parameters — pass inv2a instead of a to avoid
    * per-fragment division in the shader. */
   *out_ra  = (float)(0.5 / gd->radial.a);
   *out_rdx = (float)gd->radial.dx;
   *out_rdy = (float)gd->radial.dy;
}

/**
 * Compute per-channel gradient coefficients for the span shader.
 *
 * Inspects sc->type and sc->gradient_data.  When sc is non-NULL and holds
 * gradient data this function fills all out parameters and may downgrade
 * *inout_shader_type from LinearGradient/RadialGradient to Solid when the
 * radial geometry degenerates (fradius != 0 or a ≈ 0).  When sc is NULL
 * or has no gradient data all out values are left at their zero defaults.
 *
 * @param sc               Span collector for this channel (fill or stroke).
 * @param atlas            Gradient ramp atlas (may be NULL → gradient skipped).
 * @param inout_shader_type  On entry: LinearGradient or RadialGradient.
 *                           On exit: may be downgraded to Solid.
 * @param inout_col        Solid color — updated when downgraded to Solid.
 * @param out_ga..out_grdy Output gradient coefficients.
 * @param out_gs           Gradient spread mode (EFL enum → int).
 * @param out_gramp_y      Atlas V coordinate for this gradient's ramp row.
 * @param out_atlas_skip   Set to EINA_TRUE if atlas lookup failed (skip shape).
 * @param out_gtype        0=linear, 1=radial.
 */
static void
_compute_gradient_coeffs(Span_Collector *sc,
                          Span_Grad_Atlas *atlas,
                          int *inout_shader_type, uint32_t *inout_col,
                          float *out_ga, float *out_gb, float *out_gc,
                          int *out_gs, float *out_gramp_y, Eina_Bool *out_atlas_skip,
                          int *out_gtype,
                          float *out_gd, float *out_ge, float *out_gf,
                          float *out_gra, float *out_grdx, float *out_grdy)
{
   Ector_Renderer_Software_Gradient_Data *gd;
   int shader_type = *inout_shader_type;

   if (!sc || !sc->gradient_data) return;
   if (shader_type != (int)LinearGradient && shader_type != (int)RadialGradient) return;

   gd = (Ector_Renderer_Software_Gradient_Data *)sc->gradient_data;

   /* Upload ramp into atlas and get V coordinate.
    * version is content-derived (hash of the 4 KB staging buffer) so the
    * identity fast path in span_grad_atlas_lookup is keyed on (grad_id,
    * content_hash) — animated stop changes force a re-lookup automatically.
    * Fallback: if atlas is unavailable, mark the shape as atlas-skip. */
   if (atlas && gd->color_table && gd->ctable_status == CTABLE_READY_DONE)
     {
        uint32_t staging[1024];
        int j;
        for (j = 0; j < 1024; j++)
          {
             uint32_t c = gd->color_table[j];
             uint8_t *p = (uint8_t *)&staging[j];
             p[0] = (c >> 16) & 0xFF; /* R */
             p[1] = (c >>  8) & 0xFF; /* G */
             p[2] =  c        & 0xFF; /* B */
             p[3] = (c >> 24) & 0xFF; /* A */
          }
        uint32_t version = span_grad_atlas_hash((const uint8_t *)staging);
        int row = span_grad_atlas_lookup(atlas, sc->gradient_data,
                                         version,
                                         (const uint8_t *)staging);
        if (row < 0)
          {
             /* Atlas disabled or full — skip this shape. */
             *out_atlas_skip = EINA_TRUE;
             return;
          }
        *out_gramp_y = span_grad_atlas_row_to_v(row);
     }
   else
     {
        /* Atlas unavailable — skip gradient shapes (spec error table). */
        *out_atlas_skip = EINA_TRUE;
        return;
     }

   if (shader_type == (int)LinearGradient)
     {
        _span_gradient_linear_coeffs(gd, &sc->inv,
                                     sc->grad_offx, sc->grad_offy,
                                     0.0f, 0.0f,
                                     out_ga, out_gb, out_gc);
        *out_gtype = 0;
     }
   else /* RadialGradient */
     {
        if (gd->radial.fradius >= 0.00001f || fabsf(gd->radial.a) <= 0.00001f)
          {
             /* Degenerate radial — fall back to solid using first stop color. */
             *inout_shader_type = (int)Solid;
             if (gd->color_table)
               *inout_col = gd->color_table[0];
             return;
          }
        _span_gradient_radial_coeffs(gd, &sc->inv,
                                     sc->grad_offx, sc->grad_offy,
                                     0.0f, 0.0f,
                                     out_ga, out_gb, out_gc,
                                     out_gd, out_ge, out_gf,
                                     out_gra, out_grdx, out_grdy);
        *out_gtype = 1;
     }

   *out_gs = (int)gd->gd->s;
}

static void
eng_ector_end(void *engine,
              void *surface,
              void *context EINA_UNUSED,
              Ector_Surface *ector,
              Eina_Bool do_async EINA_UNUSED)
{
   {
      Ector_Software_Surface_Data *espd = efl_data_scope_get(ector, ECTOR_SOFTWARE_SURFACE_CLASS);
      Evas_GL_Image *glim = surface;

        /* _span_collector_alloc updates espd-> arrays directly (it receives
         * espd as its data pointer).  No sync-back from Span_Data needed —
         * the counts and pointers on espd are already authoritative. */

        {
           int fill_count   = espd ? espd->span_collectors_fill_count   : 0;
           int stroke_count = espd ? espd->span_collectors_stroke_count : 0;
           void **fill_arr   = (espd && fill_count)   ? espd->span_collectors_fill   : NULL;
           void **stroke_arr = (espd && stroke_count) ? espd->span_collectors_stroke : NULL;

           if (glim && (fill_count > 0 || stroke_count > 0))
             {
                int w, h;
                Evas_Engine_GL_Context *gc;
                int ci;

                eng_image_size_get(engine, glim, &w, &h);
                gc = gl_generic_context_find(engine, EINA_TRUE);

                span_shader_init();

                /* Upload textures for all collectors. */
                for (ci = 0; ci < fill_count; ci++)
                  span_collector_upload_textures((Span_Collector *)fill_arr[ci], gc);
                for (ci = 0; ci < stroke_count; ci++)
                  span_collector_upload_textures((Span_Collector *)stroke_arr[ci], gc);

                /* Check that at least one collector has texture data. */
                {
                   int has_data = 0;
                   for (ci = 0; !has_data && ci < fill_count; ci++)
                     has_data |= ((Span_Collector *)fill_arr[ci])->actual_max_spans > 0;
                   for (ci = 0; !has_data && ci < stroke_count; ci++)
                     has_data |= ((Span_Collector *)stroke_arr[ci])->actual_max_spans > 0;
                   if (!has_data) goto span_done;
                }

                evas_gl_common_context_target_surface_set(gc, glim);

                /* Bump gradient atlas LRU frame counter for this render pass. */
                span_grad_atlas_frame_begin(((Render_Engine_GL_Generic *)engine)->grad_atlas);

                /* Atlas offset: when VG surfaces share an FBO via the texture
                 * atlas pool, each surface occupies a sub-rectangle at (ox, oy).
                 * Dedicated FBOs have ox=oy=0 (no offset). */
                int ox = glim->tex ? glim->tex->x : 0;
                int oy = glim->tex ? glim->tex->y : 0;

                /* Clear FBO sub-region to transparent via glClear + scissor.
                 *
                 * On tile-based GPUs (Broadcom V3D / ARM Mali) glClear is
                 * a tile-buffer flag — no main-memory traffic — whereas the
                 * previous SHD_RECT + immediate flush forced a tile store
                 * and reload before the span draws.  The clear happens
                 * outside the pipe system, so no pipe-reorder races; we
                 * still flush any pending pipe content first to be safe.
                 *
                 * glScissor uses the same (ox, oy, w, h) as glViewport:
                 * _evas_gl_common_viewport_set already sets the viewport to
                 * (tex->x, tex->y, w, h) for atlas sub-rects, so scissor
                 * coords are in framebuffer space, matching GL bottom-left.
                 *
                 * The Evas GL state cache is invalidated on textures and
                 * render op below so subsequent draws restore them as needed. */
                evas_gl_common_context_flush(gc);
                glEnable(GL_SCISSOR_TEST);
                glScissor(ox, oy, w, h);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_SCISSOR_TEST);
                gc->state.current.clip      = 0;
                gc->state.current.cx        = 0;
                gc->state.current.cy        = 0;
                gc->state.current.cw        = 0;
                gc->state.current.ch        = 0;
                gc->state.current.cur_tex   = -1;
                gc->state.current.render_op = -1;

                /* Per-shape draw loop.
                 *
                 * Each iteration draws ONE fill collector + ONE stroke
                 * collector (either may be NULL/absent for that shape).
                 * Collectors are indexed by shape: fill_arr[shape] and
                 * stroke_arr[shape].  One shape may have no fill (e.g.
                 * stroke-only) or no stroke (fill-only).
                 *
                 * Within each shape's collector we still loop over split
                 * textures (spatial x-range splits from overflow spans).
                 *
                 * The draw loop is:
                 *   outer: shape index (0 .. max(fill_count, stroke_count)-1)
                 *   inner: texture split index within a collector
                 */
                Span_Data *_rsd = (espd && espd->rasterizer)
                                  ? &espd->rasterizer->fill_data : NULL;
                int max_shapes = fill_count > stroke_count
                                 ? fill_count : stroke_count;
                 int si;
                 for (si = 0; si < max_shapes; si++)
                   {
                      Span_Collector *sc_fill   = (si < fill_count)
                                                  ? (Span_Collector *)fill_arr[si]
                                                  : NULL;
                       Span_Collector *sc_stroke = (si < stroke_count)
                                                   ? (Span_Collector *)stroke_arr[si]
                                                   : NULL;

                      int fill_tc   = sc_fill   ? sc_fill->texture_count   : 0;
                     int stroke_tc = sc_stroke ? sc_stroke->texture_count : 0;
                     int max_tc    = fill_tc > stroke_tc ? fill_tc : stroke_tc;
                     if (max_tc == 0) continue;

                     /* Per-shape actual_max_spans (used to cap the shader loop). */
                     int actual_max = 1;
                     if (sc_fill   && sc_fill->actual_max_spans   > actual_max)
                       actual_max = sc_fill->actual_max_spans;
                     if (sc_stroke && sc_stroke->actual_max_spans > actual_max)
                       actual_max = sc_stroke->actual_max_spans;

                     uint32_t fill_col   = sc_fill   ? sc_fill->color   : 0;
                     uint32_t stroke_col = sc_stroke ? sc_stroke->color : 0;

                     int fill_shader_type   = sc_fill   ? (int)sc_fill->type   : (int)Solid;
                     int stroke_shader_type = sc_stroke ? (int)sc_stroke->type : (int)Solid;

                     /* Per-shape gradient coefficients. */
                     float fill_ga = 0.0f, fill_gb = 0.0f, fill_gc_coef = 0.0f;
                     int   fill_gs = 0;
                     float fill_gramp_y = 0.0f;
                     Eina_Bool fill_atlas_skip = EINA_FALSE;
                     int   fill_gtype = 0;
                     float fill_gd = 0.0f, fill_ge = 0.0f, fill_gf = 0.0f;
                     float fill_gra = 0.0f, fill_grdx = 0.0f, fill_grdy = 0.0f;

                     float stroke_ga = 0.0f, stroke_gb = 0.0f, stroke_gc_coef = 0.0f;
                     int   stroke_gs = 0;
                     float stroke_gramp_y = 0.0f;
                     Eina_Bool stroke_atlas_skip = EINA_FALSE;
                     int   stroke_gtype = 0;
                     float stroke_gd = 0.0f, stroke_ge = 0.0f, stroke_gf = 0.0f;
                     float stroke_gra = 0.0f, stroke_grdx = 0.0f, stroke_grdy = 0.0f;

                     {
                        Render_Engine_GL_Generic *re =
                           (Render_Engine_GL_Generic *)engine;
                        Span_Grad_Atlas *atlas = re->grad_atlas;

                        if (_rsd)
                          {
                             _compute_gradient_coeffs(sc_fill, atlas,
                                                      &fill_shader_type, &fill_col,
                                                      &fill_ga, &fill_gb, &fill_gc_coef,
                                                      &fill_gs, &fill_gramp_y,
                                                      &fill_atlas_skip, &fill_gtype,
                                                      &fill_gd, &fill_ge, &fill_gf,
                                                      &fill_gra, &fill_grdx, &fill_grdy);
                             _compute_gradient_coeffs(sc_stroke, atlas,
                                                      &stroke_shader_type, &stroke_col,
                                                      &stroke_ga, &stroke_gb, &stroke_gc_coef,
                                                      &stroke_gs, &stroke_gramp_y,
                                                      &stroke_atlas_skip, &stroke_gtype,
                                                      &stroke_gd, &stroke_ge, &stroke_gf,
                                                      &stroke_gra, &stroke_grdx, &stroke_grdy);
                          }

                        /* Skip gradient shapes when atlas lookup failed. */
                        if (fill_atlas_skip || stroke_atlas_skip) continue;
                     }

                     /* Draw each spatial-split texture within this shape.
                      * Most collectors have 1 texture; complex shapes that
                      * produce >max_spans spans per row may have more. */
                     {
                        int ti;
                        for (ti = 0; ti < max_tc; ti++)
                          {
                             GLuint f_tex = 0, s_tex = 0;
                             int f_tx = 0, f_ty = 0, s_tx = 0, s_ty = 0;
                             int pw = 1, ph = 1;
                             int f_xmin = 0, s_xmin = 0;

                             if (ti < fill_tc && sc_fill->textures[ti].evas_tex)
                               {
                                  Evas_GL_Texture *t = (Evas_GL_Texture *)sc_fill->textures[ti].evas_tex;
                                  f_tex  = t->pt->texture;
                                  f_tx   = t->x; f_ty = t->y;
                                  pw     = t->pt->w; ph = t->pt->h;
                                  f_xmin = sc_fill->textures[ti].x_min;
                               }
                             if (ti < stroke_tc && sc_stroke->textures[ti].evas_tex)
                               {
                                  Evas_GL_Texture *t = (Evas_GL_Texture *)sc_stroke->textures[ti].evas_tex;
                                  s_tex  = t->pt->texture;
                                  s_tx   = t->x; s_ty = t->y;
                                  pw     = t->pt->w; ph = t->pt->h;
                                  s_xmin = sc_stroke->textures[ti].x_min;
                               }

                             if (!f_tex && !s_tex) continue;

                             {
                                Span_Pipe_Params _spp = { 0 };
                                _spp.pool_w     = pw;
                                _spp.pool_h     = ph;
                                _spp.max_spans  = actual_max;
                                _spp.x          = 0;
                                _spp.y          = 0;
                                _spp.w          = w;
                                _spp.h          = h;
                                _spp.mul_col    = 0xFFFFFFFF;
                                _spp.fbo_off_x  = (float)ox;
                                _spp.fbo_off_y  = (float)oy;

                                {
                                   Render_Engine_GL_Generic *re =
                                      (Render_Engine_GL_Generic *)engine;
                                   _spp.grad_atlas_tex =
                                      (re->grad_atlas && re->grad_atlas->tex)
                                      ? re->grad_atlas->tex : 0;
                                }

                                _spp.fill.tex        = f_tex;
                                _spp.fill.off_tx     = (float)f_tx;
                                _spp.fill.off_ty     = (float)f_ty;
                                _spp.fill.col        = fill_col;
                                _spp.fill.type       = fill_shader_type;
                                _spp.fill.x_min      = f_xmin;
                                _spp.fill.grad_a     = fill_ga;
                                _spp.fill.grad_b     = fill_gb;
                                _spp.fill.grad_c     = fill_gc_coef;
                                _spp.fill.grad_spread  = fill_gs;
                                _spp.fill.grad_ramp_y  = fill_gramp_y;
                                _spp.fill.grad_type    = fill_gtype;
                                _spp.fill.grad_d     = fill_gd;
                                _spp.fill.grad_e     = fill_ge;
                                _spp.fill.grad_f     = fill_gf;
                                _spp.fill.grad_ra    = fill_gra;
                                _spp.fill.grad_rdx   = fill_grdx;
                                _spp.fill.grad_rdy   = fill_grdy;

                                _spp.stroke.tex        = s_tex;
                                _spp.stroke.off_tx     = (float)s_tx;
                                _spp.stroke.off_ty     = (float)s_ty;
                                _spp.stroke.col        = stroke_col;
                                _spp.stroke.type       = stroke_shader_type;
                                _spp.stroke.x_min      = s_xmin;
                                _spp.stroke.grad_a     = stroke_ga;
                                _spp.stroke.grad_b     = stroke_gb;
                                _spp.stroke.grad_c     = stroke_gc_coef;
                                _spp.stroke.grad_spread  = stroke_gs;
                                _spp.stroke.grad_ramp_y  = stroke_gramp_y;
                                _spp.stroke.grad_type    = stroke_gtype;
                                _spp.stroke.grad_d     = stroke_gd;
                                _spp.stroke.grad_e     = stroke_ge;
                                _spp.stroke.grad_f     = stroke_gf;
                                _spp.stroke.grad_ra    = stroke_gra;
                                _spp.stroke.grad_rdx   = stroke_grdx;
                                _spp.stroke.grad_rdy   = stroke_grdy;

                                /* GL composite mask: if a mask FBO was rendered
                                 * during render_pre for this VG container, its
                                 * Evas_GL_Image* was stored on espd->gl_comp_surface
                                 * by ector_software_surface_gl_comp_set().
                                 * Extract the GL texture name and atlas offsets
                                 * so the fragment shader can sample the mask. */
                                if (espd->gl_comp_surface)
                                  {
                                     Evas_GL_Image *mask_im =
                                        (Evas_GL_Image *)espd->gl_comp_surface;
                                     if (mask_im->tex && mask_im->tex->pt)
                                       {
                                          _spp.mask_tex     = mask_im->tex->pt->texture;
                                          _spp.comp_method  = espd->gl_comp_method;
                                          _spp.mask_w       = (float)mask_im->tex->pt->w;
                                          _spp.mask_h       = (float)mask_im->tex->pt->h;
                                          _spp.mask_off_x   = (float)mask_im->tex->x;
                                          _spp.mask_off_y   = (float)mask_im->tex->y;
                                       }
                                  }

                                evas_gl_common_context_span_push(gc, &_spp);
                             }
                          }
                     }
                  } /* per-shape loop */

                evas_gl_common_context_target_surface_set(gc, gc->def_surface);
             }
        }
span_done:

        /* Do NOT reset ector surface dimensions to 0 here — the pixel
         * buffer is managed by the high-water mark path in eng_ector_begin
         * and must persist across begin/end cycles within the same frame
         * (VG objects may be rendered multiple times per frame). */

        /* Clear the GL composite mask reference after use.  It is re-set
         * every frame by _evas_vg_render (the draw phase, which always runs)
         * rather than render_pre (which only runs on changes). */
        if (espd)
          {
             espd->gl_comp_surface = NULL;
             espd->gl_comp_method  = 0;
          }

        if (espd && espd->rasterizer)
          {
             Span_Data *sd = &espd->rasterizer->fill_data;
             sd->span_collector           = NULL;
             sd->span_collector_alloc     = NULL;
             sd->span_collector_alloc_data = NULL;
          }
   }
}

static Eina_Bool
eng_image_data_map(void *engine, void **image, Eina_Rw_Slice *slice,
                   int *stride, int x, int y, int w, int h,
                   Evas_Colorspace cspace, Efl_Gfx_Buffer_Access_Mode mode,
                   int plane)
{
   Render_Engine_GL_Generic *re = engine;
   Evas_GL_Image_Data_Map *map = NULL;
   Evas_GL_Image *glim, *glim2 = NULL;
   Eina_Bool ok = EINA_FALSE;
   RGBA_Image *im = NULL;
   int strid;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(image && *image && slice, EINA_FALSE);

   glim = *image;
   slice->mem = NULL;
   slice->len = 0;

   if (glim->im && (glim->orient == EVAS_IMAGE_ORIENT_NONE))
     {
        evas_gl_common_image_ref(glim);
        glim2 = glim;
     }
   else
     {
        glim2 = _rotate_image_data(re, glim);
     }

   if (!glim2) return EINA_FALSE;
   im = glim2->im;
   if (im)
     {
        // Call sw generic implementation.
        ok = pfunc.image_data_map(NULL, (void **) &im, slice, &strid,
                                  x, y, w, h, cspace, mode, plane);
     }

   if (!ok)
     {
        eng_image_free(re, glim2);
        return EINA_FALSE;
     }

   evas_cache_image_ref(&im->cache_entry);

   map = calloc(1, sizeof(*map));
   map->cspace = cspace;
   map->rx = x;
   map->ry = y;
   map->rw = w;
   map->rh = h;
   map->mode = mode;
   map->slice = *slice;
   map->stride = strid;
   map->im = im;
   map->glim = glim2;
   glim->maps = eina_inlist_prepend(glim->maps, EINA_INLIST_GET(map));
   if (stride) *stride = strid;

   if (mode & EFL_GFX_BUFFER_ACCESS_MODE_WRITE)
     {
        evas_gl_common_image_ref(glim2);
        evas_gl_common_image_free(glim);
        *image = glim2;
     }

   return EINA_TRUE;
}

static Eina_Bool
eng_image_data_unmap(void *engine EINA_UNUSED, void *image, const Eina_Rw_Slice *slice)
{
   Evas_GL_Image_Data_Map *map;
   Evas_GL_Image *im = image;
   Eina_Bool found = EINA_FALSE;

   if (!(image && slice))
     return EINA_FALSE;

   EINA_INLIST_FOREACH(im->maps, map)
     {
        if ((map->slice.len == slice->len) && (map->slice.mem == slice->mem))
          {
             found = EINA_TRUE;
             if (map->im)
               {
                  found = pfunc.image_data_unmap(NULL, map->im, slice);
                  evas_cache_image_drop(&map->im->cache_entry);
               }
             if (found)
               {
                  if (im->im && im->tex &&
                      (map->mode & EFL_GFX_BUFFER_ACCESS_MODE_WRITE))
                    evas_gl_common_texture_update(im->tex, im->im);
                  im->maps = eina_inlist_remove(im->maps, EINA_INLIST_GET(map));
                  evas_gl_common_image_free(map->glim);
                  free(map);
               }
             return found;
          }
     }

   ERR("failed to unmap region %p (%zu bytes)", slice->mem, slice->len);
   return EINA_FALSE;
}

static int
eng_image_data_maps_get(void *engine EINA_UNUSED, const void *image, const Eina_Rw_Slice **slices)
{
   Evas_GL_Image_Data_Map *map;
   const Evas_GL_Image *im = image;
   int k = 0;

   if (!im) return -1;

   if (!slices)
     return eina_inlist_count(im->maps);

   EINA_INLIST_FOREACH(im->maps, map)
     slices[k++] = &map->slice;

   return k;
}

static inline Eina_Bool
_is_yuv(Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        return EINA_TRUE;

      default:
        return EINA_FALSE;
     }
}

static void *
eng_image_data_slice_add(void *engine, void *image,
                         const Eina_Slice *slice, Eina_Bool copy,
                         int w, int h, int stride, Evas_Colorspace cspace,
                         int plane, Eina_Bool alpha)
{
   const Eina_Bool use_cs = _is_yuv(cspace);
   const unsigned char **cs_data;
   Evas_GL_Image *im = image;
   int bpp = 0;

   // Note: This code is not very robust by choice. It should NOT be used
   // in conjunction with data_put/data_get. Ever.
   // Assume w,h,cspace,alpha to be correct.
   // We still use cs.data for YUV.
   // 'image' may be NULL, in that case create a new one. Otherwise, it must
   // have been created by a previous call to this function.

   if ((plane < 0) || (plane >= RGBA_PLANE_MAX)) goto fail;
   if (!slice || !slice->mem) goto fail;
   copy = !!copy;

   // not implemented
   if (use_cs && copy)
     {
        // To implement this, we should switch the internals to slices first,
        // as this would give 3 planes rather than N rows of datas
        ERR("Evas can not copy YUV data (not implemented yet).");
        goto fail;
     }

   if (im && !im->im)
     {
        evas_gl_common_image_unref(im);
        im = NULL;
     }

   // alloc
   if (!im)
     {
        switch (cspace)
          {
           case EFL_GFX_COLORSPACE_ARGB8888:
           case EFL_GFX_COLORSPACE_AGRY88:
           case EFL_GFX_COLORSPACE_GRY8:
             if (plane != 0) goto fail;
             if (copy)
               im = eng_image_new_from_copied_data(engine, w, h, NULL, alpha, cspace);
             else
               im = eng_image_new_from_data(engine, w, h, NULL, alpha, cspace);
             break;

           case EFL_GFX_COLORSPACE_YCBCR422P601_PL:
           case EFL_GFX_COLORSPACE_YCBCR422P709_PL:
           case EFL_GFX_COLORSPACE_YCBCR422601_PL:
           case EFL_GFX_COLORSPACE_YCBCR420NV12601_PL:
             im = eng_image_new_from_data(engine, w, h, NULL, alpha, cspace);
             break;

           default:
             // TODO: ETC, S3TC, YCBCR420TM12 (aka ST12 or tiled NV12)
             goto fail;
          }
        if (!im) goto fail;
     }

   if (use_cs && (!im->cs.data || im->cs.no_free))
     {
        im->cs.data = calloc(1, h * sizeof(void *) * 2);
        if (!im->cs.data) goto fail;
        im->cs.no_free = EINA_FALSE;
        if (!im->im->cs.no_free) free(im->im->cs.data);
        im->im->cs.data = im->cs.data;
        im->im->cs.no_free = EINA_TRUE;
     }

   // is this allocating image.data or cs.data?
   evas_gl_common_image_alloc_ensure(im);
   if (!im->im)
     goto fail;

   // assign
   switch (cspace)
     {
      case EFL_GFX_COLORSPACE_ARGB8888:
        bpp = 4;
        EINA_FALLTHROUGH;
        // falltrhough is intended
      case EFL_GFX_COLORSPACE_AGRY88:
        if (!bpp) bpp = 2;
        EINA_FALLTHROUGH;
        // falltrhough is intended
      case EFL_GFX_COLORSPACE_GRY8:
        if (!bpp) bpp = 1;
        if (plane != 0) goto fail;
        if (!im->im->image.data) goto fail;
        if (!stride) stride = w * bpp;
        if (copy)
          {
             for (int y = 0; y < h; y++)
               {
                  const unsigned char *src = slice->bytes + h * stride;
                  unsigned char *dst = im->im->image.data8 + bpp * w;
                  memcpy(dst, src, w * bpp);
               }
          }
        else
          {
             if (stride != (bpp * w))
               {
                  ERR("invalid stride for zero-copy data set");
                  goto fail;
               }
             im->im->image.data = (DATA32 *) slice->mem;
             im->im->image.no_free = EINA_TRUE;
          }
        break;

      case EFL_GFX_COLORSPACE_YCBCR422P601_PL:
        EINA_FALLTHROUGH;
      case EFL_GFX_COLORSPACE_YCBCR422P709_PL:
        /* YCbCr 4:2:2 Planar: Y rows, then the Cb, then Cr rows. */
        cs_data = im->cs.data;
        if (plane == 0)
          {
             if (!stride) stride = w;
             for (int y = 0; y < h; y++)
               cs_data[y] = slice->bytes + (y * stride);
          }
        else if (plane == 1)
          {
             if (!stride) stride = w / 2;
             for (int y = 0; y < (h / 2); y++)
               cs_data[h + y] = slice->bytes + (y * stride);
          }
        else if (plane == 2)
          {
             if (!stride) stride = w / 2;
             for (int y = 0; y < (h / 2); y++)
               cs_data[h + (h / 2) + y] = slice->bytes + (y * stride);
          }
        break;

      case EFL_GFX_COLORSPACE_YCBCR422601_PL:
        /* YCbCr 4:2:2: lines of Y,Cb,Y,Cr bytes. */
        if (plane != 0) goto fail;
        if (!stride) stride = w * 2;
        cs_data = im->cs.data;
        for (int y = 0; y < h; y++)
          cs_data[y] = slice->bytes + (y * stride);
        break;

      case EFL_GFX_COLORSPACE_YCBCR420NV12601_PL:
        /* YCbCr 4:2:0: Y rows, then the Cb,Cr rows. */
        if (!stride) stride = w;
        cs_data = im->cs.data;
        if (plane == 0)
          {
             for (int y = 0; y < h; y++)
               cs_data[y] = slice->bytes + (y * stride);
          }
        else if (plane == 1)
          {
             for (int y = 0; y < (h / 2); y++)
               cs_data[h + y] = slice->bytes + (y * stride);
          }
        break;

        // ETC, S3TC, YCBCR420TM12 (aka ST12 or tiled NV12)
      default:
        ERR("unsupported color space %d", cspace);
        goto fail;
     }

   evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return im;

fail:
   if (im) eng_image_free(engine, im);
   return NULL;
}

static void
eng_image_prepare(void *engine EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;

   if (!im) return;
   evas_gl_common_image_update(im->gc, im);
}

static void *
eng_image_surface_noscale_new(void *engine, int w, int h, int alpha)
{
   Evas_Engine_GL_Context *gl_context;

   gl_context = gl_generic_context_find(engine, 1);
   return evas_gl_common_image_surface_noscale_new(gl_context, w, h, alpha);
}

//------------------------------------------------//

static GL_Filter_Apply_Func
_gfx_filter_func_get(Render_Engine_GL_Generic *re, Evas_Filter_Command *cmd)
{
   GL_Filter_Apply_Func funcptr = NULL;

   switch (cmd->mode)
     {
      case EVAS_FILTER_MODE_BLEND: funcptr = gl_filter_blend_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_BLUR: funcptr = gl_filter_blur_func_get(re, cmd); break;
      //case EVAS_FILTER_MODE_BUMP: funcptr = gl_filter_bump_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_CURVE: funcptr = gl_filter_curve_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_DISPLACE: funcptr = gl_filter_displace_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_FILL: funcptr = gl_filter_fill_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_MASK: funcptr = gl_filter_mask_func_get(re, cmd); break;
      //case EVAS_FILTER_MODE_TRANSFORM: funcptr = gl_filter_transform_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_GRAYSCALE: funcptr = gl_filter_grayscale_func_get(re, cmd); break;
      case EVAS_FILTER_MODE_INVERSE_COLOR: funcptr = gl_filter_inverse_color_func_get(re, cmd); break;
      default: return NULL;
     }

   return funcptr;
}

static Evas_Filter_Support
eng_gfx_filter_supports(void *engine, Evas_Filter_Command *cmd)
{
   Render_Engine_GL_Generic *re = engine;

   if (!_gfx_filter_func_get(re, cmd))
     return pfunc.gfx_filter_supports(&re->software, cmd);

   return EVAS_FILTER_SUPPORT_GL;
}

static Eina_Bool
eng_gfx_filter_process(void *engine, Evas_Filter_Command *cmd)
{
   Render_Engine_GL_Generic *re = engine;
   GL_Filter_Apply_Func funcptr;

   funcptr = _gfx_filter_func_get(re, cmd);
   if (funcptr)
     return funcptr(re, cmd);
   else
     return pfunc.gfx_filter_process(&re->software, cmd);
}

static void
eng_font_glyphs_gc_collect(void *data EINA_UNUSED, float ratio EINA_UNUSED, int *texture_size EINA_UNUSED, int *atlas_size EINA_UNUSED, Eina_Bool only_when_requested EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   gl_context = gl_generic_context_find(data, 1);

   if ((only_when_requested && gl_context->font_glyph_gc_requested) || (!only_when_requested))
     {
        if (ratio > 0.0)
          {
             int requested_size = ratio * gl_context->font_glyph_textures_size;

             Eina_List *l;
             Eina_List *ll;
             Evas_GL_Image *im;
             EINA_LIST_FOREACH_SAFE(gl_context->font_glyph_images, l, ll, im)
               {
                  int size = im->w * im->h * 4;
                  gl_context->font_glyph_textures_size -= size;
                  requested_size -= size;
                  if ((im->tex != NULL) && (im->tex->pt) && (im->tex->pt->references == 1))
                    {
                       gl_context->font_glyph_atlas_size -= im->tex->pt->w * im->tex->pt->h * 4;
                    }
                  evas_gl_common_image_free(im);
                  if (requested_size <= 0)
                    break;
               }
          }

         gl_context->font_glyph_gc_requested = EINA_FALSE;
     }
   if (texture_size) *texture_size = gl_context->font_glyph_textures_size;
   if (atlas_size) *atlas_size = gl_context->font_glyph_atlas_size;
}

//------------------------------------------------//

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic", 0)) return 0;
   if (_evas_engine_GL_log_dom < 0)
     _evas_engine_GL_log_dom = eina_log_domain_register("evas-gl_generic", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   ector_init();
   ector_glsym_set(dlsym, RTLD_DEFAULT);

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(engine_new);
   ORD(engine_free);

   ORD(context_new);
   ORD(context_clip_image_set);
   ORD(context_clip_image_unset);
   ORD(context_clip_image_get);
   ORD(context_dup);
   ORD(context_free);

   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);

   ORD(image_load);
   ORD(image_mmap);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_ref);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_direct_get);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_orient_set);
   ORD(image_orient_get);
   ORD(image_draw);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_file_colorspace_get);
   ORD(image_can_region_get);
   ORD(image_stretch_region_get);
   ORD(image_native_init);
   ORD(image_native_shutdown);
   ORD(image_native_set);
   ORD(image_native_get);

   ORD(font_draw);

   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   ORD(image_stride_get);

   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_clean);
   ORD(image_scaled_update);

   ORD(image_content_hint_set);

   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);

   ORD(image_data_map);
   ORD(image_data_unmap);
   ORD(image_data_maps_get);
   ORD(image_data_slice_add);

   ORD(image_prepare);
   ORD(image_surface_noscale_new);

   ORD(font_cache_flush);
   ORD(font_cache_set);
   ORD(font_cache_get);

   ORD(gl_supports_evas_gl);
   ORD(gl_output_set);
   ORD(gl_surface_create);
   ORD(gl_pbuffer_surface_create);
   ORD(gl_surface_destroy);
   ORD(gl_context_create);
   ORD(gl_context_destroy);
   ORD(gl_make_current);
   ORD(gl_string_query);
   ORD(gl_proc_address_get);
   ORD(gl_native_surface_get);
   ORD(gl_api_get);
   ORD(gl_direct_override_get);
   ORD(gl_surface_direct_renderable_get);
   ORD(gl_get_pixels_set);
   ORD(gl_get_pixels_pre);
   ORD(gl_get_pixels_post);
   ORD(gl_surface_lock);
   ORD(gl_surface_read_pixels);
   ORD(gl_surface_unlock);
   //ORD(gl_error_get);
   ORD(gl_surface_query);
   // gl_current_context_get is in engine
   ORD(gl_current_surface_get);
   ORD(gl_rotation_angle_get);
   ORD(gl_image_direct_get);
   ORD(gl_image_direct_set);

   ORD(image_load_error_get);

   /* now advertise out own api */
   ORD(image_animated_get);
   ORD(image_animated_frame_count_get);
   ORD(image_animated_loop_type_get);
   ORD(image_animated_loop_count_get);
   ORD(image_animated_frame_duration_get);
   ORD(image_animated_frame_set);
   ORD(image_animated_frame_get);

   ORD(image_max_size_get);

   ORD(pixel_alpha_get);

   ORD(context_flush);

   ORD(ector_create);
   ORD(ector_destroy);
   ORD(ector_buffer_wrap);
   ORD(ector_buffer_new);
   ORD(ector_begin);
   ORD(ector_renderer_draw);
   ORD(ector_end);
   ORD(ector_surface_create);
   ORD(ector_mask_surface_create);
   ORD(ector_surface_destroy);
   ORD(ector_surface_cache_set);
   ORD(ector_surface_cache_get);
   ORD(ector_surface_cache_drop);
   ORD(gfx_filter_supports);
   ORD(gfx_filter_process);
   ORD(font_glyphs_gc_collect);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   ector_shutdown();
   if (_evas_engine_GL_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_GL_log_dom);
        _evas_engine_GL_log_dom = -1;
     }
   /* Destroy the static span-lookup GLSL programs while the GL context
    * is still alive (evas_gl_common_module_close destroys it).
    * Safe to call even if span_shader_init() was never called. */
   span_shader_shutdown();
   evas_gl_common_module_close();
}

static Evas_Module_Api evas_modapi =
  {
    EVAS_MODULE_API_VERSION,
    "gl_generic",
    "none",
    {
      module_open,
      module_close
    }
  };

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_generic);

#ifndef EVAS_STATIC_BUILD_GL_COMMON
EVAS_EINA_MODULE_DEFINE(engine, gl_generic);
#endif
