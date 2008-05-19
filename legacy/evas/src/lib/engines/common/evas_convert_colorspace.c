#include "evas_common.h"

#define CONVERT_RGB_565_TO_RGB_888(s) \
	(((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
	 ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
	 ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#define CONVERT_A5P_TO_A8(s) \
	((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7))

static inline void *
evas_common_convert_argb8888_to_rgb565_a5p(void *data, int w, int h, int stride, Evas_Bool has_alpha)
{
}

static inline void *
evas_common_convert_rgb565_a5p_to_argb8888(void *data, int w, int h, int stride, Evas_Bool has_alpha)
{
   DATA16 *src, *end;
   DATA32 *ret, *dst;
   int r, g, b;

   src = data;
   end = src + (stride * h);
   ret = malloc(w * h * sizeof(DATA32));

   dst = ret;
   if (has_alpha)
     {
	DATA8 *alpha;

	alpha = end;
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

EAPI void *
evas_common_convert_argb8888_to(void *data, int w, int h, int stride, Evas_Bool has_alpha, Evas_Colorspace cspace)
{
   switch (cspace)
     {
	case EVAS_COLORSPACE_RGB565_A5P:
	  return evas_common_convert_argb8888_to_rgb565_a5p(data, w, h, stride, has_alpha);
	default:
	  break;
     }
   return NULL;
}

EAPI void *
evas_common_convert_rgb565_a5p_to(void *data, int w, int h, int stride, Evas_Bool has_alpha, Evas_Colorspace cspace)
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
