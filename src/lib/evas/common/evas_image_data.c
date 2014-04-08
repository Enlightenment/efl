#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_image.h"

int
evas_common_rgba_image_from_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;

   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
        dst->cache_entry.w = w;
        dst->cache_entry.h = h;
        dst->image.data = image_data;
        dst->image.no_free = 1;
        dst->cache_entry.flags.alpha = alpha ? 1 : 0;
        break;
      case EVAS_COLORSPACE_AGRY88:
      case EVAS_COLORSPACE_GRY8:
        dst->cache_entry.w = w;
        dst->cache_entry.h = h;
        dst->image.data8 = (DATA8 *) image_data;
        dst->image.no_free = 1;
        dst->cache_entry.flags.alpha = 1;
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
	w &= ~0x1;
	dst->cache_entry.w = w;
	dst->cache_entry.h = h;
	dst->cs.data = image_data;
	dst->cs.no_free = 1;
        break;
      default:
	abort();
	break;
     }
   dst->cache_entry.space = cspace;
   evas_common_image_colorspace_dirty(dst);
   _evas_common_rgba_image_post_surface(ie_dst);
   return 0;
}

int
evas_common_rgba_image_from_copied_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;

   /* FIXME: Is dst->image.data valid. */
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         dst->cache_entry.flags.alpha = alpha ? 1 : 0;
         if (image_data)
           memcpy(dst->image.data, image_data, w * h * sizeof(DATA32));
         break;
      case EVAS_COLORSPACE_AGRY88:
         dst->cache_entry.flags.alpha = 1;
         if (image_data)
           memcpy(dst->image.data8, image_data, w * h * sizeof(DATA16));
         break;
      case EVAS_COLORSPACE_GRY8:
        dst->cache_entry.flags.alpha = 1;
        if (image_data)
          memcpy(dst->image.data8, image_data, w * h * sizeof(DATA8));
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
         dst->cs.data = calloc(1, dst->cache_entry.h * sizeof(unsigned char*) * 2);
         if (image_data && (dst->cs.data))
           memcpy(dst->cs.data,  image_data, dst->cache_entry.h * sizeof(unsigned char*) * 2);
         break;
      default:
         abort();
         break;
     }

   dst->cache_entry.space = cspace;
   evas_common_image_colorspace_dirty(dst);
   _evas_common_rgba_image_post_surface(ie_dst);
   return 0;
}

int
evas_common_rgba_image_size_set(Image_Entry *ie_dst, const Image_Entry *ie_im, unsigned int w, unsigned int h EINA_UNUSED)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;
   RGBA_Image   *im = (RGBA_Image *) ie_im;

   if ((im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR420TM12601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR420NV12601_PL))
     {
        w &= ~0x1;
        dst->cs.data = calloc(1, dst->cache_entry.h * sizeof(unsigned char *) * 2);
     }

   dst->flags = im->flags;
   dst->cs.no_free = 0;
   evas_common_image_colorspace_dirty(dst);

   _evas_common_rgba_image_post_surface(ie_dst);
   return 0;
}

int
evas_common_rgba_image_colorspace_set(Image_Entry* ie_dst, Evas_Colorspace cspace)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;
   Eina_Bool change = (dst->cache_entry.space != cspace);

   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
      case EVAS_COLORSPACE_AGRY88:
      case EVAS_COLORSPACE_GRY8:
	if (dst->cs.data)
	  {
	     if (!dst->cs.no_free) free(dst->cs.data);
	     dst->cs.data = NULL;
	     dst->cs.no_free = 0;
	  }
        if (change && dst->image.data)
          {
             if (!dst->image.no_free) free(dst->image.data);
             dst->image.data = NULL;
             dst->image.no_free = 0;
          }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
	if (dst->image.no_free)
	  {
             ie_dst->allocated.w = 0;
             ie_dst->allocated.h = 0;
             ie_dst->flags.preload_done = 0;
             ie_dst->flags.loaded = 0;
             dst->image.data = NULL;
	     dst->image.no_free = 0;
             /* FIXME: Must allocate image.data surface cleanly. */
	  }
	if (dst->cs.data)
	  {
	     if (!dst->cs.no_free) free(dst->cs.data);
	  }
	dst->cs.data = calloc(1, dst->cache_entry.h * sizeof(unsigned char *) * 2);
	dst->cs.no_free = 0;
        break;
      default:
	abort();
	break;
     }
   dst->cache_entry.space = cspace;
   evas_common_image_colorspace_dirty(dst);

   _evas_common_rgba_image_post_surface(ie_dst);
   return 0;
}
