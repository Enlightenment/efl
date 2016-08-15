#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define ECTOR_GL_BUFFER_BASE_PROTECTED

#include <gl/Ector_GL.h>
#include "gl/ector_gl_private.h"
#include "evas_common_private.h"
#include "../gl_common/evas_gl_common.h"
#include "evas_private.h"
#include "ector_buffer.h"

#include "evas_ector_buffer.eo.h"
#include "evas_ector_gl_buffer.eo.h"
#include "evas_ector_gl_image_buffer.eo.h"

#define MY_CLASS EVAS_ECTOR_GL_IMAGE_BUFFER_CLASS

typedef struct _Ector_GL_Buffer_Map
{
   EINA_INLIST;
   void *ptr;
   unsigned int size; // in bytes
   unsigned int x, y, w, h;
   Efl_Gfx_Colorspace cspace;
   Evas_GL_Image *im;
   Eina_Bool allocated;
   Ector_Buffer_Access_Flag mode;
} Ector_GL_Buffer_Map;

typedef struct
{
   Ector_GL_Buffer_Base_Data *base;
   Evas *evas;
   Evas_GL_Image *image;
   struct {
      Eina_Inlist *maps; // Ector_GL_Buffer_Map
   } internal;
} Evas_Ector_GL_Image_Buffer_Data;

#define ENFN e->engine.func
#define ENDT e->engine.data.output

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
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_set(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd,
                                                               Evas *evas, void *image)
{
   Evas_Public_Data *e = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);
   Evas_GL_Image *im = image;
   int l = 0, r = 0, t = 0, b = 0;

   if (pd->base->generic->immutable)
     {
        CRI("Can't set image after finalize");
        return;
     }

   pd->evas = efl_xref(evas, obj);
   EINA_SAFETY_ON_NULL_RETURN(im);

   if (im->tex && im->tex->pt)
     {
        if (im->im)
          {
             l = im->im->cache_entry.borders.l;
             r = im->im->cache_entry.borders.r;
             t = im->im->cache_entry.borders.t;
             b = im->im->cache_entry.borders.b;
          }
        else
          {
             // always 1 pixel border, except FBO
             if (!im->tex->pt->fb)
               l = r = t = b = 1;
          }

        pd->image = ENFN->image_ref(ENDT, im);
        ector_gl_buffer_base_attach(obj, im->tex->pt->texture, im->tex->pt->fb, (Efl_Gfx_Colorspace) evas_gl_common_gl_format_to_colorspace(im->tex->pt->format), im->tex->w, im->tex->h, im->tex->x, im->tex->y, im->tex->pt->w, im->tex->pt->h, l, r, t, b);
     }
   else
     {
        // FIXME: This might be required to support texture upload here
        ERR("Image has no attached texture! Unsupported!");
        pd->image = NULL;
     }
}

EOLIAN static void
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_get(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_GL_Image_Buffer_Data *pd,
                                                               Evas **evas, void **image)
{
   if (evas) *evas = pd->evas;
   if (image) *image = pd->image;
}

EOLIAN static Ector_Buffer_Flag
_evas_ector_gl_image_buffer_ector_buffer_flags_get(Eo *obj EINA_UNUSED,
                                                           Evas_Ector_GL_Image_Buffer_Data *pd)
{
   Ector_Buffer_Flag flags;

   if (!pd->image) return 0;

   flags = ECTOR_BUFFER_FLAG_CPU_READABLE;
   if (pd->image->tex)
     {
        flags |= ECTOR_BUFFER_FLAG_DRAWABLE;
        if (pd->image->tex->pt->fb)
          flags |= ECTOR_BUFFER_FLAG_RENDERABLE;
     }
   if (pd->image->im)
     flags |= ECTOR_BUFFER_FLAG_CPU_WRITABLE;

   return flags;
}

EOLIAN static void *
_evas_ector_gl_image_buffer_ector_buffer_map(Eo *obj EINA_UNUSED, Evas_Ector_GL_Image_Buffer_Data *pd, unsigned int *length,
                                                     Ector_Buffer_Access_Flag mode,
                                                     unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                                     Efl_Gfx_Colorspace cspace, unsigned int *stride)
{
   Evas_Public_Data *e = efl_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);
   Ector_GL_Buffer_Map *map = NULL;
   Eina_Bool tofree = EINA_FALSE;
   Evas_GL_Image *im;
   uint32_t *data;
   int len, err;

   im = ENFN->image_data_get(ENDT, pd->image,
                             mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE,
                             &data, &err, &tofree);
   if (!im) return NULL;

   map = calloc(1, sizeof(*map));
   map->mode = mode;
   map->cspace = cspace;
   map->x = x;
   map->y = y;
   map->w = w;
   map->h = h;
   map->ptr = data;

   if (tofree)
     map->im = im;
   else
     map->im = NULL;

   len = w * h;
   if (cspace == EFL_GFX_COLORSPACE_GRY8)
     {
        uint8_t *data8 = malloc(len);
        _pixels_argb_to_gry8_convert(data8, data, len);
        map->allocated = EINA_TRUE;
        map->ptr = data8;
        map->size = len;
        if (stride) *stride = w;
     }
   else
     {
        map->allocated = EINA_FALSE;
        map->ptr = data;
        map->size = len * 4;
        if (stride) *stride = w * 4;
     }

   if (length) *length = map->size;

   pd->internal.maps = eina_inlist_prepend(pd->internal.maps, EINA_INLIST_GET(map));
   return map->ptr;
}

EOLIAN static void
_evas_ector_gl_image_buffer_ector_buffer_unmap(Eo *obj EINA_UNUSED, Evas_Ector_GL_Image_Buffer_Data *pd,
                                                       void *data, unsigned int length)
{
   Evas_Public_Data *e = efl_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);
   Ector_GL_Buffer_Map *map;
   if (!data) return;

   EINA_INLIST_FOREACH(pd->internal.maps, map)
     {
        if ((map->ptr == data) && ((map->size == length) || (length == (unsigned int) -1)))
          {
             pd->internal.maps = eina_inlist_remove(pd->internal.maps, EINA_INLIST_GET(map));
             if (map->mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE)
               {
                  CRI("Not implemented yet. Dropping pixel changes.");
               }
             if (map->im)
               ENFN->image_free(ENDT, map->im);
             if (map->allocated)
               free(map->ptr);
             return;
          }
     }

   CRI("Tried to unmap a non-mapped region!");
}

EOLIAN static uint8_t *
_evas_ector_gl_image_buffer_ector_buffer_span_get(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd, int x, int y, unsigned int w,
                                                          Efl_Gfx_Colorspace cspace, unsigned int *length)
{
   // ector_buffer_map
   return _evas_ector_gl_image_buffer_ector_buffer_map
         (obj, pd, length, ECTOR_BUFFER_ACCESS_FLAG_READ, x, y, w, 1, cspace, NULL);
}

EOLIAN static void
_evas_ector_gl_image_buffer_ector_buffer_span_free(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd, uint8_t *data)
{
   // ector_buffer_unmap
   return _evas_ector_gl_image_buffer_ector_buffer_unmap
         (obj, pd, data, (unsigned int) -1);
}


EOLIAN static Efl_Object *
_evas_ector_gl_image_buffer_efl_object_constructor(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->base = efl_data_ref(obj, ECTOR_GL_BUFFER_BASE_MIXIN);
   pd->base->generic = efl_data_ref(obj, ECTOR_BUFFER_MIXIN);
   pd->base->generic->eo = obj;
   return obj;
}

EOLIAN static Efl_Object *
_evas_ector_gl_image_buffer_efl_object_finalize(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->base, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->image, NULL);
   pd->base->generic->immutable = EINA_TRUE;
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_gl_image_buffer_efl_object_destructor(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   Evas_Public_Data *e = efl_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);

   efl_data_unref(obj, pd->base->generic);
   efl_data_unref(obj, pd->base);
   ENFN->image_free(ENDT, pd->image);
   efl_xunref(pd->evas, obj);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "evas_ector_gl_image_buffer.eo.c"
