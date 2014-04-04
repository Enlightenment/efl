#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#include <Eet.h>

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eet_File *ef;
   const char *key;
};

static void *
evas_image_load_file_open_eet(Eina_File *f, Eina_Stringshare *key,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   if (!key)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   loader->ef = eet_mmap(f);
   if (!loader->ef)
     {
        free(loader);
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader->key = eina_stringshare_ref(key);
   return loader;
}

static void
evas_image_load_file_close_eet(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eet_close(loader->ef);
   eina_stringshare_del(loader->key);
   free(loader);
}

static inline Eina_Bool
_evas_image_load_return_error(int err, int *error)
{
   *error = err;
   return EINA_FALSE;
}

static const Evas_Colorspace cspaces_etc1[2] = {
  EVAS_COLORSPACE_ETC1,
  EVAS_COLORSPACE_ARGB8888
};

static Eina_Bool
evas_image_load_file_head_eet(void *loader_data,
			      Evas_Image_Property *prop,
			      int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   int       a, compression, quality;
   Eet_Image_Encoding lossy;
   const Eet_Colorspace *cspaces = NULL;
   int       ok;

   ok = eet_data_image_header_read(loader->ef, loader->key,
                                   &prop->w, &prop->h, &a, &compression, &quality, &lossy);
   if (!ok)
     return _evas_image_load_return_error(EVAS_LOAD_ERROR_DOES_NOT_EXIST, error);
   if (IMG_TOO_BIG(prop->w, prop->h))
     return _evas_image_load_return_error(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED, error);

   if (eet_data_image_colorspace_get(loader->ef, loader->key, NULL, &cspaces))
     {
        unsigned int i;

	if (cspaces != NULL)
	  {
	    for (i = 0; cspaces[i] != EET_COLORSPACE_ARGB8888; i++)
	      if (cspaces[i] == EET_COLORSPACE_ETC1)
		{
		  prop->cspaces = cspaces_etc1;
		  break;
		}
	  }
     }

   prop->alpha = !!a;
   *error = EVAS_LOAD_ERROR_NONE;

   return EINA_TRUE;
}

Eina_Bool
evas_image_load_file_data_eet(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
			      int *error)
{
   Evas_Loader_Internal *loader = loader_data;
   int       alpha, compression, quality, ok;
   Eet_Image_Encoding lossy;
   DATA32   *body, *p, *end;
   DATA32    nas = 0;
   Eet_Colorspace cspace;

   switch (prop->cspace)
     {
      case EVAS_COLORSPACE_ETC1: cspace = EET_COLORSPACE_ETC1; break;
      case EVAS_COLORSPACE_ARGB8888: cspace = EET_COLORSPACE_ARGB8888; break;
      default:
         return _evas_image_load_return_error(EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED, error);
     }

   ok = eet_data_image_read_to_cspace_surface_cipher(loader->ef, loader->key, NULL, 0, 0,
                                                     pixels, prop->w, prop->h, prop->w * 4,
                                                     cspace,
                                                     &alpha, &compression, &quality, &lossy);
   if (!ok)
     return _evas_image_load_return_error(EVAS_LOAD_ERROR_GENERIC, error);

   if (alpha)
     {
        prop->alpha = 1;
	body = pixels;

	end = body + (prop->w * prop->h);
	for (p = body; p < end; p++)
	  {
	     DATA32 r, g, b, a;

	     a = A_VAL(p);
	     r = R_VAL(p);
	     g = G_VAL(p);
	     b = B_VAL(p);
	     if ((a == 0) || (a == 255)) nas++;
	     if (r > a) r = a;
	     if (g > a) g = a;
	     if (b > a) b = a;
	     *p = ARGB_JOIN(a, r, g, b);
	  }
	if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (prop->w * prop->h))
	  prop->alpha_sparse = 1;
     }
// result is already premultiplied now if u compile with edje
//   evas_common_image_premul(im);
   *error = EVAS_LOAD_ERROR_NONE;

   return EINA_TRUE;
}

Evas_Image_Load_Func evas_image_load_eet_func =
{
  evas_image_load_file_open_eet,
  evas_image_load_file_close_eet,
  evas_image_load_file_head_eet,
  evas_image_load_file_data_eet,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   eet_init();
   em->functions = (void *)(&evas_image_load_eet_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   eet_shutdown();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "eet",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, eet);

#ifndef EVAS_STATIC_BUILD_EET
EVAS_EINA_MODULE_DEFINE(image_loader, eet);
#endif
