#include "evas_common.h"
#include "evas_private.h"

extern Evas_List *evas_modules;

EAPI RGBA_Image *
evas_common_load_image_from_file(const char *file, const char *key, RGBA_Image_Loadopts *lo)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   Evas_List *l;
   RGBA_Image *im;
   char *p;
   char *loader = NULL;
   Evas_Module *em;
   struct stat st;
   
   if (file == NULL) return NULL;

   im = evas_common_image_find(file, key, 0, lo);
   if (im)
     {
	evas_common_image_ref(im);
	return im;
     }
   if (stat(file, &st) < 0) return NULL;
   
   im = evas_common_image_new();
   if (!im) return NULL;
   
   im->timestamp = st.st_mtime;
   im->laststat = time(NULL);
   
   if (lo) im->load_opts = *lo;
   
   p = strrchr(file, '.');
   if (p)
     {
        p++;
        if (!strcasecmp(p, "png"))
           loader = "png";
        else if ((!strcasecmp(p, "jpg")) || (!strcasecmp(p, "jpeg")) ||
            (!strcasecmp(p, "jfif")))
           loader = "jpeg";
        else if ((!strcasecmp(p, "eet")) || (!strcasecmp(p, "edj")) ||
            (!strcasecmp(p, "eap")))
           loader = "eet";
        else if (!strcasecmp(p, "edb"))
           loader = "edb";
        else if ((!strcasecmp(p, "tiff")) || (!strcasecmp(p, "tif")))
           loader = "tiff";
        else if (!strcasecmp(p, "xpm"))
           loader = "xpm";
        else if (!strcasecmp(p, "svg"))
           loader = "svg";
        else if (!strcasecmp(p, "svgz"))
           loader = "svg";
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
		  if (evas_image_load_func->file_head(im, file, key))
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
	if (evas_image_load_func->file_head(im, file, key))
	  {
	     if (evas_modules != l)
	       {
		  evas_modules = evas_list_promote_list(evas_modules, l);
	       }
	     goto ok;
	  }
     }
   
   evas_common_image_free(im);
   return NULL;
   ok:

   im->info.module = (void *)em;
   im->info.loader = (void *)evas_image_load_func;
   im->info.file = (char *)evas_stringshare_add(file);
   if (key) im->info.key = (char *)evas_stringshare_add(key);
   evas_module_ref((Evas_Module *)im->info.module);
   evas_common_image_ref(im);
   return im;
}

EAPI void
evas_common_load_image_data_from_file(RGBA_Image *im)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   
   if (im->image->data) return;
   if (!im->info.module) return;

   evas_image_load_func = im->info.loader;
   evas_module_use((Evas_Module *)im->info.module);
   if (!evas_image_load_func->file_data(im, im->info.file, im->info.key))
     {
	evas_common_image_surface_alloc(im->image);
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
}
