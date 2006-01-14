#include "evas_common.h"
#include "evas_private.h"

extern Evas_List *evas_modules;

static Evas_Image_Load_Func *evas_image_load_func = NULL;

static int evas_common_load_image_lookup(const char *name);

RGBA_Image *
evas_common_load_image_from_file(const char *file, const char *key)
{
   Evas_List *l;
   RGBA_Image *im;
   char *p;
   char *loader = NULL;

   if (file == NULL)
        return NULL;

   im = evas_common_image_find(file, key, 0);
   if (im)
     {
	evas_common_image_ref(im);
	return im;
     }
   im = evas_common_image_new();
   if (!im)
     {
	return NULL;
     }

   p = strrchr(file, '.');
   if (p)
     {
        p++;

#ifdef BUILD_LOADER_PNG
        if (!strcasecmp(p, "png"))
           loader = "png";
#endif
#ifdef BUILD_LOADER_JPEG
        if ((!strcasecmp(p, "jpg")) ||
            (!strcasecmp(p, "jpeg")) ||
            (!strcasecmp(p, "jfif")))
           loader = "jpeg";
#endif
#ifdef BUILD_LOADER_EET
        if (!strcasecmp(p, "eet"))
           loader = "eet";
#endif
#ifdef BUILD_LOADER_EDB
        if (!strcasecmp(p, "edb"))
           loader = "edb";
#endif
     }

   if (!loader)
      return NULL;

   for (l = evas_modules; l; l = l->next)
     {
        Evas_Module *em;
	Evas_Module_Image_Loader *emil;

	em = l->data;
	if (em->type != EVAS_MODULE_TYPE_IMAGE_LOADER) continue;
	if (!em->data) continue;
	emil = (Evas_Module_Image_Loader *)em->data;
	if (emil->id != evas_common_load_image_lookup(loader)) continue;
	if (!evas_module_load(em)) return NULL;
        evas_image_load_func = em->functions;
        goto beach;
     }
   return NULL;

 beach:
   if (!evas_image_load_func->file_head(im, file, key))
     {
        evas_common_image_free(im);
        return NULL;
     }
   
   im->info.file = (char *)evas_stringshare_add(file);

   if (key) im->info.key = (char *)evas_stringshare_add(key);
   evas_common_image_ref(im);
   return im;
}

void
evas_common_load_image_data_from_file(RGBA_Image *im)
{
   if (im->image->data) return;

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
}

static int
evas_common_load_image_lookup(const char *name)
{
   static int i = 1;
   Evas_Module *em;
   Evas_Module_Image_Loader *emil;
   
   if (!name) return RENDER_METHOD_INVALID;
   /* search on the engines list for the name */
   em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, name);
   
   if(!em) return RENDER_METHOD_INVALID;
   
   emil = (Evas_Module_Image_Loader *)em->data;
   if(!emil)
     {
	emil = (Evas_Module_Image_Loader *)malloc(sizeof(Evas_Module_Image_Loader));
	em->data = emil;
	emil->id = i;
	i++;
     }
   return emil->id;
}
