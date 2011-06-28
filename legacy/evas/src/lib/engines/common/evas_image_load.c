#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "evas_common.h"
#include "evas_private.h"
#include "evas_cs.h"

struct ext_loader_s
{
   unsigned int length;
   const char *extension;
   const char *loader;
};

#define MATCHING(Ext, Module)                   \
  { sizeof (Ext), Ext, Module }

static const struct ext_loader_s loaders[] =
{ /* map extensions to loaders to use for good first-guess tries */
   MATCHING(".png", "png"),
   MATCHING(".jpg", "jpeg"),
   MATCHING(".jpeg", "jpeg"),
   MATCHING(".jfif", "jpeg"),
   MATCHING(".eet", "eet"),
   MATCHING(".edj", "eet"),
   MATCHING(".eap", "eet"),
   MATCHING(".edb", "edb"),
   MATCHING(".xpm", "xpm"),
   MATCHING(".tiff", "tiff"),
   MATCHING(".tif", "tiff"),
   MATCHING(".svg", "svg"),
   MATCHING(".svgz", "svg"),
   MATCHING(".svg.gz", "svg"),
   MATCHING(".gif", "gif"),
   MATCHING(".pbm", "pmaps"),
   MATCHING(".pgm", "pmaps"),
   MATCHING(".ppm", "pmaps"),
   MATCHING(".pnm", "pmaps"),
   MATCHING(".bmp", "bmp"),
   MATCHING(".tga", "tga"),
   MATCHING(".wbmp", "wbmp"),
   MATCHING(".ico", "ico"),
   MATCHING(".cur", "ico"),
   MATCHING(".psd", "psd"),
   MATCHING(".pdf", "generic"),
   MATCHING(".ps", "generic"),
   MATCHING(".xcf", "generic"),
   MATCHING(".xcf.gz", "generic"),
   /* RAW */
   MATCHING(".arw", "generic"),
   MATCHING(".cr2", "generic"),
   MATCHING(".crw", "generic"),
   MATCHING(".dcr", "generic"),
   MATCHING(".dng", "generic"),
   MATCHING(".k25", "generic"),
   MATCHING(".kdc", "generic"),
   MATCHING(".erf", "generic"),
   MATCHING(".mrw", "generic"),
   MATCHING(".nef", "generic"),
   MATCHING(".nrf", "generic"),
   MATCHING(".nrw", "generic"),
   MATCHING(".orf", "generic"),
   MATCHING(".raw", "generic"),
   MATCHING(".rw2", "generic"),
   MATCHING(".pef", "generic"),
   MATCHING(".raf", "generic"),
   MATCHING(".sr2", "generic"),
   MATCHING(".srf", "generic"),
   MATCHING(".x3f", "generic"),
   /* video */
   MATCHING(".264", "generic"),
   MATCHING(".3g2", "generic"),
   MATCHING(".3gp", "generic"),
   MATCHING(".3gp2", "generic"),
   MATCHING(".3gpp", "generic"),
   MATCHING(".3gpp2", "generic"),
   MATCHING(".3p2", "generic"),
   MATCHING(".asf", "generic"),
   MATCHING(".avi", "generic"),
   MATCHING(".bdm", "generic"),
   MATCHING(".bdmv", "generic"),
   MATCHING(".clpi", "generic"),
   MATCHING(".clp", "generic"),
   MATCHING(".fla", "generic"),
   MATCHING(".flv", "generic"),
   MATCHING(".m1v", "generic"),
   MATCHING(".m2v", "generic"),
   MATCHING(".m2t", "generic"),
   MATCHING(".m4v", "generic"),
   MATCHING(".mkv", "generic"),
   MATCHING(".mov", "generic"),
   MATCHING(".mp2", "generic"),
   MATCHING(".mp2ts", "generic"),
   MATCHING(".mp4", "generic"),
   MATCHING(".mpe", "generic"),
   MATCHING(".mpeg", "generic"),
   MATCHING(".mpg", "generic"),
   MATCHING(".mpl", "generic"),
   MATCHING(".mpls", "generic"),
   MATCHING(".mts", "generic"),
   MATCHING(".mxf", "generic"),
   MATCHING(".nut", "generic"),
   MATCHING(".nuv", "generic"),
   MATCHING(".ogg", "generic"),
   MATCHING(".ogm", "generic"),
   MATCHING(".ogv", "generic"),
   MATCHING(".rm", "generic"),
   MATCHING(".rmj", "generic"),
   MATCHING(".rmm", "generic"),
   MATCHING(".rms", "generic"),
   MATCHING(".rmx", "generic"),
   MATCHING(".rmvb", "generic"),
   MATCHING(".swf", "generic"),
   MATCHING(".ts", "generic"),
   MATCHING(".weba", "generic"),
   MATCHING(".webm", "generic"),
   MATCHING(".wmv", "generic")
};

static const char *loaders_name[] =
{ /* in order of most likely needed */
  "png", "jpeg", "eet", "xpm", "tiff", "gif", "svg", "pmaps", "bmp", "tga", "wbmp", "ico", "psd", "edb", "generic"
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
   const char           *loader = NULL, *end;
   Evas_Module          *em;
   struct stat		 st;
   unsigned int          i;
   int                   len, ret = EVAS_LOAD_ERROR_NONE;
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
   if (stat(ie->file, &st) != 0 || S_ISDIR(st.st_mode))
     {
        DBG("trying to open directory '%s' !", ie->file);
        return EVAS_LOAD_ERROR_DOES_NOT_EXIST;
     }

   len = strlen(ie->file);
   end = ie->file + len;
   for (i = 0; i < (sizeof (loaders) / sizeof(struct ext_loader_s)); i++)
     {
        int len2 = strlen(loaders[i].extension);
        if (len2 > len) continue;
        if (!strcasecmp(end - len2, loaders[i].extension))
          {
             loader = loaders[i].loader;
             DBG("known loader '%s' handles extension '%s' of file '%s'",
                 loader, end - len2, ie->file);
             break;
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
		  INF("failed to load file head using module '%s' (%p): "
		      "%s (%s)",
		      loader, em, ie->file, evas_load_error_str(ret));
	       }
	     else
	       WRN("failed to load module '%s' (%p)", loader, em);
	  }
	else
	  INF("image loader '%s' is not enabled or missing!", loader);
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
		    INF("brute force loader '%s' (%p) failed on %s (%s)",
			loaders_name[i], em, ie->file,
			evas_load_error_str(ret));

		  evas_module_unload(em);
	       }
	     else
	       INF("failed to load module '%s' (%p)", loaders_name[i], em);
	  }
	else
	  DBG("could not find module '%s'", loaders_name[i]);
     }

   INF("exhausted all means to load image '%s'", ie->file);
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

//   printf("load data [%p] %s %s\n", ie, ie->file, ie->key);
           
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

EAPI Eina_Bool
evas_common_extension_can_load_get(const char *file)
{
   unsigned int length;
   unsigned int i;

   length = eina_stringshare_strlen(file) + 1;
   if (length < 5) return EINA_FALSE;

   for (i = 0; i < sizeof (loaders) / sizeof (struct ext_loader_s); ++i)
     {
        if (loaders[i].length > length) continue;

        if (!strcasecmp(loaders[i].extension, file + length - loaders[i].length))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}
