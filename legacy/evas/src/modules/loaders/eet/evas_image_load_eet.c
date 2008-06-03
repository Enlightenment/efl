#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#include <Eet.h>

#include "evas_common.h"
#include "evas_private.h"


int evas_image_load_file_head_eet(Image_Entry *ie, const char *file, const char *key);
int evas_image_load_file_data_eet(Image_Entry *ie, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_eet_func =
{
  evas_image_load_file_head_eet,
  evas_image_load_file_data_eet
};


int
evas_image_load_file_head_eet(Image_Entry *ie, const char *file, const char *key)
{
   int                  alpha, compression, quality, lossy;
   unsigned int         w, h;
   Eet_File            *ef;
   int                  ok;
   int			res = 0;

   if ((!file) || (!key)) return 0;
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef) return 0;
   ok = eet_data_image_header_read(ef, key,
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (!ok) goto on_error;
   if (alpha) ie->flags.alpha = 1;
   ie->w = w;
   ie->h = h;
   res = 1;

 on_error:
   eet_close(ef);
   return res;
}

int
evas_image_load_file_data_eet(Image_Entry *ie, const char *file, const char *key)
{
   unsigned int         w, h;
   int                  alpha, compression, quality, lossy, ok;
   Eet_File            *ef;
   DATA32              *body, *p, *end;
   DATA32               nas = 0;
   int			res = 0;

   if ((!file) || (!key)) return 0;
   if (ie->flags.loaded) return 1;
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef) return 0;
   ok = eet_data_image_header_read(ef, key,
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (!ok) goto on_error;
   evas_cache_image_surface_alloc(ie, w, h);
   ok = eet_data_image_read_to_surface(ef, key, 0, 0,
				       evas_cache_image_pixels(ie), w, h, w * 4,
				       &alpha, &compression, &quality, &lossy);
   if (!ok) goto on_error;
   if (alpha)
     {
	ie->flags.alpha = 1;

	body = evas_cache_image_pixels(ie);

	end = body +(w * h);
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
	if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
	  ie->flags.alpha_sparse = 1;
     }
// result is already premultiplied now if u compile with edje
//   evas_common_image_premul(im);
   res = 1;

 on_error:
   eet_close(ef);
   return res;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_eet_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "eet",
     "none"
};
