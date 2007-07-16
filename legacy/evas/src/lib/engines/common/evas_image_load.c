#include "evas_common.h"
#include "evas_private.h"

extern Evas_List *evas_modules;

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
   { "svgz", "svg" }
};

EAPI int
evas_common_load_image_module_from_file(RGBA_Image *im)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   const char           *loader = NULL;
   char                 *dot;
   Evas_List            *l;
   Evas_Module          *em;
   int                  i;

   dot = strrchr (im->info.file, '.');
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
		  if (evas_image_load_func->file_head(im, im->info.file, im->info.key))
		    goto ok;
	       }
	  }
     }

   for (l = evas_modules; l; l = l->next)
     {
	em = l->data;
	if (em->type != EVAS_MODULE_TYPE_IMAGE_LOADER) continue;
	if (!evas_module_load(em)) continue;
        evas_image_load_func = em->functions;
	evas_module_use(em);
	if (evas_image_load_func->file_head(im, im->info.file, im->info.key))
	  {
	     if (evas_modules != l)
	       {
		  evas_modules = evas_list_promote_list(evas_modules, l);
	       }
	     goto ok;
	  }
     }

   evas_common_image_delete(im);
   return -1;

  ok:
   im->info.module = (void *)em;
   im->info.loader = (void *)evas_image_load_func;
   evas_module_ref((Evas_Module *)im->info.module);
   return 0;
}

EAPI void
evas_common_load_image_data_from_file(RGBA_Image *im)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;

   if ((im->flags & RGBA_IMAGE_LOADED) == RGBA_IMAGE_LOADED) return ;
   if (!im->info.module) return;

   evas_image_load_func = im->info.loader;
   evas_module_use((Evas_Module *)im->info.module);
   if (!evas_image_load_func->file_data(im, im->info.file, im->info.key))
     {
        if (!im->image->data) evas_common_image_surface_alloc(im->image);
	if (!im->image->data)
	  {
	     const DATA32 pixel = 0xffffffff;

	     im->image->w = 1;
	     im->image->h = 1;
	     im->image->data = (DATA32 *)&pixel;
	     im->image->no_free = 1;
	  }
     }
   else
     {
	evas_module_unref((Evas_Module *)im->info.module);
	im->info.module = NULL;
     }
   im->flags |= RGBA_IMAGE_LOADED;
}
