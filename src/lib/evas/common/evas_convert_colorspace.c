#include "evas_common_private.h"
#include "evas_convert_colorspace.h"

#define CONVERT_RGB_565_TO_RGB_888(s) \
	(((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
	 ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
	 ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#define CONVERT_A5P_TO_A8(s) \
	((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7))

#define CONVERT_ARGB_8888_TO_A_8(s)	((s) >> 24)


static inline void *
evas_common_convert_argb8888_to_rgb565_a5p(void *data EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, int stride EINA_UNUSED, Eina_Bool has_alpha EINA_UNUSED)
{
   return NULL;
}

static inline void *
evas_common_convert_rgb565_a5p_to_argb8888(void *data, int w, int h, int stride, Eina_Bool has_alpha)
{
   DATA16 *src, *end;
   DATA32 *ret, *dst;

   src = data;
   end = src + (stride * h);
   ret = malloc(w * h * sizeof(DATA32));
   if (!ret) return NULL;

   dst = ret;
   if (has_alpha)
     {
	DATA8 *alpha;

	alpha = (DATA8 *)end;
	for (; src < end; src++, alpha++, dst++)
	  *dst = (CONVERT_A5P_TO_A8(*alpha) << 24) |
		  CONVERT_RGB_565_TO_RGB_888(*src);
     }
   else
     {
	for (; src < end; src++, dst++)
	  *dst = CONVERT_RGB_565_TO_RGB_888(*src);
     }
   return ret;
}

static inline void *
evas_common_convert_agry88_to_argb8888(const void *data, int w, int h, int stride, Eina_Bool has_alpha)
{
   const DATA16 *src, *end;
   DATA32 *ret, *dst;

   src = data;
   end = src + ((stride >> 1) * h);
   ret = malloc(w * h * sizeof(DATA32));
   if (!ret) return NULL;
   dst = ret;

   if (has_alpha)
     {
        for (; src < end; src++, dst++)
          {
             int c = (*src) & 0xFF;
             *dst = ARGB_JOIN((*src >> 8), c, c, c);
          }
     }
   else
     {
        for (; src < end; src++, dst++)
          {
             int c = (*src) & 0xFF;
             *dst = ARGB_JOIN(0xFF, c, c, c);
          }
     }

   return ret;
}

void *
evas_common_convert_agry88_to(const void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace)
{
   switch (cspace) {
      case EVAS_COLORSPACE_ARGB8888:
        return evas_common_convert_agry88_to_argb8888(data, w, h, stride, has_alpha);
      default:
        return NULL;
     }
}

static inline void *
evas_common_convert_gry8_to_argb8888(const void *data, int w, int h, int stride, Eina_Bool has_alpha)
{
   const DATA8 *src, *end;
   DATA32 *ret, *dst;

   src = data;
   end = src + (stride * h);
   ret = malloc(w * h * sizeof(DATA32));
   if (!ret) return NULL;
   dst = ret;

   if (has_alpha)
     {
        for (; src < end; src++, dst++)
          {
             int c = (*src) & 0xFF;
             *dst = ARGB_JOIN(c, c, c, c);
          }
     }
   else
     {
        for (; src < end; src++, dst++)
          {
             int c = (*src) & 0xFF;
             *dst = ARGB_JOIN(0xFF, c, c, c);
          }
     }

   return ret;
}

void *
evas_common_convert_gry8_to(const void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace)
{
   switch (cspace) {
      case EVAS_COLORSPACE_ARGB8888:
        return evas_common_convert_gry8_to_argb8888(data, w, h, stride, has_alpha);
      default:
        return NULL;
     }
}

static inline void *
evas_common_convert_argb8888_to_a8(void *data, int w, int h, int stride, Eina_Bool has_alpha)
{
   uint32_t *src, *end;
   uint8_t *ret, *dst;

   src = data;
   end = src + (stride * h);
   ret = malloc(w * h);
   if (!ret) return NULL;

   if (!has_alpha)
     {
        return memset(ret, 0xff, w * h);
     }

   dst = ret;
   for ( ; src < end ; src++, dst++)
      *dst = CONVERT_ARGB_8888_TO_A_8(*src);
   return ret;
}

EVAS_API void *
evas_common_convert_argb8888_to(void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace)
{
   switch (cspace)
     {
	case EVAS_COLORSPACE_RGB565_A5P:
	  return evas_common_convert_argb8888_to_rgb565_a5p(data, w, h, stride, has_alpha);
	case EVAS_COLORSPACE_GRY8:
	  return evas_common_convert_argb8888_to_a8(data, w, h, stride, has_alpha);
	default:
	  break;
     }
   return NULL;
}

EVAS_API void *
evas_common_convert_rgb565_a5p_to(void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace)
{
   switch (cspace)
     {
	case EVAS_COLORSPACE_ARGB8888:
	  return evas_common_convert_rgb565_a5p_to_argb8888(data, w, h, stride, has_alpha);
	default:
	  break;
     }
   return NULL;
}

EVAS_API void *
evas_common_convert_yuv_422_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           dst = malloc(sizeof (unsigned int) * w * h);
           if (!dst) return NULL;

           evas_common_convert_yuv_422_601_rgba(data, dst, w, h);
           return dst;
        }
      default:
         break;
     }
   return NULL;
}

EVAS_API void *
evas_common_convert_yuv_422P_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           dst = malloc(sizeof (unsigned int) * w * h);
           if (!dst) return NULL;

           evas_common_convert_yuv_422p_601_rgba(data, dst, w, h);
           return dst;
        }
      default:
         break;
     }
   return NULL;
}

EVAS_API void *
evas_common_convert_yuv_420_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           dst = malloc(sizeof (unsigned int) * w * h);
           if (!dst) return NULL;

           evas_common_convert_yuv_420_601_rgba(data, dst, w, h);
           return dst;
        }
      default:
         break;
     }
   return NULL;
}

EVAS_API void *
evas_common_convert_yuv_420T_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           dst = malloc(sizeof (unsigned int) * w * h);
           if (!dst) return NULL;

           evas_common_convert_yuv_420_601_rgba(data, dst, w, h);
           return dst;
        }
      default:
         break;
     }
   return NULL;
}


/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
