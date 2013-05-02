#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#include <Eet.h>

#include "evas_common.h"
#include "evas_private.h"

static Eina_Bool
evas_image_load_file_head_eet(Eina_File *f, const char *key,
			      Evas_Image_Property *prop,
			      Evas_Image_Load_Opts *opts EINA_UNUSED,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      int *error)
{
   Eet_File *ef = NULL;
   int       a, compression, quality, lossy;
   int       ok;
   Eina_Bool res = EINA_FALSE;

   if (!key)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   ef = eet_mmap(f);
   if (!ef)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	goto on_error;
     }
   ok = eet_data_image_header_read(ef, key,
				   &prop->w, &prop->h, &a, &compression, &quality, &lossy);
   if (!ok)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	goto on_error;
     }
   if (IMG_TOO_BIG(prop->w, prop->h))
     {
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	goto on_error;
     }
   prop->alpha = !!a;
   res = EINA_TRUE;
   *error = EVAS_LOAD_ERROR_NONE;

 on_error:
   if (ef) eet_close(ef);
   return res;
}

Eina_Bool
evas_image_load_file_data_eet(Eina_File *f, const char *key,
			      Evas_Image_Property *prop,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
			      Evas_Image_Animated *animated EINA_UNUSED,
			      void *pixels,
			      int *error)
{
   int       alpha, compression, quality, lossy, ok;
   Eet_File *ef;
   DATA32   *body, *p, *end;
   DATA32    nas = 0;
   Eina_Bool res = EINA_FALSE;

   if (!key)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ef = eet_mmap(f);
   if (!ef)
     {
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }
   ok = eet_data_image_read_to_surface(ef, key, 0, 0,
				       pixels, prop->w, prop->h, prop->w * 4,
				       &alpha, &compression, &quality, &lossy);
   if (!ok)
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	goto on_error;
     }
   
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
   res = EINA_TRUE;

 on_error:
   if (ef) eet_close(ef);
   return res;
}

Evas_Image_Load_Func evas_image_load_eet_func =
{
  EINA_TRUE,
  evas_image_load_file_head_eet,
  evas_image_load_file_data_eet,
  NULL,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_eet_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
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
