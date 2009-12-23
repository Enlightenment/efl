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

static struct ext_loader_s	const loaders[] = {
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

static const char *loaders_name[] = {
  "png", "jpeg", "eet", "xpm", "tiff", "gif", "svg", "pmaps", "edb"
};

struct evas_image_foreach_loader_data
{
   Image_Entry *ie;
   int *error;
   Evas_Module *em;
};


static Eina_Bool
_evas_image_foreach_loader(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   Evas_Module *em = data;
   struct evas_image_foreach_loader_data *d = fdata;
   Image_Entry *ie = d->ie;

   if (!evas_module_load(em)) return EINA_TRUE;
   evas_image_load_func = em->functions;
   evas_module_use(em);
   *(d->error) = EVAS_LOAD_ERROR_NONE;
   if (evas_image_load_func &&
       evas_image_load_func->file_head(ie, ie->file, ie->key, d->error) &&
       (*(d->error) == EVAS_LOAD_ERROR_NONE))
     {
	d->em = em;
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
   int                   ret = EVAS_LOAD_ERROR_NONE;
   struct evas_image_foreach_loader_data fdata;


#ifdef EVAS_CSERVE
   if (evas_cserve_use_get())
     {
	// TODO: handle errors from server and return them?
	DBG("try cserve '%s' '%s'", ie->file, ie->key ? ie->key : "");
        if (evas_cserve_image_load(ie, ie->file, ie->key, &(ie->load_opts)))
          {
	     DBG("try cserve '%s' '%s' loaded!",
		 ie->file, ie->key ? ie->key : "");
             return EVAS_LOAD_ERROR_NONE;
          }
     }
#endif   
   dot = strrchr (ie->file, '.');
   if (dot)
     {
	for (i = 0, ++dot; i < (sizeof (loaders) / sizeof (struct ext_loader_s)); ++i)
	  {
	     if (!strcasecmp(dot, loaders[i].extention))
	       {
		  loader = loaders[i].loader;
		  DBG("found loader '%s' matching extension in file '%s'",
		      loader, ie->file);
		  break;
	       }
	  }
     }

   if (loader)
     {
	em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loader);
	if (em)
	  {
	     DBG("found image loader '%s' (%p)", loader, em);
	     if (evas_module_load(em))
	       {
		  evas_module_use(em);
		  evas_image_load_func = em->functions;
		  ret = EVAS_LOAD_ERROR_NONE;
		  if (evas_image_load_func->file_head(ie, ie->file, ie->key, &ret))
		    {
		       DBG("loaded file head using module '%s' (%p): %s",
			   loader, em, ie->file);
		       goto end;
		    }
		  evas_module_unload(em);
		  DBG("failed to load file head using module '%s' (%p): "
		      "%s (%s)",
		      loader, em, ie->file, evas_load_error_str(ret));
	       }
	     else
	       WRN("failed to load module '%s' (%p)", loader, em);
	  }
     }

   fdata.ie = ie;
   fdata.error = &ret;
   fdata.em = NULL;
   ret = EVAS_LOAD_ERROR_NONE;
   evas_module_foreach_image_loader(_evas_image_foreach_loader, &fdata);
   em = fdata.em;
   evas_image_load_func = em ? em->functions : NULL;
   if (em) goto end;

   /* This is our last chance, try all known image loader. */
   /* FIXME: We could use eina recursive module search ability. */
   for (i = 0; i < sizeof (loaders_name) / sizeof (char *); i++)
     {
	em = evas_module_find_type(EVAS_MODULE_TYPE_IMAGE_LOADER, loaders_name[i]);
	if (em)
	  {
	     if (evas_module_load(em))
	       {
		  evas_module_use(em);
		  evas_image_load_func = em->functions;
		  ret = EVAS_LOAD_ERROR_NONE;
		  if (evas_image_load_func->file_head(ie, ie->file, ie->key, &ret))
		    {
		       DBG("brute force loader '%s' (%p) worked on %s",
			   loaders_name[i], em, ie->file);
		       goto end;
		    }
		  else
		    DBG("brute force loader '%s' (%p) failed on %s (%s)",
			loaders_name[i], em, ie->file,
			evas_load_error_str(ret));

		  evas_module_unload(em);
	       }
	     else
	       WRN("failed to load module '%s' (%p)", loaders_name[i], em);
	  }
	else
	  DBG("could not find module '%s'", loaders_name[i]);
     }

   DBG("exhausted all means to load image '%s'", ie->file);
   return EVAS_LOAD_ERROR_UNKNOWN_FORMAT;

   end:

   if (ret != EVAS_LOAD_ERROR_NONE)
     {
	const char *modname = NULL;
	int modversion = -1;
	if (em && em->definition)
	  {
	     modname = em->definition->name;
	     modversion = em->definition->version;
	  }
	WRN("loader '%s' (version %d) "
	    "handled file '%s', key '%s' with errors: %s",
	    modname ? modname : "<UNKNOWN>", modversion,
	    ie->file, ie->key ? ie->key : "",
	    evas_load_error_str(ret));
	goto end;
     }

   DBG("loader '%s' used for file %s",
       (em && em->definition && em->definition->name) ?
       em->definition->name : "<UNKNOWN>",
       ie->file);

   ie->info.module = (void*) em;
   ie->info.loader = (void*) evas_image_load_func;
   evas_module_ref((Evas_Module*) ie->info.module);
   return ret;
}

EAPI int
evas_common_load_rgba_image_data_from_file(Image_Entry *ie)
{
   Evas_Image_Load_Func *evas_image_load_func = NULL;
   int ret = EVAS_LOAD_ERROR_NONE;

   if (ie->flags.loaded) return EVAS_LOAD_ERROR_GENERIC;

#ifdef EVAS_CSERVE
   if (ie->data1)
     {
        if (evas_cserve_image_data_load(ie))
          {
             RGBA_Image *im = (RGBA_Image *)ie;
             Mem *mem = ie->data2;
             if (mem)
               {
		  im->image.data = (void*) (mem->data + mem->offset);
                  im->image.no_free = 1;
                  return EVAS_LOAD_ERROR_NONE;
               }
          }
	return EVAS_LOAD_ERROR_GENERIC;
     }
#endif

   if (!ie->info.module) return EVAS_LOAD_ERROR_GENERIC;

   evas_image_load_func = ie->info.loader;
   evas_module_use((Evas_Module*) ie->info.module);
   if (!evas_image_load_func->file_data(ie, ie->file, ie->key, &ret))
     {
        return ret;
     }

//   evas_module_unref((Evas_Module*) ie->info.module);
//   ie->info.module = NULL;

   return EVAS_LOAD_ERROR_NONE;
}
