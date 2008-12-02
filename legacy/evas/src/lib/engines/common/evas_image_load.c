/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"

extern Eina_List *evas_modules;

struct ext_loader_s {
   const char*	extention;
   const char*	loader;
};

static struct ext_loader_s	loaders[] = {
   { "png", "png" },
   { "jpg", "jpeg" },
   { "jpeg", "jpeg" },
   { "jfif", "jpeg" },
   { "eet", "eet" },
   { "edj", "eet" },
   { "eap", "eet" },
   { "edb", "edb" },
   { "xpm", "xpm" },
   { "tiff", "tiff" },
   { "tif", "tiff" },
   { "svg", "svg" },
   { "svgz", "svg" },
   { "gif", "gif" },
   { "pbm", "pmaps" },
   { "pgm", "pmaps" },
   { "ppm", "pmaps" },
   { "pnm", "pmaps" }
};

int
evas_common_load_rgba_image_module_from_file(Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   const char           *loader = NULL;
   Eina_List            *l;
   Evas_Module          *em;
   char                 *dot;
   int                   i;

   dot = strrchr (ie->file, '.');
   if (dot)
     {
	for (i = 0, ++dot; i < (sizeof (loaders) / sizeof (struct ext_loader_s)); ++i)
	  {
	     if (!strcasecmp (dot, loaders[i].extention))
	       {
		  loader = loaders[i].loader;
		  break;
	       }
	  }
     }

   if (loader)
     {
	em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
	if (em)
	  {
	     if (evas_module_load(em))
	       {
		  evas_module_use(em);
		  evas_image_load_func = em->functions;
		  if (evas_image_load_func->file_head(ie, ie->file, ie->key))
		    goto ok;
	       }
	  }
     }

   EINA_LIST_FOREACH(evas_modules, l, em)
     {
	if (em->type != EVAS_MODULE_TYPE_IMAGE_LOADER) continue;
	if (!evas_module_load(em)) continue;
        evas_image_load_func = em->functions;
	evas_module_use(em);
	if (evas_image_load_func->file_head(ie, ie->file, ie->key))
	  {
	     if (evas_modules != l)
	       {
		  evas_modules = eina_list_promote_list(evas_modules, l);
	       }
	     goto ok;
	  }
     }

   return -1;

  ok:
   ie->info.module = (void*) em;
   ie->info.loader = (void*) evas_image_load_func;
   evas_module_ref((Evas_Module*) ie->info.module);
   return 0;
}

int
evas_common_load_rgba_image_data_from_file(Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;

   if (!ie->info.module) return -1;
   if (ie->flags.loaded) return -1;

   evas_image_load_func = ie->info.loader;
   evas_module_use((Evas_Module*) ie->info.module);
   if (!evas_image_load_func->file_data(ie, ie->file, ie->key))
     return -1;

   evas_module_unref((Evas_Module*) ie->info.module);
   ie->info.module = NULL;

   return 0;
}
