#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* this one is read-only buffer wrapping an existing evas_gl_image */

#define ECTOR_GL_BUFFER_BASE_PROTECTED

#include "evas_common_private.h"
#include "evas_gl_private.h"

#include <software/Ector_Software.h>
#include <gl/Ector_GL.h>
#include "Evas_Engine_GL_Generic.h"
#include "evas_ector_gl.h"
#include "evas_gl_private.h"

#define MY_CLASS EVAS_ECTOR_GL_IMAGE_BUFFER_CLASS

typedef struct _Ector_GL_Buffer_Map Ector_GL_Buffer_Map;
typedef struct _Evas_Ector_GL_Image_Buffer_Data Evas_Ector_GL_Image_Buffer_Data;

struct _Ector_GL_Buffer_Map
{
   EINA_INLIST;
   void *ptr;
   unsigned int base_size; // in bytes
   unsigned int x, y, w, h;
   void *image_data, *base_data;
   size_t length;
   Efl_Gfx_Colorspace cspace;
   Evas_GL_Image *im;
   Eina_Bool allocated, free_image;
   Ector_Buffer_Access_Flag mode;
};

struct _Evas_Ector_GL_Image_Buffer_Data
{
   Render_Output_GL_Generic *re;
   Evas_GL_Image *glim;
   Ector_GL_Buffer_Map *maps;
};

#undef ENFN
#undef ENDT
#undef ENC

#define ENC  pd->re

void *eng_image_data_put(void *data, void *image, DATA32 *image_data);
void *eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err, Eina_Bool *tofree);
void eng_image_free(void *data, void *image);

// testing out some macros to maybe add to eina
#define EINA_INLIST_REMOVE(l,i) do { l = (__typeof__(l)) eina_inlist_remove(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)
#define EINA_INLIST_APPEND(l,i) do { l = (__typeof__(l)) eina_inlist_append(EINA_INLIST_GET(l), EINA_INLIST_GET(i)); } while (0)

#define fail(fmt, ...) do { ERR(fmt, ##__VA_ARGS__); goto on_fail; } while (0)

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

EOLIAN static void
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_set(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_GL_Image_Buffer_Data *pd,
                                                               void *engine, void *image)
{
   Render_Output_GL_Generic *re = engine;
   Evas_GL_Image *im = image;

   EINA_SAFETY_ON_FALSE_RETURN(!pd->glim);
   EINA_SAFETY_ON_NULL_RETURN(im);

   if (!im->tex)
     {
        Evas_Engine_GL_Context *gc;

        gc = re->window_gl_context_get(re->software.ob);
        evas_gl_common_image_update(gc, im);

        if (!im->tex)
          fail("Image has no texture!");
     }

   pd->re = re;
   evas_gl_common_image_ref(im);
   pd->glim = im;

 on_fail:
   return;
}

EOLIAN static void *
_evas_ector_gl_image_buffer_evas_ector_buffer_drawable_image_get(Eo *obj EINA_UNUSED,
                                                                 Evas_Ector_GL_Image_Buffer_Data *pd)
{
   if (!pd->glim->tex)
     fail("Image has no texture!");

   evas_gl_common_image_ref(pd->glim);
   return pd->glim;

on_fail:
   return NULL;
}

EOLIAN static Eina_Bool
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_release(Eo *obj EINA_UNUSED,
                                                                   Evas_Ector_GL_Image_Buffer_Data *pd,
                                                                   void *image)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->glim == image, EINA_FALSE);

   evas_gl_common_image_free(pd->glim);
   return EINA_TRUE;
}

EOLIAN static Ector_Buffer_Flag
_evas_ector_gl_image_buffer_ector_buffer_flags_get(const Eo *obj EINA_UNUSED,
                                                   Evas_Ector_GL_Image_Buffer_Data *pd EINA_UNUSED)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE | ECTOR_BUFFER_FLAG_DRAWABLE;
}

EOLIAN static void
_evas_ector_gl_image_buffer_ector_buffer_size_get(const Eo *obj EINA_UNUSED,
                                                  Evas_Ector_GL_Image_Buffer_Data *pd,
                                                  int *w, int *h)
{
   if (w) *w = pd->glim->w;
   if (h) *h = pd->glim->h;
}

EOLIAN static Efl_Gfx_Colorspace
_evas_ector_gl_image_buffer_ector_buffer_cspace_get(const Eo *obj EINA_UNUSED,
                                                    Evas_Ector_GL_Image_Buffer_Data *pd EINA_UNUSED)
{
   return EFL_GFX_COLORSPACE_ARGB8888;
}

EOLIAN static void *
_evas_ector_gl_image_buffer_ector_buffer_map(Eo *obj EINA_UNUSED, Evas_Ector_GL_Image_Buffer_Data *pd, unsigned int *length,
                                             Ector_Buffer_Access_Flag mode,
                                             unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                             Efl_Gfx_Colorspace cspace, unsigned int *stride)
{
   Ector_GL_Buffer_Map *map = NULL;
   Eina_Bool tofree = EINA_FALSE;
   Evas_GL_Image *im = NULL;
   unsigned int W, H;
   int len, err;
   uint32_t *data;
   int pxs;

   if ((cspace != EFL_GFX_COLORSPACE_GRY8) && (cspace != EFL_GFX_COLORSPACE_ARGB8888))
     {
        ERR("Unsupported colorspace for map: %d", (int) cspace);
        return NULL;
     }

   if (!mode)
     {
        ERR("Invalid access mode for map (none)");
        return NULL;
     }

   if (mode & EFL_GFX_BUFFER_ACCESS_MODE_WRITE)
     {
        ERR("%s does not support write access for map", efl_class_name_get(MY_CLASS));
        return NULL;
     }

   W = pd->glim->w;
   H = pd->glim->h;
   if (!w) w = W - x;
   if (!h) h = H - y;
   if ((x + w > W) || (y + h > H)) return NULL;

   im = eng_image_data_get(ENC, pd->glim, EINA_FALSE, &data, &err, &tofree);
   if (!im) return NULL;

   map = calloc(1, sizeof(*map));
   map->mode = mode;
   map->cspace = cspace;
   map->x = x;
   map->y = y;
   map->w = w;
   map->h = h;
   map->image_data = data;
   map->im = im;
   map->free_image = tofree;

   len = W * H;
   if (cspace == EFL_GFX_COLORSPACE_GRY8)
     {
        uint8_t *data8 = malloc(len);

        if (!data8) goto fail;
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

   map->base_size = len * pxs;
   map->length = (W * h + w - W) * pxs;
   if (stride) *stride = W * pxs;
   if (length) *length = map->length;

   if (!tofree)
     pd->glim = im;

   EINA_INLIST_APPEND(pd->maps, map);
   return map->ptr;

fail:
   free(map);
   return NULL;
}

EOLIAN static void
_evas_ector_gl_image_buffer_ector_buffer_unmap(Eo *obj EINA_UNUSED,
                                               Evas_Ector_GL_Image_Buffer_Data *pd,
                                               void *data, unsigned int length)
{
   Ector_GL_Buffer_Map *map;
   if (!data) return;

   EINA_INLIST_FOREACH(pd->maps, map)
     {
        if ((map->base_data == data) && (map->length == length))
          {
             EINA_INLIST_REMOVE(pd->maps, map);
             if (map->free_image)
               eng_image_free(ENC, map->im);
             else
               map->im = eng_image_data_put(ENC, map->im, map->image_data);
             if (map->allocated)
               free(map->base_data);
             free(map);
             return;
          }
     }

   ERR("Tried to unmap a non-mapped region: %p +%u", data, length);
}

EOLIAN static Efl_Object *
_evas_ector_gl_image_buffer_efl_object_finalize(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   if (!pd->glim)
     {
        ERR("Buffer was not initialized properly!");
        return NULL;
     }
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_gl_image_buffer_efl_object_destructor(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   evas_gl_common_image_free(pd->glim);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "evas_ector_gl_image_buffer.eo.c"
