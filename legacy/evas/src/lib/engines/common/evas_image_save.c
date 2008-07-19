/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_options.h"

#include "evas_common.h"
#include "evas_private.h"


extern Evas_List *evas_modules;

int
evas_common_save_image_to_file(RGBA_Image *im, const char *file, const char *key, int quality, int compress)
{
   Evas_Image_Save_Func *evas_image_save_func = NULL;
   char *p;
   char *saver = NULL;

   p = strrchr(file, '.');
   if (p)
     {
	p++;

	if (!strcasecmp(p, "png"))
          saver = "png";
	if ((!strcasecmp(p, "jpg")) || (!strcasecmp(p, "jpeg")) ||
	    (!strcasecmp(p, "jfif")))
          saver = "jpeg";
	if ((!strcasecmp(p, "eet")) || (!strcasecmp(p, "edj")) ||
            (!strcasecmp(p, "eap")))
          saver = "eet";
	if (!strcasecmp(p, "edb"))
          saver = "edb";
     }

   if (saver)
     {
        Evas_Module *em;

	em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_SAVER, saver);
	if (em)
	  {
	     evas_module_use(em);
	     if (evas_module_load(em))
	       {
		  evas_image_save_func = em->functions;
		  return evas_image_save_func->image_save(im, file, key, quality, compress);
	       }
	  }
     }
   return 0;
}
