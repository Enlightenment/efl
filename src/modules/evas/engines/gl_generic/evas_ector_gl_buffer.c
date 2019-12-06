#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define ECTOR_GL_BUFFER_BASE_PROTECTED

#include "evas_common_private.h"
#include "evas_gl_private.h"

#include <software/Ector_Software.h>
#include <gl/Ector_GL.h>
#include "Evas_Engine_GL_Generic.h"
#include "evas_ector_gl.h"
#include "evas_gl_private.h"

#define MY_CLASS EVAS_ECTOR_GL_BUFFER_CLASS

typedef struct _Ector_GL_Buffer_Map Ector_GL_Buffer_Map;
typedef struct _Evas_Ector_GL_Buffer_Data Evas_Ector_GL_Buffer_Data;

static int _map_id = 0;

typedef struct _Buffer_Cache
{
   char *buffer;
   int len;
   Eina_Bool use;
} Buffer_Cache;

////////////////////////////////////////////////////////////////////////////////////////////////
Buffer_Cache _buffer_cache[20];
static int _ector_cnt = 0;

char * _ector_buffer_get(int len)
{
   for (int i = 0; i < 10; i++)
     {
        if (_buffer_cache[i].use) continue;
        if (_buffer_cache[i].len == 0)
          {
             _buffer_cache[i].use = 1;
             _buffer_cache[i].buffer = malloc(len);
             _buffer_cache[i].len = len;
             printf("malloc = %d\n", len);
             return _buffer_cache[i].buffer;
          }

        if (_buffer_cache[i].len >= len)
          {
             _buffer_cache[i].use = 1;
             return _buffer_cache[i].buffer;
          }
        else
          {
             free(_buffer_cache[i].buffer);
             _buffer_cache[i].use = 1;
             _buffer_cache[i].buffer = malloc(len);
             _buffer_cache[i].len = len;
             printf("malloc = %d\n", len);

             return _buffer_cache[i].buffer;
          }
        printf("What?!\n");
     }

   return NULL;
}

void _ector_buffer_return(char *buffer)
{
   for (int i = 0; i < 10; i++)
     {
        if (_buffer_cache[i].buffer == buffer)
          {
             _buffer_cache[i].use = 0;
             return;
          }
     }
   printf("What?!\n");
}

static void _ector_buffer_term()
{
   for (int i = 0; i < 10; i++)
     {
        free(_buffer_cache[i].buffer);
        _buffer_cache[i].buffer = NULL;
        _buffer_cache[i].use = 0;
        _buffer_cache[i].len = 0;
     }
}

static void _ector_buffer_init()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////




struct _Ector_GL_Buffer_Map
{
   EINA_INLIST;
   void *ptr;
   unsigned int base_size; // in bytes
   unsigned int x, y, w, h;
   void *image_data, *base_data;
   int map_id;
   size_t length;
   Efl_Gfx_Colorspace cspace;
   Evas_GL_Image *im;
   Eina_Bool allocated;
   Ector_Buffer_Access_Flag mode;
};

struct _Evas_Ector_GL_Buffer_Data
{
   Render_Engine_GL_Generic *re;
   Evas_GL_Image *glim;
   Eina_Bool alpha_only, was_render;
   Ector_GL_Buffer_Map *maps;
   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used */
   int w, h;
   void *image_data;
   /* END */
};

void *eng_image_data_put(void *data, void *image, DATA32 *image_data);
void *eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err, Eina_Bool *tofree);
void eng_image_free(void *data, void *image);

#undef ENFN
#undef ENDT
#undef ENC

#define ENC pd->re

// testing out some macros to maybe add to eina
#define EINA_INLIST_REMOVE(l,i) do { l = (__typeof__(l)) eina_inlist_remove(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)
#define EINA_INLIST_APPEND(l,i) do { l = (__typeof__(l)) eina_inlist_append(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)
#define EINA_INLIST_PREPEND(l,i) do { l = (__typeof__(l)) eina_inlist_prepend(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)

#define fail(fmt, ...) do { ERR(fmt, ##__VA_ARGS__); goto on_fail; } while (0)

#if 0
static inline void
_mapped_image_dump(Eo *buf, Evas_GL_Image *im, const char *fmt, int id)
{
   if (!im || !im->im) return;
   evas_common_save_image_to_file(im->im, eina_slstr_printf("/tmp/dump/%s_%02d_buf_%p_im_%p.png", fmt, id, buf, im),
                                  NULL, 100, 9, NULL);
}

#define MAP_DUMP(_im, _fmt) _mapped_image_dump(obj, _im, _fmt, map->map_id)
#else
#define MAP_DUMP(...)
#endif

/* FIXME: Conversion routines don't belong here */
static inline void
_pixels_argb_to_gry8_convert(uint8_t *dst, const uint32_t *src, int len)
{
   int k;
   for (k = 0; k < len; k++)
     {
        const uint32_t *s = src++;
        *dst++ = A_VAL(s);
     }
}

static inline void
_pixels_gry8_to_argb_convert(uint32_t *dst, const uint8_t *src, int len)
{
   int k;
   for (k = 0; k < len; k++)
     {
        const uint8_t s = *src++;
        *dst++ = ARGB_JOIN(s, s, s, s);
     }
}

static inline Eina_Bool
_evas_gl_image_is_fbo(Evas_GL_Image *glim)
{
   return glim && glim->tex && glim->tex->pt && glim->tex->pt->fb;
}

EOLIAN static void
_evas_ector_gl_buffer_gl_buffer_prepare(Eo *obj, Evas_Ector_GL_Buffer_Data *pd,
                                        void *engine,
                                        int w, int h, Efl_Gfx_Colorspace cspace,
                                        Ector_Buffer_Flag flags EINA_UNUSED)
{
   Render_Engine_GL_Generic *re = engine;
   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image *im;

   // this is meant to be called only once
   EINA_SAFETY_ON_FALSE_GOTO(!pd->re, on_fail);
   EINA_SAFETY_ON_FALSE_GOTO(!efl_finalized_get(obj), on_fail);
    */
   EINA_SAFETY_ON_FALSE_RETURN(!pd->re);
   EINA_SAFETY_ON_FALSE_RETURN(!efl_finalized_get(obj));
   /* END */

   if (cspace == EFL_GFX_COLORSPACE_ARGB8888)
     pd->alpha_only = EINA_FALSE;
   else if (cspace == EFL_GFX_COLORSPACE_GRY8)
     pd->alpha_only = EINA_TRUE;
   else
     /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
     fail("Unsupported colorspace: %u", cspace);
      */
     {
        ERR("Unsupported colorspace: %u", cspace);
        return;
     }
     /* END */

   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   pd->re = re;
   gc = gl_generic_context_find(re, 1);
   im = evas_gl_common_image_surface_new(gc, w, h, EINA_TRUE, EINA_FALSE);
   if (!im) fail("Failed to create GL surface!");

   pd->glim = im;
   return;

on_fail:
   evas_gl_common_image_free(pd->glim);
   pd->glim = NULL;
    */
   pd->image_data = calloc(1, w * h * 4);
   pd->w = w;
   pd->h = h;
   pd->re = re;
   pd->glim = NULL;
   /* END */
}

static inline void *
_image_get(Evas_Ector_GL_Buffer_Data *pd, Eina_Bool render)
{
   if (pd->maps != NULL)
     fail("Image is currently mapped!");

   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   if (!pd->glim || !pd->glim->tex || !pd->glim->tex->pt)
     fail("Image has no texture!");
    */

   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used */
   if (pd->glim == NULL)
     {
        Evas_Engine_GL_Context *gc = gl_generic_context_find(pd->re, 1);

        if (render)
          {
             Evas_GL_Image *old_glim = NULL, *im = NULL;
             Eina_Bool tofree = EINA_FALSE;
             DATA32 *data = NULL;
             int err;

             pd->glim = evas_gl_common_image_surface_new(gc, pd->w, pd->h, EINA_TRUE, EINA_FALSE);
             im = eng_image_data_get(ENC, pd->glim, EINA_FALSE, &data, &err, &tofree);
             old_glim = pd->glim;

             memcpy(data, pd->image_data, pd->w * pd->h * 4);
             pd->glim = evas_gl_common_image_surface_update(im);
             if (tofree)
               evas_gl_common_image_free(old_glim);

             free(pd->image_data);
             pd->image_data = NULL;
          }
        else
          {
             gc = gl_generic_context_find(pd->re, 1);
             pd->glim = evas_gl_common_image_new_from_data(gc, pd->w, pd->h, pd->image_data, EINA_TRUE, EFL_GFX_COLORSPACE_ARGB8888);
             evas_gl_common_image_dirty(pd->glim, 0, 0, 0, 0);
             evas_gl_common_image_update(gc, pd->glim);
          }
     }
   /* END */

   evas_gl_common_image_ref(pd->glim);
   if (render)
     {
        if (!pd->glim->tex->pt->fb)
          fail("Image has no FBO!");
        pd->was_render = EINA_TRUE;
     }
   return pd->glim;

on_fail:
   return NULL;
}

EOLIAN static void *
_evas_ector_gl_buffer_evas_ector_buffer_drawable_image_get(Eo *obj EINA_UNUSED,
                                                           Evas_Ector_GL_Buffer_Data *pd)
{
   return _image_get(pd, EINA_FALSE);
}

EOLIAN static void *
_evas_ector_gl_buffer_evas_ector_buffer_render_image_get(Eo *obj EINA_UNUSED,
                                                         Evas_Ector_GL_Buffer_Data *pd)
{
   return _image_get(pd, EINA_TRUE);
}

EOLIAN static Eina_Bool
_evas_ector_gl_buffer_evas_ector_buffer_engine_image_release(Eo *obj EINA_UNUSED,
                                                             Evas_Ector_GL_Buffer_Data *pd,
                                                             void *image)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->glim == image, EINA_FALSE);

   if (pd->was_render)
     pd->glim = evas_gl_common_image_surface_detach(pd->glim);

   evas_gl_common_image_free(pd->glim);

   return EINA_TRUE;
}

EOLIAN static void
_evas_ector_gl_buffer_ector_buffer_size_get(const Eo *obj EINA_UNUSED,
                                            Evas_Ector_GL_Buffer_Data *pd,
                                            int *w, int *h)
{
   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   if (w) *w = pd->glim->w;
   if (h) *h = pd->glim->h;
    */
   if (w) *w = pd->w;
   if (h) *h = pd->h;
   /* END */
}

EOLIAN static Efl_Gfx_Colorspace
_evas_ector_gl_buffer_ector_buffer_cspace_get(const Eo *obj EINA_UNUSED,
                                              Evas_Ector_GL_Buffer_Data *pd)
{
   if (pd->alpha_only)
     return EFL_GFX_COLORSPACE_GRY8;
   else
     return EFL_GFX_COLORSPACE_ARGB8888;
}

EOLIAN static Ector_Buffer_Flag
_evas_ector_gl_buffer_ector_buffer_flags_get(const Eo *obj EINA_UNUSED,
                                             Evas_Ector_GL_Buffer_Data *pd EINA_UNUSED)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE | ECTOR_BUFFER_FLAG_DRAWABLE |
         ECTOR_BUFFER_FLAG_CPU_WRITABLE | ECTOR_BUFFER_FLAG_RENDERABLE;
}

EOLIAN static void *
_evas_ector_gl_buffer_ector_buffer_map(Eo *obj EINA_UNUSED, Evas_Ector_GL_Buffer_Data *pd,
                                       unsigned int *length,
                                       Ector_Buffer_Access_Flag mode,
                                       unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                       Efl_Gfx_Colorspace cspace, unsigned int *stride)
{
   Eina_Bool write = !!(mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE);
   Ector_GL_Buffer_Map *map = NULL;
   Eina_Bool tofree = EINA_FALSE;
   Evas_GL_Image *im = NULL;
   unsigned int W, H;
   int len, err;
   uint32_t *data;
   int pxs;

   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   W = pd->glim->w;
   H = pd->glim->h;
    */
   W = pd->w;
   H = pd->h;
   /* END */
   if (!w) w = W - x;
   if (!h) h = H - y;
   if ((x + w > W) || (y + h > H)) return NULL;

   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   if (write && _evas_gl_image_is_fbo(pd->glim))
     {
        // Can not open FBO data to write!
        im = eng_image_data_get(ENC, pd->glim, EINA_FALSE, &data, &err, &tofree);
        if (!im) return NULL;
     }
   else
     {
        im = eng_image_data_get(ENC, pd->glim, write, &data, &err, &tofree);
        if (!im) return NULL;
     }
    */
   if (pd->glim)
     {
        if (write && _evas_gl_image_is_fbo(pd->glim))
          {
             // Can not open FBO data to write!
             im = eng_image_data_get(ENC, pd->glim, EINA_FALSE, &data, &err, &tofree);
             if (!im) return NULL;
          }
        else
          {
             im = eng_image_data_get(ENC, pd->glim, write, &data, &err, &tofree);
             if (!im) return NULL;
          }
     }
   else
     {
        data = pd->image_data;
        tofree = EINA_FALSE;
     }
   /* END */

   map = calloc(1, sizeof(*map));
   map->mode = mode;
   map->cspace = cspace;
   map->x = x;
   map->y = y;
   map->w = w;
   map->h = h;
   map->image_data = data;
   map->im = tofree ? im : NULL;

   len = W * H;
   if (cspace == EFL_GFX_COLORSPACE_GRY8)
     {
//        uint8_t *data8 = _ector_buffer_get(len);
        uint8_t *data8 = malloc(len);
        if (!data8) goto on_fail;
        _pixels_argb_to_gry8_convert(data8, data, len);
        map->allocated = EINA_TRUE;
        map->base_data = data8;
        map->ptr = data8 + x + (y * W);
        pxs = 1;
     }
   else
     {
        map->allocated = EINA_FALSE;
        map->base_data = data;
        map->ptr = data + x + (y * W);
        pxs = 4;
     }

   map->map_id = ++_map_id;
   map->base_size = len * pxs;
   map->length = (W * h + w - W) * pxs;
   if (stride) *stride = W * pxs;
   if (length) *length = map->length;

   MAP_DUMP(im, "in");

   EINA_INLIST_PREPEND(pd->maps, map);
   return map->ptr;

on_fail:
   free(map);
   return NULL;
}

EOLIAN static void
_evas_ector_gl_buffer_ector_buffer_unmap(Eo *obj EINA_UNUSED, Evas_Ector_GL_Buffer_Data *pd,
                                         void *data, unsigned int length)
{
   Ector_GL_Buffer_Map *map;
   if (!data) return;

   EINA_INLIST_FOREACH(pd->maps, map)
     {
        if ((map->ptr == data) && (map->length == length))
          {
             EINA_INLIST_REMOVE(pd->maps, map);
             if (map->mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE)
               {
                  Evas_GL_Image *old_glim = pd->glim;
                  int W, H;

                  /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
                  W = pd->glim->w;
                  H = pd->glim->h;
                   */
                  W = pd->w;
                  H = pd->h;
                  /* END */

                  if (map->cspace == EFL_GFX_COLORSPACE_GRY8)
                    _pixels_gry8_to_argb_convert(map->image_data, map->base_data, W * H);

                  /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
                  if (map->im)
                    {
                       MAP_DUMP(map->im, "out_w_free");
                       pd->glim = evas_gl_common_image_surface_update(map->im);
                       evas_gl_common_image_free(old_glim);
                    }
                  else
                    {
                       MAP_DUMP(old_glim, "out_w_nofree");
                       pd->glim = evas_gl_common_image_surface_update(old_glim);
                    }
                   */
                  if (pd->glim)
                    {
                       if (pd->was_render)
                         {
                            if (map->im)
                              {
                                 MAP_DUMP(map->im, "out_w_free");
                                 pd->glim = evas_gl_common_image_surface_update(map->im);
                                 evas_gl_common_image_free(old_glim);
                              }
                            else
                              {
                                 MAP_DUMP(old_glim, "out_w_nofree");
                                 pd->glim = evas_gl_common_image_surface_update(old_glim);
                              }
                         }
                       else
                         {
                            if (map->im)
                              {
                                 MAP_DUMP(map->im, "out_w_free");
                                 pd->glim = eng_image_data_put(ENC, map->im, map->image_data);

                                 if (pd->image_data != map->image_data)
                                   {
                                      free(pd->image_data);
                                      pd->image_data = map->image_data;
                                   }

                                 evas_gl_common_image_free(old_glim);
                                 evas_gl_common_image_dirty(pd->glim, 0, 0, 0, 0);
                                 evas_gl_common_image_update(pd->glim->gc, pd->glim);
                              }
                            else
                              {
                                 MAP_DUMP(old_glim, "out_w_nofree");
                                 pd->glim = eng_image_data_put(ENC, pd->glim, map->image_data);

                                 if (pd->image_data != map->image_data)
                                   {
                                      free(pd->image_data);
                                      pd->image_data = map->image_data;
                                   }

                                 evas_gl_common_image_dirty(pd->glim, 0, 0, 0, 0);
                                 evas_gl_common_image_update(pd->glim->gc, pd->glim);
                              }
                         }
                    }
                  /* END */
               }
             else
               {
                  /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
                  if (map->im)
                    {
                       MAP_DUMP(map->im, "out_ro_free");
                       eng_image_free(ENC, map->im);
                    }
                  else
                    {
                       MAP_DUMP(pd->glim, "out_ro_nofree");
                       pd->glim = eng_image_data_put(ENC, pd->glim, map->image_data);
                    }
                   */
                  if (pd->glim)
                    {
                       if (pd->was_render)
                         {
                            if (map->im)
                              {
                                 MAP_DUMP(map->im, "out_ro_free");
                                 eng_image_free(ENC, map->im);
                              }
                            else
                              {
                                 MAP_DUMP(pd->glim, "out_ro_nofree");
                                 pd->glim = eng_image_data_put(ENC, pd->glim, map->image_data);
                              }
                         }
                       else
                         {
                            if (map->im)
                              {
                                 MAP_DUMP(map->im, "out_ro_free");
                                 eng_image_free(ENC, map->im);
                              }
                            else
                              {
                                 MAP_DUMP(pd->glim, "out_ro_nofree");
                                 pd->glim = eng_image_data_put(ENC, pd->glim, map->image_data);

                                 if (pd->image_data != map->image_data)
                                   {
                                      free(pd->image_data);
                                      pd->image_data = map->image_data;
                                   }

                                 evas_gl_common_image_dirty(pd->glim, 0, 0, 0, 0);
                                 evas_gl_common_image_update(pd->glim->gc, pd->glim);
                              }
                         }
                    }
                  /* END */
               }
             if (map->allocated)
               {
                  free(map->base_data);
               }
             free(map);
             return;
          }
     }

   ERR("Tried to unmap a non-mapped region!");
}

EOLIAN static Efl_Object *
_evas_ector_gl_buffer_efl_object_finalize(Eo *obj, Evas_Ector_GL_Buffer_Data *pd)
{
   if (_ector_cnt == 0)
     _ector_buffer_init();

   ++_ector_cnt;
//   printf("ector_cnt = %d", _ector_cnt);
   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used
   if (!pd->glim)
     {
        ERR("Buffer was not initialized properly!");
        return NULL;
     }
    */
   /* END */
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_gl_buffer_efl_object_destructor(Eo *obj, Evas_Ector_GL_Buffer_Data *pd)
{
   --_ector_cnt;
   evas_gl_common_image_free(pd->glim);
   /* TIZEN_ONLY(20181130): evas ector - create a gl image when only it is going to be used */
   free(pd->image_data);
   pd->image_data = NULL;
   /* END */
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "evas_ector_gl_buffer.eo.c"
