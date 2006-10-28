#include "evas_common.h"
#include "evas_private.h"

#include <Edb.h>
#include <zlib.h>

int evas_image_save_file_edb(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

Evas_Image_Save_Func evas_image_save_edb_func =
{
   evas_image_save_file_edb
};

int
evas_image_save_file_edb(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return 0;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_edb_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_SAVER,
     "edb",
     "none"
};
