#include <Eet.h>

#include "evas_common.h"
#include "evas_private.h"


int evas_image_load_file_head_eet(RGBA_Image *im, const char *file, const char *key);
int evas_image_load_file_data_eet(RGBA_Image *im, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_eet_func =
{
  evas_image_load_file_head_eet,
  evas_image_load_file_data_eet
};


int
evas_image_load_file_head_eet(RGBA_Image *im, const char *file, const char *key)
{
   int                  alpha, compression, quality, lossy;
   unsigned int         w, h;
   Eet_File            *ef;
   int                  ok;

   if ((!file) || (!key)) return 0;
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef) return 0;
   ok = eet_data_image_header_read(ef, (char *)key,
				   &w, &h, &alpha, &compression, &quality, &lossy);
   if (!ok)
     {
	eet_close(ef);
	return 0;
     }
   if ((w > 8192) || (h > 8192))
     {
	eet_close(ef);
	return 0;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	eet_close(ef);
	return 0;
     }
   im->image->w = w;
   im->image->h = h;
   eet_close(ef);
   return 1;
}

int
evas_image_load_file_data_eet(RGBA_Image *im, const char *file, const char *key)
{
   unsigned int         w, h;
   int                  alpha, compression, quality, lossy;
   Eet_File            *ef;
   DATA32              *body, *p, *end;

   if ((!file) || (!key)) return 0;
   if ((im->image) && (im->image->data)) return 1;
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef) return 0;
   body = eet_data_image_read(ef, (char *)key,
			      &w, &h, &alpha, &compression, &quality, &lossy);
   if (!body)
     {
	eet_close(ef);
	return 0;
     }
   if ((w > 8192) || (h > 8192))
     {
	free(body);
	eet_close(ef);
	return 0;
     }
   if (alpha) im->flags |= RGBA_IMAGE_HAS_ALPHA;
   if (!im->image)
     im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	free(body);
	eet_close(ef);
	return 0;
     }
   im->image->w = w;
   im->image->h = h;
   im->image->data = body;
   im->image->no_free = 0;
   if (alpha)
     {
	end = body +(w * h);
	for (p = body; p < end; p++)
	  {
	     DATA32 r, g, b, a;
	     
	     a = A_VAL(p);
	     r = R_VAL(p);
	     g = G_VAL(p);
	     b = B_VAL(p);
	     if (r > a) r = a;
	     if (g > a) g = a;
	     if (b > a) b = a;
	     *p = ARGB_JOIN(a, r, g, b);
	  }
     }
// result is already premultiplied now if u compile with edje   
//   evas_common_image_premul(im);
   eet_close(ef);
   return 1;
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
