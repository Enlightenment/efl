#include <Eet.h>

#include "evas_common.h"
#include "evas_private.h"


int evas_image_save_file_eet(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

Evas_Image_Save_Func evas_image_save_eet_func =
{
   evas_image_save_file_eet
};

int
evas_image_save_file_eet(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   Eet_File            *ef;
   int alpha = 0, lossy = 0, ok = 0;

   ef = eet_open((char *)file, EET_FILE_MODE_READ_WRITE);
   if (!ef) ef = eet_open((char *)file, EET_FILE_MODE_WRITE);
   if (!ef) return 0;
   if ((quality <= 100) || (compress < 0)) lossy = 1;
   if (im->flags & RGBA_IMAGE_HAS_ALPHA) alpha = 1;
   ok = eet_data_image_write(ef, (char *)key, im->image->data,
			     im->image->w, im->image->h, alpha, compress,
			     quality, lossy);
   eet_close(ef);
   return ok;
}

int module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_eet_func);
   return 1;
}

void module_close(void)
{
   
}

Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_SAVER,
     "eet",
     "none"
};
