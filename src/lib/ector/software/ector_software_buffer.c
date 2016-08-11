#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ector_Software.h"
#include "ector_private.h"
#include "ector_software_private.h"
#include "ector_buffer.h"
#include "draw.h"

#define MY_CLASS ECTOR_SOFTWARE_BUFFER_CLASS

#define fail(fmt, ...) do { ERR(fmt, ##__VA_ARGS__); goto on_fail; } while (0)

typedef struct _Ector_Software_Buffer_Map
{
   EINA_INLIST;
   uint8_t *ptr;
   unsigned int size; // in bytes
   unsigned int x, y, w, h;
   Efl_Gfx_Colorspace cspace;
   Eina_Bool allocated;
   Ector_Buffer_Access_Flag mode;
} Ector_Software_Buffer_Map;

static inline int
_min_stride_calc(int width, Efl_Gfx_Colorspace cspace)
{
   switch (cspace)
     {
      case EFL_GFX_COLORSPACE_ARGB8888: return width * 4;
      case EFL_GFX_COLORSPACE_GRY8: return width;
      default: return 0;
     }
}

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
        uint8_t s = *src++;
        *dst++ = DRAW_ARGB_JOIN(s, s, s, s);
     }
}

EOLIAN static void
_ector_software_buffer_base_pixels_clear(Eo *obj, Ector_Software_Buffer_Base_Data *pd)
{
   if (!pd->pixels.u8)
     return;

   if (pd->internal.maps)
     {
        CRI("Can not call pixels_clear when the buffer is mapped.");
        return;
     }

   efl_event_callback_call(obj, ECTOR_BUFFER_EVENT_DETACHED, pd->pixels.u8);
   if (!pd->nofree)
     {
        free(pd->pixels.u8);
     }
   pd->pixels.u8 = NULL;
   pd->nofree = EINA_FALSE;
}

EOLIAN static Eina_Bool
_ector_software_buffer_base_ector_buffer_pixels_set(Eo *obj, Ector_Software_Buffer_Base_Data *pd,
                                                            void *pixels, int width, int height, int stride,
                                                            Efl_Gfx_Colorspace cspace, Eina_Bool writable,
                                                            unsigned char l, unsigned char r,
                                                            unsigned char t, unsigned char b)
{
   unsigned px;

   if (pd->generic->immutable)
     {
        ERR("This buffer is immutable.");
        return EINA_FALSE;
     }

   if (pd->internal.maps)
     {
        ERR("Can not call pixels_set when the buffer is mapped.");
        return EINA_FALSE;
     }

   // safety check
   px = _min_stride_calc(1, cspace);
   if (px && ((unsigned long long)(uintptr_t)pixels) & (px - 1))
     ERR("Pixel data is not aligned to %u bytes!", px);

   if ((cspace != EFL_GFX_COLORSPACE_ARGB8888) &&
       (cspace != EFL_GFX_COLORSPACE_GRY8))
     {
        ERR("Unsupported colorspace: %u", cspace);
        return EINA_FALSE;
     }

   if (!stride)
     stride = _min_stride_calc(width + l + r, cspace);
   else if (stride < _min_stride_calc(width + l + r, cspace))
     {
        ERR("Invalid stride %u for width %u (+%u+%u) cspace %u. pixels_set failed.",
            stride, width, l, r, cspace);
        _ector_software_buffer_base_pixels_clear(obj, pd);
        return EINA_FALSE;
     }

   if ((px > 1) && (stride & (px - 1)))
     ERR("Stride (%d) is not aligned to the pixel size (%d)", stride, px);

   if (pd->pixels.u8 && (pd->pixels.u8 != pixels))
     _ector_software_buffer_base_pixels_clear(obj, pd);

   if (pixels)
     {
        pd->pixels.u8 = pixels;
        pd->nofree = EINA_TRUE;
        pd->writable = !!writable;
     }
   else
     {
        pd->pixels.u8 = calloc(stride * (height + t + b), 1);
        pd->nofree = EINA_FALSE;
        pd->writable = EINA_TRUE;
     }
   pd->generic->w = width;
   pd->generic->h = height;
   pd->generic->l = l;
   pd->generic->r = r;
   pd->generic->t = t;
   pd->generic->b = b;
   pd->generic->cspace = cspace;
   pd->stride = stride;
   pd->pixel_size = px;
   return EINA_TRUE;
}

EOLIAN static void *
_ector_software_buffer_base_ector_buffer_map(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                     unsigned int *length, Ector_Buffer_Access_Flag mode,
                                                     unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                                     Efl_Gfx_Colorspace cspace EINA_UNUSED, unsigned int *stride)
{
   Ector_Software_Buffer_Map *map = NULL;
   Eina_Bool need_cow = EINA_FALSE;
   unsigned int off, k, dst_stride;

   if (!w) w = pd->generic->w;
   if (!h) h = pd->generic->h;

   if (!pd->pixels.u8 || !pd->stride)
     fail("Buffer has no pixel data yet");
   if (((x + w) > pd->generic->w) || (y + h > pd->generic->h))
     fail("Invalid region requested: wanted %u,%u %ux%u but image is %ux%u",
          x, y, w, h, pd->generic->w, pd->generic->h);
   if ((mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE) && !pd->writable)
     fail("Can not map a read-only buffer for writing");

   if ((mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE) &&
       (mode & ECTOR_BUFFER_ACCESS_FLAG_COW))
     {
        EINA_INLIST_FOREACH(pd->internal.maps, map)
          if (map->mode == ECTOR_BUFFER_ACCESS_FLAG_READ)
            {
               need_cow = EINA_TRUE;
               break;
            }
     }

   map = calloc(1, sizeof(*map));
   if (!map) fail("Out of memory");

   map->mode = mode;
   map->cspace = cspace;
   map->x = x;
   map->y = y;
   map->w = w;
   map->h = h;

   off = _min_stride_calc(x + pd->generic->l, pd->generic->cspace) + (pd->stride * (y + pd->generic->t));
   dst_stride = _min_stride_calc(w, cspace);

   if (cspace != pd->generic->cspace)
     {
        // convert on the fly
        map->size = _min_stride_calc(w, cspace) * h;
        map->allocated = EINA_TRUE;
        map->ptr = malloc(map->size);
        if (!map->ptr) fail("Out of memory");

        if (cspace == EFL_GFX_COLORSPACE_ARGB8888)
          {
             for (k = 0; k < h; k++)
               _pixels_gry8_to_argb_convert((uint32_t *) map->ptr + (k * w), pd->pixels.u8 + off + (k * pd->stride), w);
          }
        else
          {
             for (k = 0; k < h; k++)
               _pixels_argb_to_gry8_convert(map->ptr + (k * w), (uint32_t *) (pd->pixels.u8 + off + (k * pd->stride)), w);
          }
     }
   else if (need_cow)
     {
        // copy-on-write access
        map->size = _min_stride_calc(w, cspace) * h;
        map->allocated = EINA_TRUE;
        map->ptr = malloc(map->size);
        if (!map->ptr) fail("Out of memory");
        for (k = 0; k < h; k++)
          memcpy(map->ptr + k * dst_stride, pd->pixels.u8 + x + (k + y) * pd->stride, dst_stride);
     }
   else
     {
        // direct access, zero-copy
        map->size = (pd->stride * h) - off;
        map->ptr = pd->pixels.u8 + off;
        dst_stride = pd->stride;
     }

   pd->internal.maps = eina_inlist_prepend(pd->internal.maps, EINA_INLIST_GET(map));
   if (length) *length = map->size;
   if (stride) *stride = dst_stride;
   return map->ptr;

on_fail:
   free(map);
   if (length) *length = 0;
   if (stride) *stride = 0;
   return NULL;
}

EOLIAN static void
_ector_software_buffer_base_ector_buffer_unmap(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                       void *data, unsigned int length)
{
   Ector_Software_Buffer_Map *map;
   if (!data) return;

   EINA_INLIST_FOREACH(pd->internal.maps, map)
     {
        if ((map->ptr == data) && ((map->size == length) || (length == (unsigned int) -1)))
          {
             pd->internal.maps = eina_inlist_remove(pd->internal.maps, EINA_INLIST_GET(map));
             if (map->allocated)
               {
                  if (map->mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE)
                    {
                       unsigned k, dst_stride;

                       if (map->cspace != pd->generic->cspace)
                         {
                            if (pd->generic->cspace == EFL_GFX_COLORSPACE_ARGB8888)
                              {
                                 for (k = 0; k < map->h; k++)
                                   _pixels_gry8_to_argb_convert((uint32_t *) (pd->pixels.u8 + (k + map->y) * pd->stride),
                                                                map->ptr + (k * map->w),
                                                                map->w);
                              }
                            else
                              {
                                 for (k = 0; k < map->h; k++)
                                   _pixels_argb_to_gry8_convert(pd->pixels.u8 + (k + map->y) * pd->stride,
                                                                (uint32_t *) map->ptr + (k * map->w),
                                                                map->w);
                              }
                         }
                       else
                         {
                            dst_stride = _min_stride_calc(map->w, map->cspace);
                            for (k = 0; k < map->h; k++)
                              {
                                 memcpy(pd->pixels.u8 + map->x + (k + map->y) * pd->stride,
                                        map->ptr + k * dst_stride,
                                        dst_stride);
                              }
                         }
                    }
                  free(map->ptr);
               }
             free(map);
             return;
          }
     }

   CRI("Tried to unmap a non-mapped region!");
}

EOLIAN static uint8_t *
_ector_software_buffer_base_ector_buffer_span_get(Eo *obj, Ector_Software_Buffer_Base_Data *pd,
                                                          int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace,
                                                          unsigned int *length)
{
   // ector_buffer_map
   return _ector_software_buffer_base_ector_buffer_map
         (obj, pd, length, ECTOR_BUFFER_ACCESS_FLAG_READ, x, y, w, 1, cspace, NULL);
}

EOLIAN static void
_ector_software_buffer_base_ector_buffer_span_free(Eo *obj, Ector_Software_Buffer_Base_Data *pd,
                                                           uint8_t *data)
{
   // ector_buffer_unmap
   return _ector_software_buffer_base_ector_buffer_unmap
         (obj, pd, data, (unsigned int) -1);
}

EOLIAN static Ector_Buffer_Flag
_ector_software_buffer_base_ector_buffer_flags_get(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE |
         ECTOR_BUFFER_FLAG_DRAWABLE |
         ECTOR_BUFFER_FLAG_CPU_READABLE_FAST |
         ECTOR_BUFFER_FLAG_RENDERABLE |
         (pd->writable ? (ECTOR_BUFFER_FLAG_CPU_WRITABLE |
                          ECTOR_BUFFER_FLAG_CPU_WRITABLE_FAST)
                       : 0);
}

EOLIAN static Efl_Object *
_ector_software_buffer_efl_object_constructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd;
   obj = efl_constructor(eo_super(obj, MY_CLASS));
   pd = eo_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   pd->generic = eo_data_ref(obj, ECTOR_BUFFER_MIXIN);
   pd->generic->eo = obj;
   return obj;
}

EOLIAN static void
_ector_software_buffer_efl_object_destructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd = eo_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   _ector_software_buffer_base_pixels_clear(obj, pd);
   eo_data_unref(obj, pd->generic);
   efl_destructor(eo_super(obj, MY_CLASS));
   if (pd->internal.maps)
     {
        ERR("Pixel data is still mapped during destroy! Check your code!");
     }
}

#include "ector_software_buffer.eo.c"
#include "ector_software_buffer_base.eo.c"
