#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#include <Eet.h>

#include "evas_common_private.h"
#include "evas_private.h"

static int evas_image_save_file_eet(RGBA_Image *im, const char *file, const char *key, int quality, int compress, const char *encoding);

static Evas_Image_Save_Func evas_image_save_eet_func =
{
   evas_image_save_file_eet
};

static int
evas_image_save_file_eet(RGBA_Image *im, const char *file, const char *key,
                         int quality, int compress, const char *encoding EINA_UNUSED)
{
   Eet_File            *ef;
   int alpha = 0, lossy = 0, ok = 0;
   DATA32   *data;

   if (!im || !im->image.data || !file)
      return 0;

   ef = eet_open((char *)file, EET_FILE_MODE_READ_WRITE);
   if (!ef) ef = eet_open((char *)file, EET_FILE_MODE_WRITE);
   if (!ef) return 0;
   if ((quality <= 100) || (compress < 0)) lossy = 1;
   if (im->cache_entry.flags.alpha) alpha = 1;
//   if (alpha)
//     {
//       data = malloc(im->image->w * im->image->h * sizeof(DATA32));
//       if (!data)
//	 {
//	   eet_close(ef);
//	   return 0;
//	 }
//       memcpy(data, im->image->data, im->image->w * im->image->h * sizeof(DATA32));
//       evas_common_convert_argb_unpremul(data, im->image->w * im->image->h);
//     }
//   else
       data = im->image.data;
   ok = eet_data_image_write(ef, (char *)key, data,
			     im->cache_entry.w, im->cache_entry.h, alpha, compress,
			     quality, lossy);
//   if (alpha)
//     free(data);
   eet_close(ef);
   return ok;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_eet_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "eet",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, eet);

#ifndef EVAS_STATIC_BUILD_EET
EVAS_EINA_MODULE_DEFINE(image_saver, eet);
#endif

