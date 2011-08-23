#include "evas_common.h"
#include "evas_convert_colorspace.h"

#define CONVERT_RGB_565_TO_RGB_888(s) \
	(((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
	 ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
	 ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#define CONVERT_A5P_TO_A8(s) \
	((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7))

#define CONVERT_ARGB_8888_TO_A_8(s)	((s) >> 24)


static inline void *
evas_common_convert_argb8888_to_rgb565_a5p(void *data __UNUSED__, int w __UNUSED__, int h __UNUSED__, int stride __UNUSED__, Eina_Bool has_alpha __UNUSED__)
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



EAPI void *
evas_common_convert_argb8888_to(void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace)
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

EAPI void *
evas_common_convert_yuv_422_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           fprintf(stderr, "to argb888\n");

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

EAPI void *
evas_common_convert_yuv_422P_601_to(void *data, int w, int h, Evas_Colorspace cspace)
{
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        {
           void *dst;

           dst = malloc(sizeof (unsigned int) * w * h);
           if (!dst) return NULL;

           evas_common_convert_yuv_420p_601_rgba(data, dst, w, h);
           break;
        }
      default:
         break;
     }
   return NULL;
}


/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
