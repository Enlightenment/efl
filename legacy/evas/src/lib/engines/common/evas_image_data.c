#include "evas_common.h"
#include "evas_private.h"

#include <assert.h>

EAPI int
evas_common_image_from_data(RGBA_Image* dst, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   switch (cspace)
     {
     case EVAS_COLORSPACE_ARGB8888:
	dst->image->w = w;
	dst->image->h = h;
	dst->image->data = image_data;
	dst->image->no_free = 1;
	if (alpha)
	  dst->flags |= RGBA_IMAGE_HAS_ALPHA;
	else
	  dst->flags &= ~RGBA_IMAGE_HAS_ALPHA;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	w &= ~0x1;
	dst->image->w = w;
	dst->image->h = h;
	evas_common_image_surface_alloc(dst->image);
	dst->cs.data = image_data;
	dst->cs.no_free = 1;
	break;
      default:
	abort();
	break;
     }
   dst->cs.space = cspace;
   evas_common_image_colorspace_dirty(dst);
   return 0;
}

EAPI int
evas_common_image_from_copied_data(RGBA_Image* dst, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   switch (cspace)
     {
     case EVAS_COLORSPACE_ARGB8888:
	if (alpha)
	  dst->flags |= RGBA_IMAGE_HAS_ALPHA;
	else
	  dst->flags &= ~RGBA_IMAGE_HAS_ALPHA;
        if (image_data)
          memcpy(dst->image->data, image_data, w * h * sizeof(DATA32));
        break;
     case EVAS_COLORSPACE_YCBCR422P601_PL:
     case EVAS_COLORSPACE_YCBCR422P709_PL:
        dst->cs.data = calloc(1, dst->image->h * sizeof(unsigned char*) * 2);
        if (image_data && (dst->cs.data))
          memcpy(dst->cs.data,  image_data, dst->image->h * sizeof(unsigned char*) * 2);
        break;
     default:
        abort();
        break;
     }

   dst->cs.space = cspace;
   evas_common_image_colorspace_dirty(dst);
   return 0;
}

EAPI int
evas_common_image_size_set(RGBA_Image* dst, const RGBA_Image* im, int w, int h)
{
   if ((im->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   dst->cs.space = im->cs.space;
   dst->flags = im->flags;
   dst->cs.no_free = 0;
   if ((im->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     dst->cs.data = calloc(1, dst->image->h * sizeof(unsigned char *) * 2);
   evas_common_image_surface_alloc(dst->image);
   evas_common_image_colorspace_dirty(dst);

   return 0;
}

EAPI int
evas_common_image_colorspace_set(RGBA_Image* dst, int cspace)
{
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
	if (dst->image->no_free)
	  {
	     dst->image->data = NULL;
	     dst->image->no_free = 0;
             evas_common_image_surface_alloc(dst->image);
	  }
	if (dst->cs.data)
	  {
	     if (!dst->cs.no_free) free(dst->cs.data);
	  }
	dst->cs.data = calloc(1, dst->image->h * sizeof(unsigned char *) * 2);
	dst->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   dst->cs.space = cspace;
   evas_common_image_colorspace_dirty(dst);

   return 0;
}
