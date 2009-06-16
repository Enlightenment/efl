#include "evas_common.h"
#include "evas_private.h"

#include <Edb.h>
#include <zlib.h>

static int evas_image_save_file_edb(RGBA_Image *im, const char *file, const char *key, int quality, int compress);

static Evas_Image_Save_Func evas_image_save_edb_func =
{
   evas_image_save_file_edb
};

static int
evas_image_save_file_edb(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   return 0;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_save_edb_func);
   return 1;
}

static void
module_close(Evas_Module *em)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "edb",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_SAVER, image_saver, edb);

#ifndef EVAS_STATIC_BUILD_EDB
EVAS_EINA_MODULE_DEFINE(image_saver, edb);
#endif
