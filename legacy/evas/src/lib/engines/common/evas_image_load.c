/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
#include "evas_cs.h"

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

static Eina_Bool
_evas_image_foreach_loader(const Eina_Hash *hash, const char *key, Evas_Module *em, Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;

   if (!evas_module_load(em)) return EINA_TRUE;
   evas_image_load_func = em->functions;
   evas_module_use(em);
   if (evas_image_load_func && evas_image_load_func->file_head(ie, ie->file, ie->key))
     {
	ie->info.module = (void*) em;
	ie->info.loader = (void*) evas_image_load_func;
	evas_module_ref((Evas_Module*) ie->info.module);
	return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI int
evas_common_load_rgba_image_module_from_file(Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   const char           *loader = NULL;
   Evas_Module          *em;
   char                 *dot;
   int                   i;


#ifdef EVAS_CSERVE
   if (evas_cserve_use_get())
     {
        if (evas_cserve_image_load(ie, ie->file, ie->key, &(ie->load_opts)))
          {
             return 0;
          }
     }
#endif   
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

   /* FIXME: We don't try not loaded module yet, changed behaviour with previous one. */
   evas_module_foreach_image_loader(_evas_image_foreach_loader, ie);
   if (ie->info.module) return 0;

   return -1;

   ok:
   ie->info.module = (void*) em;
   ie->info.loader = (void*) evas_image_load_func;
   evas_module_ref((Evas_Module*) ie->info.module);
   return 0;
}

EAPI int
evas_common_load_rgba_image_data_from_file(Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;

   if (ie->flags.loaded) return -1;

#ifdef EVAS_CSERVE
   if (ie->data1)
     {
        if (evas_cserve_image_data_load(ie))
          {
             RGBA_Image *im = (RGBA_Image *)ie;
             Mem *mem;
             
             mem = ie->data2;
             if (mem)
               {
                  im->image.data = mem->data + mem->offset;
                  im->image.no_free = 1;
                  return 0;
               }
          }
        return -1;
     }
#endif
   
   if (!ie->info.module) return -1;

   evas_image_load_func = ie->info.loader;
   evas_module_use((Evas_Module*) ie->info.module);
   if (!evas_image_load_func->file_data(ie, ie->file, ie->key))
     return -1;

//   evas_module_unref((Evas_Module*) ie->info.module);
//   ie->info.module = NULL;

   return 0;
}
