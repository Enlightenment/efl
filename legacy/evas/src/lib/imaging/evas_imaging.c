#include "config.h"
#include "evas_options.h"
#include "evas_common.h"
#include "evas_private.h"

Evas_Imaging_Image *
evas_imaging_image_load(const char *file, const char *key)
{
   Evas_Imaging_Image *im;
   RGBA_Image *image;
   
   if (!file) file = "";
   if (!key) key = "";
   evas_common_cpu_init();
   evas_common_image_init();
   image = evas_common_load_image_from_file(file, key);
   if (!image) return NULL;
   im = calloc(1, sizeof(Evas_Imaging_Image));
   if (!im)
     {
	evas_common_image_free(image);
	return NULL;
     }
   im->image = image;
   return im;
}

void
evas_imaging_image_free(Evas_Imaging_Image *im)
{
   if (!im) return;
   evas_common_image_unref(im->image);
   free(im);
}

void
evas_imaging_image_size_get(Evas_Imaging_Image *im, int *w, int *h)
{
   if (!im) return;
   if (w) *w = im->image->image->w;
   if (h) *h = im->image->image->h;
}

Evas_Bool
evas_imaging_image_alpha_get(Evas_Imaging_Image *im)
{
   if (!im) return 0;
   if (im->image->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
   return 0;
}

void
evas_imaging_image_cache_set(int bytes)
{
   evas_common_image_set_cache(bytes);
}

int
evas_imaging_image_cache_get(void)
{
   return evas_common_image_get_cache();
}
 
