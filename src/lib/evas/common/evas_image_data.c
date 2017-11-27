#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_image.h"

int
evas_common_rgba_image_from_data(Image_Entry* ie_dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
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
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_ETC1_ALPHA:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        // FIXME: Borders are just guessed, not passed in (they should be)
        dst->cache_entry.w = w;
        dst->cache_entry.h = h;
        dst->cache_entry.borders.l = 1;
        dst->cache_entry.borders.t = 1;
        dst->cache_entry.borders.r = ((w + 2 + 3) & ~0x3) - w - 1;
        dst->cache_entry.borders.b = ((h + 2 + 3) & ~0x3) - h - 1;
        dst->image.data = image_data;
        dst->image.no_free = 1;
        dst->cache_entry.flags.alpha = alpha ? 1 : 0;
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
evas_common_rgba_image_from_copied_data(Image_Entry* ie_dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
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
evas_common_rgba_image_size_set(Image_Entry *ie_dst, const Image_Entry *ie_im, unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;
   RGBA_Image   *im = (RGBA_Image *) ie_im;

   if ((im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422P709_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR422601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR420TM12601_PL) ||
       (im->cache_entry.space == EVAS_COLORSPACE_YCBCR420NV12601_PL))
     {
        dst->cs.data = calloc(1, dst->cache_entry.h * sizeof(unsigned char *) * 2);
     }

   dst->flags = im->flags;
   dst->cs.no_free = 0;
   evas_common_image_colorspace_dirty(dst);

   _evas_common_rgba_image_post_surface(ie_dst);
   return 0;
}

int
evas_common_rgba_image_colorspace_set(Image_Entry* ie, Evas_Colorspace cspace)
{
   RGBA_Image *im = (RGBA_Image *) ie;

   // FIXME: This function looks extremely dubious now, trying to free the
   // data pointer without even knowing how it was allocated (malloc / mmap).
   // Also, lacks support for S3TC and exotic formats.

   if (im->cache_entry.space == cspace)
     return 1;

   if (ie->references > 1)
     WRN("Releasing data of image with >1 refs. Bad things may happen.");

   if (im->cs.data)
     {
        if (!im->cs.no_free) free(im->cs.data);
        im->cs.data = NULL;
     }
   im->cs.no_free = 0;
   if (im->image.data && !im->image.no_free)
     {
        // FIXME: Call _evas_common_rgba_image_surface_munmap
        free(im->image.data);
     }
   ie->allocated.w = 0;
   ie->allocated.h = 0;
   ie->flags.preload_done = 0;
   ie->flags.loaded = 0;
   im->image.data = NULL;
   im->image.no_free = 0;

   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
      case EVAS_COLORSPACE_AGRY88:
      case EVAS_COLORSPACE_GRY8:
        // all good
        break;
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_ETC1_ALPHA:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
        // living on the edge (no conversion atm)
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
        // prepare cspace conversion buffer
        im->cs.data = calloc(1, im->cache_entry.h * sizeof(unsigned char *) * 2);
        im->cs.no_free = 0;
        break;
      default:
        CRI("Can't set colorspace to %u: unsupported", (unsigned) cspace);
        abort();
        return 0;
     }
   im->cache_entry.space = cspace;
   evas_common_image_colorspace_dirty(im);
   _evas_common_rgba_image_post_surface(ie);
   return 1;
}
