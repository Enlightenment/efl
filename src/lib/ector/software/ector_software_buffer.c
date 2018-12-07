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
   unsigned int size, stride; // in bytes
   unsigned int x, y, w, h;
   Efl_Gfx_Colorspace cspace;
   Eina_Bool allocated;
   Ector_Buffer_Access_Flag mode;
} Ector_Software_Buffer_Map;

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
     fail("Can not call pixels_clear when the buffer is mapped.");

   efl_event_callback_call(obj, ECTOR_BUFFER_EVENT_DETACHED, pd->pixels.u8);
   if (!pd->nofree)
     {
        free(pd->pixels.u8);
     }
   pd->pixels.u8 = NULL;
   pd->nofree = EINA_FALSE;

   return;

on_fail:
   return;
}

EOLIAN static Eina_Bool
_ector_software_buffer_base_ector_buffer_pixels_set(Eo *obj, Ector_Software_Buffer_Base_Data *pd,
                                                    void *pixels, int width, int height, int stride,
                                                    Efl_Gfx_Colorspace cspace, Eina_Bool writable)
{
   unsigned pxs;

   //if (pd->generic->immutable)
//     fail("This buffer is immutable.");

   if (pd->internal.maps)
     fail("Can not call pixels_set when the buffer is mapped.");

   if (cspace == EFL_GFX_COLORSPACE_ARGB8888)
     pxs = 4;
   else if (cspace == EFL_GFX_COLORSPACE_GRY8)
     pxs = 1;
   else
     fail("Unsupported colorspace: %u", cspace);

   if (((unsigned long long)(uintptr_t)pixels) & (pxs - 1))
     fail ("Pixel data is not aligned to %u bytes!", pxs);

   if (stride == 0)
     stride = width * pxs;
   else if (stride < (int)(width * pxs))
     fail ("Stride is less than minimum stride: provided %u bytes, minimum %u bytes!", stride, (width * pxs));

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
        pd->pixels.u8 = calloc(stride * height, 1);
        pd->nofree = EINA_FALSE;
        pd->writable = EINA_TRUE;
     }
   pd->generic->w = width;
   pd->generic->h = height;
   pd->generic->cspace = cspace;
   pd->stride = stride;
   pd->pixel_size = pxs;
   return EINA_TRUE;

on_fail:
   return EINA_FALSE;
}

EOLIAN static void *
_ector_software_buffer_base_ector_buffer_map(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                             unsigned int *length, Ector_Buffer_Access_Flag mode,
                                             unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                             Efl_Gfx_Colorspace cspace EINA_UNUSED, unsigned int *stride)
{
   Ector_Software_Buffer_Map *map = NULL;
   Eina_Bool need_cow = EINA_FALSE;
   unsigned int off, k, dst_stride, pxs, pxs_dest;

   if (!w) w = pd->generic->w;
   if (!h) h = pd->generic->h;

   if (!pd->pixels.u8 || !pd->stride)
     fail("Buffer has no pixel data yet");
   if (((x + w) > pd->generic->w) || (y + h > pd->generic->h))
     fail("Invalid region requested: wanted %u,%u %ux%u but image is %ux%u",
          x, y, w, h, pd->generic->w, pd->generic->h);
   if ((mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE) && !pd->writable)
     fail("Can not map a read-only buffer for writing");

   pxs = (pd->generic->cspace == EFL_GFX_COLORSPACE_ARGB8888) ? 4 : 1;
   if (cspace == EFL_GFX_COLORSPACE_ARGB8888)
     pxs_dest = 4;
   else if (cspace == EFL_GFX_COLORSPACE_GRY8)
     pxs_dest = 1;
   else
     fail("Unsupported colorspace: %u", cspace);

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

   off = (pxs * x) + (pd->stride * y);
   dst_stride = w * pxs_dest;

   map->mode = mode;
   map->cspace = cspace;
   map->stride = dst_stride;
   map->x = x;
   map->y = y;
   map->w = w;
   map->h = h;

   if (cspace != pd->generic->cspace)
     {
        // convert on the fly
        map->size = w * h * pxs_dest;
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
        map->size = w * h * pxs_dest;
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
                       unsigned k;

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
                            for (k = 0; k < map->h; k++)
                              {
                                 memcpy(pd->pixels.u8 + map->x + (k + map->y) * pd->stride,
                                        map->ptr + k * map->stride, map->stride);
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

EOLIAN static Ector_Buffer_Flag
_ector_software_buffer_base_ector_buffer_flags_get(const Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE |
         ECTOR_BUFFER_FLAG_DRAWABLE |
         ECTOR_BUFFER_FLAG_CPU_READABLE_FAST |
         (pd->writable ? (ECTOR_BUFFER_FLAG_CPU_WRITABLE |
                          ECTOR_BUFFER_FLAG_RENDERABLE |
                          ECTOR_BUFFER_FLAG_CPU_WRITABLE_FAST)
                       : 0);
}

EOLIAN static Efl_Object *
_ector_software_buffer_efl_object_constructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd = efl_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   pd->generic = efl_data_ref(obj, ECTOR_BUFFER_MIXIN);
   pd->generic->eo = obj;
   return obj;
}

EOLIAN static void
_ector_software_buffer_efl_object_destructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd = efl_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   _ector_software_buffer_base_pixels_clear(obj, pd);
   efl_data_unref(obj, pd->generic);
   efl_destructor(efl_super(obj, MY_CLASS));
   if (pd->internal.maps)
     {
        ERR("Pixel data is still mapped during destroy! Check your code!");
     }
}

#include "ector_software_buffer.eo.c"
#include "ector_software_buffer_base.eo.c"
