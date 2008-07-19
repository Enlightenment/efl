/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <assert.h>

#include "evas_common.h"
#include "evas_private.h"

int
evas_common_rgba_image_from_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, int cspace)
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
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
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
   return 0;
}

int
evas_common_rgba_image_from_copied_data(Image_Entry* ie_dst, int w, int h, DATA32 *image_data, int alpha, int cspace)
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
     case EVAS_COLORSPACE_YCBCR422P601_PL:
     case EVAS_COLORSPACE_YCBCR422P709_PL:
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
   return 0;
}

int
evas_common_rgba_image_size_set(Image_Entry *ie_dst, const Image_Entry *ie_im, int w, int h)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;
   RGBA_Image   *im = (RGBA_Image *) ie_im;

   if ((im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   dst->flags = im->flags;
   dst->cs.no_free = 0;
   if ((im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     dst->cs.data = calloc(1, dst->cache_entry.h * sizeof(unsigned char *) * 2);
   evas_common_image_colorspace_dirty(dst);

   return 0;
}

int
evas_common_rgba_image_colorspace_set(Image_Entry* ie_dst, int cspace)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;

   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (dst->cs.data)
	  {
	     if (!dst->cs.no_free) free(dst->cs.data);
	     dst->cs.data = NULL;
	     dst->cs.no_free = 0;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (dst->image.no_free)
	  {
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

   return 0;
}
