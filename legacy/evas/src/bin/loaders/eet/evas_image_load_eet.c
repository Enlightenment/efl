#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#include <Eet.h>

#include "evas_macros.h"

#include "evas_cserve2.h"
#include "evas_cserve2_slave.h"


static Eina_Bool
evas_image_load_file_head_eet(Evas_Img_Load_Params *ilp, const char *file, const char *key, int *error)
{
   int                  alpha, compression, quality, lossy;
   unsigned int         w, h;
   Eet_File            *ef;
   int                  ok;
   Eina_Bool		res = EINA_FALSE;

   if (!key)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }

   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ok = eet_data_image_header_read(ef, key,
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (!ok)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	goto on_error;
     }
   if (IMG_TOO_BIG(w, h))
     {
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto on_error;
     }
   if (alpha) ilp->alpha = 1;
   ilp->w = w;
   ilp->h = h;
   res = EINA_TRUE;
   *error = CSERVE2_NONE;

 on_error:
   eet_close(ef);
   return res;
}

Eina_Bool
evas_image_load_file_data_eet(Evas_Img_Load_Params *ilp, const char *file, const char *key, int *error)
{
   unsigned int         w, h;
   int                  alpha, compression, quality, lossy, ok;
   Eet_File            *ef;
   DATA32              *body, *p, *end, *data;
   DATA32               nas = 0;
   Eina_Bool		res = EINA_FALSE;

   if (!key)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ok = eet_data_image_header_read(ef, key,
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (IMG_TOO_BIG(w, h))
     {
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto on_error;
     }
   if (!ok)
     {
	*error = CSERVE2_DOES_NOT_EXIST;
	goto on_error;
     }
   data = ilp->buffer;
   if (!data)
     {
	*error = CSERVE2_RESOURCE_ALLOCATION_FAILED;
	goto on_error;
     }
   ok = eet_data_image_read_to_surface(ef, key, 0, 0,
				       data, w, h, w * 4,
				       &alpha, &compression, &quality, &lossy);
   if (!ok)
     {
	*error = CSERVE2_GENERIC;
	goto on_error;
     }
   if (alpha)
     {
	ilp->alpha = 1;

	body = ilp->buffer;

	end = body + (w * h);
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
	if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (w * h))
	  ilp->alpha_sparse = 1;
     }
   *error = CSERVE2_NONE;
   res = EINA_TRUE;

 on_error:
   eet_close(ef);
   return res;
}

static Evas_Loader_Module_Api modapi =
{
   EVAS_CSERVE2_MODULE_API_VERSION,
   "eet",
   evas_image_load_file_head_eet,
   evas_image_load_file_data_eet
};

static Eina_Bool
module_init(void)
{
   eet_init();
   return evas_cserve2_loader_register(&modapi);
}

static void
module_shutdown(void)
{
   eet_shutdown();
}

EINA_MODULE_INIT(module_init);
EINA_MODULE_SHUTDOWN(module_shutdown);
