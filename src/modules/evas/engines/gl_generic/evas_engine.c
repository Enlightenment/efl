#include "evas_common_private.h"
#include "evas_gl_core_private.h"

//#define TIMDBG 1
#ifdef TIMDBG
# include <sys/time.h>
# include <unistd.h>
#endif

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_x11 should not get compiled if dlsym is not found on the system!
#endif

#include "../gl_common/evas_gl_common.h"

#include "Evas_Engine_GL_Generic.h"

#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

static int _evas_engine_GL_log_dom = -1;

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_rect_draw(gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int p1x, int p1y, int p2x, int p2y, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_line_draw(gl_context, p1x, p1y, p2x, p2y);
}

static void *
eng_polygon_point_add(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon, int x, int y)
{
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon)
{
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface EINA_UNUSED, void *polygon, int x, int y, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   evas_gl_common_poly_draw(gl_context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!image) return 1;
   im = image;
   return im->alpha;
}

static Evas_Colorspace
eng_image_colorspace_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->cs.space;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im;

   if (!image) return NULL;
   im = image;
   if (im->alpha == has_alpha) return image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   re->window_use(re->software.ob);
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
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->im->cache_entry))
               evas_cache2_image_load_data(&im->im->cache_entry);
             else
#endif
               evas_cache_image_load_data(&im->im->cache_entry);
          }
        evas_gl_common_image_alloc_ensure(im);
        im_new = evas_gl_common_image_new_from_copied_data
           (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
               im->im->image.data,
               eng_image_alpha_get(data, image),
               eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return evas_gl_common_image_alpha_set(im, has_alpha ? 1 : 0);
}

static void *
eng_image_border_set(void *data EINA_UNUSED, void *image, int l EINA_UNUSED, int r EINA_UNUSED, int t EINA_UNUSED, int b EINA_UNUSED)
{
   return image;
}

static void
eng_image_border_get(void *data EINA_UNUSED, void *image EINA_UNUSED, int *l EINA_UNUSED, int *r EINA_UNUSED, int *t EINA_UNUSED, int *b EINA_UNUSED)
{
}


static char *
eng_image_comment_get(void *data EINA_UNUSED, void *image, char *key EINA_UNUSED)
{
   Evas_GL_Image *im;

   if (!image) return NULL;
   im = image;
   if (!im->im) return NULL;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, Evas_Colorspace cspace)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im;

   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   re->window_use(re->software.ob);
   evas_gl_common_image_alloc_ensure(im);
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
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
         abort();
         break;
     }
   im->cs.space = cspace;
}

static void
_native_bind_cb(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n = im->native.data;

   if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, n->data.opengl.texture_id);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
   return;
}

static void
_native_unbind_cb(void *data EINA_UNUSED, void *image)
{
  Evas_GL_Image *im = image;
  Evas_Native_Surface *n = im->native.data;

  if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, 0);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
  return;
}

static void
_native_free_cb(void *data, void *image)
{
  Evas_Engine_GL_Context *gl_context;
  Render_Engine_GL_Generic *re = data;
  Evas_GL_Image *im = image;
  Evas_Native_Surface *n = im->native.data;
  uint32_t texid;

  if (n->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       gl_context = re->window_gl_context_get(re->software.ob);

       texid = n->data.opengl.texture_id;
       eina_hash_del(gl_context->shared->native_tex_hash, &texid, im);
    }
  im->native.data        = NULL;
  im->native.func.data   = NULL;
  im->native.func.bind   = NULL;
  im->native.func.unbind = NULL;
  im->native.func.free   = NULL;
  free(n);
}

static void *
eng_image_native_set(void *data, void *image, void *native)
{
  Evas_Engine_GL_Context *gl_context;
  Render_Engine_GL_Generic *re = data;
  Evas_Native_Surface *ns = native;
  Evas_GL_Image *im = image, *im2 = NULL;
  uint32_t texid;
  Evas_Native_Surface *n;
  unsigned int tex = 0;
  unsigned int fbo = 0;

  gl_context = re->window_gl_context_get(re->software.ob);

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
  if ((!ns) && (!im->native.data)) return im;

  re->window_use(re->software.ob);

  if (im->native.data)
    {
      if (im->native.func.free)
        im->native.func.free(im->native.func.data, im);
      evas_gl_common_image_native_disable(im);
    }

  if (!ns) return im;

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
              im->native.data        = n;
              im->native.func.data   = re;
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
eng_image_native_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n;

   if (!im) return NULL;
   n = im->native.data;
   if (!n) return NULL;
   return n;
}

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   *error = EVAS_LOAD_ERROR_NONE;
   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   return evas_gl_common_image_load(gl_context, file, key, lo, error);
}

static void *
eng_image_mmap(void *data, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   *error = EVAS_LOAD_ERROR_NONE;
   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   return evas_gl_common_image_mmap(gl_context, f, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   return evas_gl_common_image_new_from_data(gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   return evas_gl_common_image_new_from_copied_data(gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine_GL_Generic *re = data;

   if (!image) return;
   re->window_use(re->software.ob);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data EINA_UNUSED, void *image, int *w, int *h)
{
   if (!image)
     {
        *w = 0;
        *h = 0;
        return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->w;
   if (h) *h = ((Evas_GL_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;

   if (!im) return NULL;
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
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

   switch (eng_image_colorspace_get(data, image))
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
   if (im_old)
     {
        im = evas_gl_common_image_new(gl_context, w, h,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = image;

   if (!image) return NULL;
   if (im->native.data) return image;
   re->window_use(re->software.ob);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im;
   int error;

   if (!image)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }

#ifdef GL_GLES
   re->window_use(re->software.ob);

   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.img) && (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        void *disp;

        if (im->tex->pt->dyn.checked_out > 0)
          {
             im->tex->pt->dyn.checked_out++;
             *image_data = im->tex->pt->dyn.data;
             if (err) *err = EVAS_LOAD_ERROR_NONE;
             return im;
          }
        disp = re->window_egl_display_get(re->software.ob);
        *image_data = im->tex->pt->dyn.data = secsym_eglMapImageSEC(disp,
                                                                    im->tex->pt->dyn.img, 
                                                                    EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC, 
                                                                    EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC);

        if (!im->tex->pt->dyn.data)
          {
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
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
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }

   re->window_use(re->software.ob);
#endif

   /* Engine can be fail to create texture after cache drop like eng_image_content_hint_set function,
        so it is need to add code which check im->im's NULL value*/

   if (!im->im)
    {
       *image_data = NULL;
       if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
       return NULL;
    }

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->im->cache_entry))
     error = evas_cache2_image_load_data(&im->im->cache_entry);
   else
#endif
     error = evas_cache_image_load_data(&im->im->cache_entry);
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
                   Evas_GL_Image *im_new;

                   im_new = evas_gl_common_image_new_from_copied_data
                      (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
                       im->im->image.data,
                       eng_image_alpha_get(data, image),
                       eng_image_colorspace_get(data, image));
                   if (!im_new)
                     {
                        *image_data = NULL;
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
         *image_data = NULL;
         break;
      default:
         abort();
         break;
     }
   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im, *im2;

   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   re->window_use(re->software.ob);
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
                      void *disp;

                      disp = re->window_egl_display_get(re->software.ob);
                      secsym_eglUnmapImageSEC(disp,
                                              im->tex->pt->dyn.img,
                                              EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC);
                   }
#endif
               }

             return image;
          }
        im2 = eng_image_new_from_data(data, im->w, im->h, image_data,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        if (!im2) return im;
        evas_gl_common_image_free(im);
        im = im2;
        evas_gl_common_image_dirty(im, 0, 0, 0, 0);
        return im;
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if ((!im->im) || (image_data != im->im->image.data))
           {
              im2 = eng_image_new_from_data(data, im->w, im->h, image_data,
                                            eng_image_alpha_get(data, image),
                                            eng_image_colorspace_get(data, image));
              if (!im2) return im;
              evas_gl_common_image_free(im);
              im = im2;
           }
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
         break;
      default:
         abort();
         break;
     }
   return im;
}

static void
eng_image_data_preload_request(void *data, void *image, const Eo *target)
{
   Evas_GL_Image *gim = image;
   Render_Engine_GL_Generic *re = data;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
     evas_cache2_image_preload_data(&im->cache_entry, target);
   else
#endif
     evas_cache_image_preload_data(&im->cache_entry, target, NULL, NULL, NULL);
   if (!gim->tex)
     {
        Evas_Engine_GL_Context *gl_context;

        re->window_use(re->software.ob);
        gl_context = re->window_gl_context_get(re->software.ob);
        gim->tex = evas_gl_common_texture_new(gl_context, gim->im);
     }
   evas_gl_preload_target_register(gim->tex, (Eo*) target);
}

static void
eng_image_data_preload_cancel(void *data EINA_UNUSED, void *image, const Eo *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
     evas_cache2_image_preload_cancel(&im->cache_entry, target);
   else
#endif
     evas_cache_image_preload_cancel(&im->cache_entry, target);
   evas_gl_preload_target_unregister(gim->tex, (Eo*) target);
}

static Eina_Bool
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = image;
   Evas_Native_Surface *n;

   if (!im) return EINA_FALSE;
   n = im->native.data;

   gl_context = re->window_gl_context_get(re->software.ob);
   re->window_use(re->software.ob);

   if ((n) && (n->type == EVAS_NATIVE_SURFACE_OPENGL) &&
       (n->data.opengl.framebuffer_id == 0) &&
       re->func.get_pixels)
     {
        DBG("Rendering Directly to the window: %p", data);

        gl_context->dc = context;

        if (re->func.get_pixels)
          {
             if ((gl_context->master_clip.enabled) &&
                 (gl_context->master_clip.w > 0) &&
                 (gl_context->master_clip.h > 0))
               {
                  // Pass the preserve flag info the evas_gl
                  evgl_direct_partial_info_set(gl_context->preserve_bit);
               }

             // Set necessary info for direct rendering
             evgl_direct_info_set(gl_context->w,
                                  gl_context->h,
                                  gl_context->rot,
                                  dst_x, dst_y, dst_w, dst_h,
                                  gl_context->dc->clip.x,
                                  gl_context->dc->clip.y,
                                  gl_context->dc->clip.w,
                                  gl_context->dc->clip.h);

             // Call pixel get function
             re->func.get_pixels(re->func.get_pixels_data, re->func.obj);

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
eng_image_scale_hint_set(void *data EINA_UNUSED, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static int
eng_image_scale_hint_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static Eina_Bool
eng_image_map_draw(void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level, Eina_Bool do_async)
{
   Evas_Engine_GL_Context *gl_context;
   Evas_GL_Image *gim = image;
   Render_Engine_GL_Generic *re = data;

   if (!image) return EINA_FALSE;
   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
   if (m->count != 4)
     {
        // FIXME: nash - you didn't fix this
        abort();
     }
   if ((m->pts[0].x == m->pts[3].x) &&
       (m->pts[1].x == m->pts[2].x) &&
       (m->pts[0].y == m->pts[1].y) &&
       (m->pts[3].y == m->pts[2].y) &&
       (m->pts[0].x <= m->pts[1].x) &&
       (m->pts[0].y <= m->pts[2].y) &&
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
        eng_image_draw(data, context, surface, image,
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
eng_image_map_clean(void *data EINA_UNUSED, RGBA_Map *m EINA_UNUSED)
{
}

static void *
eng_image_map_surface_new(void *data, int w, int h, int alpha)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   return evas_gl_common_image_surface_new(gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data, void *surface)
{
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   evas_gl_common_image_free(surface);
}

static void
eng_image_content_hint_set(void *data, void *image, int hint)
{
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   evas_gl_common_image_content_hint_set(image, hint);
}

static int
eng_image_content_hint_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_CONTENT_HINT_NONE;
   return gim->content_hint;
}

static void
eng_image_cache_flush(void *data)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;
   int tmp_size;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_gl_common_image_cache_flush(gl_context);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);

   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
   evas_gl_common_image_cache_flush(gl_context);
}

static int
eng_image_cache_get(void *data EINA_UNUSED)
{
   return evas_common_image_get_cache();
}

static void
eng_image_stride_get(void *data EINA_UNUSED, void *image, int *stride)
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
           default:
             ERR("Requested stride on an invalid format %d", im->cs.space);
             *stride = 0;
             return;
          }
     }
}

static Eina_Bool
eng_font_draw(void *data, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Text_Props *intl_props, Eina_Bool do_async EINA_UNUSED)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);

   evas_gl_common_context_target_surface_set(gl_context, surface);
   gl_context->dc = context;
     {
        // FIXME: put im into context so we can free it
        static RGBA_Image *im = NULL;

        if (!im)
          im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.w = gl_context->shared->w;
        im->cache_entry.h = gl_context->shared->h;

        evas_common_draw_context_font_ext_set(context,
                                              gl_context,
                                              evas_gl_font_texture_new,
                                              evas_gl_font_texture_free,
                                              evas_gl_font_texture_draw);
        evas_common_font_draw_prepare(intl_props);
        evas_common_font_draw(im, context, x, y, intl_props->glyphs);
        evas_common_draw_context_font_ext_set(context,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL);
     }

   return EINA_FALSE;
}

//--------------------------------//
// Evas GL Related Code
static int
evgl_init(Render_Engine_GL_Generic *re)
{
   if (re->evgl_initted) return 1;
   if (!evgl_engine_init(re, re->evgl_funcs)) return 0;
   re->evgl_initted = EINA_TRUE;
   return 1;
}

#define EVGLINIT(_re, _ret) if (!evgl_init(_re)) return _ret

static void *
eng_gl_surface_create(void *data, void *config, int w, int h)
{
   Evas_GL_Config *cfg = (Evas_GL_Config *)config;

   EVGLINIT(data, NULL);
   return evgl_surface_create(data, cfg, w, h);
}

static int
eng_gl_surface_destroy(void *data, void *surface)
{
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;

   EVGLINIT(data, 0);
   return evgl_surface_destroy(data, sfc);
}

static void *
eng_gl_context_create(void *data, void *share_context)
{
   EVGL_Context  *sctx = (EVGL_Context *)share_context;

   EVGLINIT(data, NULL);
   return evgl_context_create(data, sctx);
}

static int
eng_gl_context_destroy(void *data, void *context)
{
   EVGL_Context  *ctx = (EVGL_Context *)context;

   EVGLINIT(data, 0);
   return evgl_context_destroy(data, ctx);
}

static int
eng_gl_make_current(void *data, void *surface, void *context)
{
   Render_Engine_GL_Generic *re  = data;
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;
   EVGL_Context  *ctx = (EVGL_Context *)context;

   EVGLINIT(data, 0);
   if ((sfc) && (ctx))
     {
        Evas_Engine_GL_Context *gl_context;

        gl_context = re->window_gl_context_get(re->software.ob);
        if ((gl_context->havestuff) ||
            (gl_context->master_clip.used))
          {
             re->window_use(re->software.ob);
             evas_gl_common_context_flush(gl_context);
             if (gl_context->master_clip.used)
                evas_gl_common_context_done(gl_context);
          }
     }

   return evgl_make_current(data, sfc, ctx);
}

static void *
eng_gl_string_query(void *data, int name)
{
   EVGLINIT(data, NULL);
   return (void *)evgl_string_query(name);
}

// Need to deprecate this function..
static void *
eng_gl_proc_address_get(void *data EINA_UNUSED, const char *name EINA_UNUSED)
{
   return NULL;
}

static int
eng_gl_native_surface_get(void *data EINA_UNUSED, void *surface, void *native_surface)
{
   EVGL_Surface  *sfc = (EVGL_Surface *)surface;
   Evas_Native_Surface *ns = (Evas_Native_Surface *)native_surface;

   return evgl_native_surface_get(sfc, ns);
}

static void *
eng_gl_api_get(void *data)
{
   EVGLINIT(data, NULL);
   return evgl_api_get();
}


static void
eng_gl_direct_override_get(void *data, int *override, int *force_off)
{
   EVGLINIT(data, );
   evgl_direct_override_get(override, force_off);
}

static void
eng_gl_get_pixels_set(void *data, void *get_pixels, void *get_pixels_data, void *obj)
{
   Render_Engine_GL_Generic *re = data;

   EVGLINIT(data, );
   re->func.get_pixels = get_pixels;
   re->func.get_pixels_data = get_pixels_data;
   re->func.obj = (Evas_Object*)obj;
}

static Eina_Bool
eng_gl_surface_lock(void *data, void *surface)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = surface;

   EVGLINIT(re, EINA_FALSE);
   if (!im->tex || !im->tex->pt)
     {
        ERR("Can not lock image that is not a surface!");
        return EINA_FALSE;
     }

   evas_gl_common_context_flush(im->gc);
   im->locked = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
eng_gl_surface_unlock(void *data, void *surface)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = surface;

   EVGLINIT(re, EINA_FALSE);
   im->locked = EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
eng_gl_surface_read_pixels(void *data, void *surface,
                           int x, int y, int w, int h,
                           Evas_Colorspace cspace, void *pixels)
{
   Render_Engine_GL_Generic *re = data;
   Evas_GL_Image *im = surface;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pixels, EINA_FALSE);

   EVGLINIT(re, EINA_FALSE);
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

#ifdef GL_GLES
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
#endif

   /* Since this is an FBO, the pixels are already in the right Y order.
    * But some devices don't support GL_BGRA, so we still need to convert.
    */

   glsym_glBindFramebuffer(GL_FRAMEBUFFER, im->tex->pt->fb);
   if (im->tex->pt->format == GL_BGRA)
     glReadPixels(x, y, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
   else
     {
        DATA32 *ptr = pixels;
        int k;

        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        for (k = w * h; k; --k)
          {
             const DATA32 v = *ptr;
             *ptr++ = (v & 0xFF00FF00)
                   | ((v & 0x00FF0000) >> 16)
                   | ((v & 0x000000FF) << 16);
          }
     }
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);

   return EINA_TRUE;
}
//--------------------------------//

static int
eng_image_load_error_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *im;

   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->im->cache_entry.load_error;
}

static Eina_Bool
eng_image_animated_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   return im->animated.animated;
}

static int
eng_image_animated_frame_count_get(void *data EINA_UNUSED, void *image)
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
eng_image_animated_loop_type_get(void *data EINA_UNUSED, void *image)
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
eng_image_animated_loop_count_get(void *data EINA_UNUSED, void *image)
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
eng_image_animated_frame_duration_get(void *data EINA_UNUSED, void *image, int start_frame, int frame_num)
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
eng_image_animated_frame_set(void *data EINA_UNUSED, void *image, int frame_index)
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

static Eina_Bool
eng_image_can_region_get(void *data EINA_UNUSED, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;
   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;
   return ((Evas_Image_Load_Func*) im->info.loader)->do_region;
}


static void
eng_image_max_size_get(void *data, int *maxw, int *maxh)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   gl_context = re->window_gl_context_get(re->software.ob);
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

#ifdef EVAS_CSERVE2
          if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->im->cache_entry))
            evas_cache2_image_load_data(&im->im->cache_entry);
          else
#endif
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

static void
eng_context_flush(void *data)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   gl_context = re->window_gl_context_get(re->software.ob);

   if ((gl_context->havestuff) ||
     (gl_context->master_clip.used))
   {
      re->window_use(re->software.ob);
      evas_gl_common_context_flush(gl_context);
      if (gl_context->master_clip.used)
         evas_gl_common_context_done(gl_context);
   }
}

static void
eng_context_3d_use(void *data)
{
   Render_Engine_GL_Generic *re = data;

   if (!re->context_3d)
     re->context_3d = re->window_gl_context_new(re->software.ob);
   if (re->context_3d) re->window_gl_context_use(re->context_3d);
}

static E3D_Renderer *
eng_renderer_3d_get(void *data)
{
   Render_Engine_GL_Generic *re = data;

   if (!re->renderer_3d)
     re->renderer_3d = e3d_renderer_new();
   return re->renderer_3d;
}

static void *
eng_drawable_new(void *data, int w, int h, int alpha)
{
   eng_context_3d_use(data);
#ifdef GL_GLES
   return e3d_drawable_new(w, h, alpha, GL_DEPTH_STENCIL_OES, GL_NONE);
#else
   return e3d_drawable_new(w, h, alpha, GL_DEPTH24_STENCIL8, GL_NONE);
#endif
}

static void
eng_drawable_free(void *data, void *drawable)
{
   eng_context_3d_use(data);
   e3d_drawable_free(drawable);
}

static void
eng_drawable_size_get(void *data EINA_UNUSED, void *drawable, int *w, int *h)
{
   e3d_drawable_size_get((E3D_Drawable *)drawable, w, h);
}

static void *
eng_image_drawable_set(void *data, void *image, void *drawable)
{
   E3D_Drawable *d = drawable;
   Evas_Native_Surface ns;
   int w, h;

   ns.type = EVAS_NATIVE_SURFACE_OPENGL;
   ns.data.opengl.texture_id = e3d_drawable_texture_id_get(d);
   ns.data.opengl.framebuffer_id = 0;
   ns.data.opengl.internal_format = e3d_drawable_format_get(d);
   ns.data.opengl.format = e3d_drawable_format_get(d);
   ns.data.opengl.x = 0;
   ns.data.opengl.y = 0;
   e3d_drawable_size_get(d, &w, &h);
   ns.data.opengl.w = w;
   ns.data.opengl.h = h;

   return eng_image_native_set(data, image, &ns);
}

static void
eng_drawable_scene_render(void *data, void *drawable, void *scene_data)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;
   E3D_Renderer *renderer = NULL;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_flush(gl_context);

   eng_context_3d_use(data);
   renderer = eng_renderer_3d_get(data);
   e3d_drawable_scene_render(drawable, renderer, scene_data);
}

static void *
eng_texture_new(void *data EINA_UNUSED)
{
   return e3d_texture_new();
}

static void
eng_texture_free(void *data EINA_UNUSED, void *texture)
{
   e3d_texture_free((E3D_Texture *)texture);
}

static void
eng_texture_data_set(void *data, void *texture, Evas_3D_Color_Format color_format,
                     Evas_3D_Pixel_Format pixel_format, int w, int h, const void *pixels)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_flush(gl_context);
   eng_context_3d_use(data);

   e3d_texture_data_set((E3D_Texture *)texture, color_format, pixel_format, w, h, pixels);
}

static void
eng_texture_file_set(void *data, void *texture, const char *file, const char *key)
{
   Evas_Engine_GL_Context *gl_context;
   Render_Engine_GL_Generic *re = data;

   re->window_use(re->software.ob);
   gl_context = re->window_gl_context_get(re->software.ob);
   evas_gl_common_context_flush(gl_context);
   eng_context_3d_use(data);

   e3d_texture_file_set((E3D_Texture *)texture, file, key);
}

static void
eng_texture_color_format_get(void *data EINA_UNUSED, void *texture, Evas_3D_Color_Format *format)
{
   *format = e3d_texture_color_format_get((E3D_Texture *)texture);
}

static void
eng_texture_size_get(void *data EINA_UNUSED, void *texture, int *w, int *h)
{
   e3d_texture_size_get((E3D_Texture *)texture, w, h);
}

static void
eng_texture_wrap_set(void *data EINA_UNUSED, void *texture,
                     Evas_3D_Wrap_Mode s, Evas_3D_Wrap_Mode t)
{
   e3d_texture_wrap_set((E3D_Texture *)texture, s, t);
}

static void
eng_texture_wrap_get(void *data EINA_UNUSED, void *texture,
                     Evas_3D_Wrap_Mode *s, Evas_3D_Wrap_Mode *t)
{
   e3d_texture_wrap_get((E3D_Texture *)texture, s, t);
}

static void
eng_texture_filter_set(void *data EINA_UNUSED, void *texture,
                       Evas_3D_Texture_Filter min, Evas_3D_Texture_Filter mag)
{
   e3d_texture_filter_set((E3D_Texture *)texture, min, mag);
}

static void
eng_texture_filter_get(void *data EINA_UNUSED, void *texture,
                       Evas_3D_Texture_Filter *min, Evas_3D_Texture_Filter *mag)
{
   e3d_texture_filter_get((E3D_Texture *)texture, min, mag);
}

static void
eng_texture_image_set(void *data EINA_UNUSED, void *texture, void *image)
{
   Evas_GL_Image *im = (Evas_GL_Image *)image;
   e3d_texture_import((E3D_Texture *)texture, im->tex->pt->texture);
}

static Evas_Func func, pfunc;

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   if (_evas_engine_GL_log_dom < 0)
     _evas_engine_GL_log_dom = eina_log_domain_register("evas-gl", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
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
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_can_region_get);
   ORD(image_native_set);
   ORD(image_native_get);

   ORD(font_draw);

   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   ORD(image_stride_get);

   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);
   ORD(image_map_clean);

   ORD(image_content_hint_set);
   ORD(image_content_hint_get);

   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);

   ORD(gl_surface_create);
   ORD(gl_surface_destroy);
   ORD(gl_context_create);
   ORD(gl_context_destroy);
   ORD(gl_make_current);
   ORD(gl_string_query);
   ORD(gl_proc_address_get);
   ORD(gl_native_surface_get);
   ORD(gl_api_get);
   ORD(gl_direct_override_get);
   ORD(gl_get_pixels_set);
   ORD(gl_surface_lock);
   ORD(gl_surface_read_pixels);
   ORD(gl_surface_unlock);

   ORD(image_load_error_get);

   /* now advertise out own api */
   ORD(image_animated_get);
   ORD(image_animated_frame_count_get);
   ORD(image_animated_loop_type_get);
   ORD(image_animated_loop_count_get);
   ORD(image_animated_frame_duration_get);
   ORD(image_animated_frame_set);

   ORD(image_max_size_get);

   ORD(pixel_alpha_get);

   ORD(context_flush);

   /* 3D features */
   ORD(drawable_new);
   ORD(drawable_free);
   ORD(drawable_size_get);
   ORD(image_drawable_set);

   ORD(drawable_scene_render);

   ORD(texture_new);
   ORD(texture_free);
   ORD(texture_data_set);
   ORD(texture_file_set);
   ORD(texture_color_format_get);
   ORD(texture_size_get);
   ORD(texture_wrap_set);
   ORD(texture_wrap_get);
   ORD(texture_filter_set);
   ORD(texture_filter_get);
   ORD(texture_image_set);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   eina_log_domain_unregister(_evas_engine_GL_log_dom);
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
