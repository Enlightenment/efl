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
   void *ptr;
   unsigned int len;
   Eina_Bool allocated;
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

   eo_do(obj, eo_event_callback_call(ECTOR_GENERIC_BUFFER_EVENT_DETACHED, pd->pixels.u8));
   if (!pd->nofree)
     {
        free(pd->pixels.u8);
     }
   pd->pixels.u8 = NULL;
   pd->nofree = EINA_FALSE;
}

EOLIAN static Eina_Bool
_ector_software_buffer_base_ector_generic_buffer_pixels_set(Eo *obj, Ector_Software_Buffer_Base_Data *pd,
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
_ector_software_buffer_base_ector_generic_buffer_map(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                     unsigned int *length, Ector_Buffer_Access_Flag mode,
                                                     unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                                     Efl_Gfx_Colorspace cspace EINA_UNUSED, unsigned int *stride)
{
   Ector_Software_Buffer_Map *map;
   int off;

   if (!pd->pixels.u8 || !pd->stride)
     fail("Buffer has no pixel data yet");
   if (cspace != pd->generic->cspace)
     fail("Invalid colorspace");
   if (!w || !h || ((x + w) > pd->generic->w) || (y + h > pd->generic->h))
     fail("Invalid region requested: wanted %u,%u %ux%u but image is %ux%u",
          x, y, w, h, pd->generic->w, pd->generic->h);
   if ((mode & ECTOR_BUFFER_ACCESS_FLAG_WRITE) && !pd->writable)
     fail("can not map a read-only buffer for writing");

   off = _min_stride_calc(x + pd->generic->l, pd->generic->cspace) + (pd->stride * (y + pd->generic->t));

   map = calloc(1, sizeof(*map));
   map->len = (pd->stride * h) - off;
   map->ptr = pd->pixels.u8 + off;
   pd->internal.maps = eina_inlist_append(pd->internal.maps, EINA_INLIST_GET(map));

   if (length) *length = map->len;
   if (stride) *stride = pd->stride;
   return map->ptr;

on_fail:
   if (length) *length = 0;
   if (stride) *stride = 0;
   return NULL;
}

EOLIAN static void
_ector_software_buffer_base_ector_generic_buffer_unmap(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                       void *data, unsigned int length)
{
   Ector_Software_Buffer_Map *map;
   if (!data) return;

   EINA_INLIST_FOREACH(pd->internal.maps, map)
     {
        if ((map->ptr == data) && (map->len == length))
          {
             pd->internal.maps = eina_inlist_remove(pd->internal.maps, EINA_INLIST_GET(map));
             free(map);
             return;
          }
     }

   CRI("Tried to unmap a non-mapped region!");
}

EOLIAN static uint8_t *
_ector_software_buffer_base_ector_generic_buffer_span_get(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                          int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace,
                                                          unsigned int *length)
{
   uint8_t *src;
   int len, px;

   if (!pd->pixels.u8)
     fail("No pixel data");
   if ((x < -pd->generic->l) || (y < -pd->generic->t) ||
       ((unsigned) x > pd->generic->w) || ((unsigned) y > pd->generic->h))
     fail("Out of bounds");
   if (((unsigned) x + w) > (pd->generic->w + pd->generic->l + pd->generic->r))
     fail("Requested span too large");

   px = _min_stride_calc(1, pd->generic->cspace);
   len = _min_stride_calc(w, cspace);
   if (length) *length = len;

   src = pd->pixels.u8 + ((pd->generic->t + y) * pd->stride) + (px * (pd->generic->l + x));

   if (cspace == pd->generic->cspace)
     {
        pd->span_free = EINA_FALSE;
        return src;
     }
   else if ((cspace == EFL_GFX_COLORSPACE_ARGB8888) &&
            (pd->generic->cspace == EFL_GFX_COLORSPACE_GRY8))
     {
        uint32_t *buf = malloc(len);
        _pixels_gry8_to_argb_convert(buf, src, w);
        pd->span_free = EINA_TRUE;
        return (uint8_t *) buf;
     }
   else if ((cspace == EFL_GFX_COLORSPACE_GRY8) &&
            (pd->generic->cspace == EFL_GFX_COLORSPACE_ARGB8888))
     {
        uint8_t *buf = malloc(len);
        _pixels_argb_to_gry8_convert(buf, (uint32_t *) src, w);
        pd->span_free = EINA_TRUE;
        return buf;
     }
   else
     fail("Unsupported colorspace %u", cspace);

on_fail:
   if (length) *length = 0;
   return NULL;
}

EOLIAN static void
_ector_software_buffer_base_ector_generic_buffer_span_free(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd,
                                                           uint8_t *data)
{
   if (pd->span_free) free(data);
   pd->span_free = EINA_FALSE;
}

EOLIAN static Ector_Buffer_Flag
_ector_software_buffer_base_ector_generic_buffer_flags_get(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Base_Data *pd)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE |
         ECTOR_BUFFER_FLAG_CPU_READABLE_FAST |
         ECTOR_BUFFER_FLAG_RENDERABLE |
         (pd->writable ? (ECTOR_BUFFER_FLAG_CPU_WRITABLE |
                          ECTOR_BUFFER_FLAG_CPU_WRITABLE_FAST)
                       : 0);
}

EOLIAN static Eo_Base *
_ector_software_buffer_eo_base_constructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd;
   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   pd = eo_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   pd->generic = eo_data_ref(obj, ECTOR_GENERIC_BUFFER_MIXIN);
   pd->generic->eo = obj;
   return obj;
}

EOLIAN static void
_ector_software_buffer_eo_base_destructor(Eo *obj, void *data EINA_UNUSED)
{
   Ector_Software_Buffer_Base_Data *pd = eo_data_scope_get(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   _ector_software_buffer_base_pixels_clear(obj, pd);
   eo_data_unref(obj, pd->generic);
   eo_do_super(obj, MY_CLASS, eo_destructor());
   if (pd->internal.maps)
     {
        ERR("Pixel data is still mapped during destroy! Check your code!");
     }
}

#include "ector_software_buffer.eo.c"
#include "ector_software_buffer_base.eo.c"
