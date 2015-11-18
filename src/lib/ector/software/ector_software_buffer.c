#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include "Ector_Software.h"
#include "ector_private.h"
#include "ector_software_private.h"
#include "ector_generic_buffer.eo.h"

#define fail(fmt, ...) do { ERR(fmt, ##__VA_ARGS__); goto on_fail; } while (0)

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
        *dst++ = ARGB_JOIN(s, s, s, s);
     }
}

EOLIAN static void
_ector_software_buffer_pixels_clear(Eo *obj, Ector_Software_Buffer_Data *pd)
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
_ector_software_buffer_ector_generic_buffer_pixels_set(Eo *obj, Ector_Software_Buffer_Data *pd,
                                                       void *pixels, int width, int height, int stride,
                                                       Efl_Gfx_Colorspace cspace, Eina_Bool writable,
                                                       unsigned char l, unsigned char r,
                                                       unsigned char t, unsigned char b)
{
   // safety check
   unsigned px = _min_stride_calc(1, cspace);
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
        _ector_software_buffer_pixels_clear(obj, pd);
        return EINA_FALSE;
     }

   if (pd->pixels.u8 && (pd->pixels.u8 != pixels))
     _ector_software_buffer_pixels_clear(obj, pd);

   if (pixels)
     {
        pd->pixels.u8 = pixels;
        pd->nofree = EINA_TRUE;
        pd->writable = !!writable;
     }
   else
     {
        pd->pixels.u8 = malloc(stride * (height + t + b));
        pd->nofree = EINA_FALSE;
        pd->writable = EINA_TRUE;
     }
   pd->generic.w = width;
   pd->generic.h = height;
   pd->generic.l = l;
   pd->generic.r = r;
   pd->generic.t = t;
   pd->generic.b = b;
   pd->generic.cspace = cspace;
   pd->stride = stride;
   return EINA_TRUE;
}

EOLIAN static uint8_t *
_ector_software_buffer_ector_generic_buffer_map(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Data *pd,
                                                int *offset, unsigned int *length,
                                                Ector_Buffer_Access_Flag mode EINA_UNUSED,
                                                unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                                Efl_Gfx_Colorspace cspace EINA_UNUSED, unsigned int *stride)
{
   int off;

   if (!pd->pixels.u8 || !pd->stride)
     fail("Buffer has no pixel data yet");
   if (cspace != pd->generic.cspace)
     fail("Invalid colorspace");
   if (!w || !h || ((x + w) > pd->generic.w) || (y + h > pd->generic.h))
     fail("Invalid region requested: wanted %u,%u %ux%u but image is %ux%u",
          x, y, w, h, pd->generic.w, pd->generic.h);

   pd->map_count++;
   off = _min_stride_calc(x + pd->generic.l, pd->generic.cspace) + (pd->stride * (y + pd->generic.t));
   if (offset) *offset = off;
   if (length) *length = (pd->stride * pd->generic.h) - off;
   if (stride) *stride = pd->stride;
   return pd->pixels.u8;

on_fail:
   if (offset) *offset = 0;
   if (length) *length = 0;
   if (stride) *stride = 0;
   return NULL;
}

EOLIAN static void
_ector_software_buffer_ector_generic_buffer_unmap(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Data *pd, void *data, int offset EINA_UNUSED, unsigned int length EINA_UNUSED)
{
   if (!data) return;
   if (data != pd->pixels.u8)
     {
        CRI("Trying to unmap a non-mapped region!");
        return;
     }
   if (pd->map_count == 0)
     {
        CRI("Unmapped too many times! Check your code!");
        return;
     }
   pd->map_count--;
}

EOLIAN uint8_t *
_ector_software_buffer_ector_generic_buffer_span_get(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Data *pd,
                                                     int x, int y, unsigned int w, Efl_Gfx_Colorspace cspace,
                                                     unsigned int *length)
{
   uint8_t *src;
   int len, px;

   if (!pd->pixels.u8)
     fail("No pixel data");
   if ((x < -pd->generic.l) || (y < -pd->generic.t) ||
       ((unsigned) x > pd->generic.w) || ((unsigned) y > pd->generic.h))
     fail("Out of bounds");
   if (((unsigned) x + w) > (pd->generic.w + pd->generic.l + pd->generic.r))
     fail("Requested span too large");

   px = _min_stride_calc(1, pd->generic.cspace);
   len = _min_stride_calc(w, cspace);
   if (length) *length = len;

   src = pd->pixels.u8 + ((pd->generic.t + y) * pd->stride) + (px * (pd->generic.l + x));

   if (cspace == pd->generic.cspace)
     {
        pd->span_free = EINA_FALSE;
        return src;
     }
   else if ((cspace == EFL_GFX_COLORSPACE_ARGB8888) &&
            (pd->generic.cspace == EFL_GFX_COLORSPACE_GRY8))
     {
        uint32_t *buf = malloc(len);
        _pixels_gry8_to_argb_convert(buf, src, w);
        pd->span_free = EINA_TRUE;
        return (uint8_t *) buf;
     }
   else if ((cspace == EFL_GFX_COLORSPACE_GRY8) &&
            (pd->generic.cspace == EFL_GFX_COLORSPACE_ARGB8888))
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

EOLIAN void
_ector_software_buffer_ector_generic_buffer_span_free(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Data *pd,
                                                      uint8_t *data)
{
   if (pd->span_free) free(data);
   pd->span_free = EINA_FALSE;
}

EOLIAN static Ector_Buffer_Flag
_ector_software_buffer_ector_generic_buffer_flags_get(Eo *obj EINA_UNUSED, Ector_Software_Buffer_Data *pd)
{
   return ECTOR_BUFFER_FLAG_CPU_READABLE |
         ECTOR_BUFFER_FLAG_CPU_READABLE_FAST |
         ECTOR_BUFFER_FLAG_RENDERABLE |
         (pd->writable ? (ECTOR_BUFFER_FLAG_CPU_WRITABLE |
                          ECTOR_BUFFER_FLAG_CPU_WRITABLE_FAST)
                       : 0);
}

EOLIAN static void
_ector_software_buffer_eo_base_destructor(Eo *obj, Ector_Software_Buffer_Data *pd)
{
   _ector_software_buffer_pixels_clear(obj, pd);
   eo_do_super(obj, ECTOR_SOFTWARE_BUFFER_CLASS, eo_destructor());
   if (pd->map_count)
     {
        ERR("Pixel data is still mapped during destroy! Check your code!");
     }
}

#include "ector_software_buffer.eo.c"
