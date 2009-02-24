#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#ifdef BUILD_FONT_LOADER_EET
#include <Eet.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

EAPI Evas_Imaging_Image *
evas_imaging_image_load(const char *file, const char *key)
{
   Evas_Imaging_Image *im;
   RGBA_Image *image;

   if (!file) file = "";
   if (!key) key = "";
   evas_common_cpu_init();
   evas_common_image_init();
   image = evas_common_load_image_from_file(file, key, NULL);
   if (!image) return NULL;
   im = calloc(1, sizeof(Evas_Imaging_Image));
   if (!im)
     {
        evas_cache_image_drop(&image->cache_entry);
	return NULL;
     }
   im->image = image;
   return im;
}

EAPI void
evas_imaging_image_free(Evas_Imaging_Image *im)
{
   if (!im) return;
   evas_cache_image_drop(&im->image->cache_entry);
   free(im);
}

EAPI void
evas_imaging_image_size_get(const Evas_Imaging_Image *im, int *w, int *h)
{
   if (!im) return;
   if (w) *w = im->image->cache_entry.w;
   if (h) *h = im->image->cache_entry.h;
}

EAPI Evas_Bool
evas_imaging_image_alpha_get(const Evas_Imaging_Image *im)
{
   if (!im) return 0;
   if (im->image->cache_entry.flags.alpha) return 1;
   return 0;
}

EAPI void
evas_imaging_image_cache_set(int bytes)
{
   evas_common_image_set_cache(bytes);
}

EAPI int
evas_imaging_image_cache_get(void)
{
   return evas_common_image_get_cache();
}

static Evas_Font_Hinting_Flags _evas_hinting = EVAS_FONT_HINTING_BYTECODE;

EAPI void
evas_imaging_font_hinting_set(Evas_Font_Hinting_Flags hinting)
{
   _evas_hinting = hinting;
}

EAPI Evas_Font_Hinting_Flags
evas_imaging_font_hinting_get(void)
{
   return _evas_hinting;
}

EAPI Evas_Bool
evas_imaging_font_hinting_can_hint(Evas_Font_Hinting_Flags hinting)
{
   return evas_common_hinting_available(hinting);
}

EAPI Evas_Imaging_Font *
evas_imaging_font_load(const char *file, const char *key, int size)
{
   Evas_Imaging_Font *fn;
   RGBA_Font *font = NULL;

   evas_common_cpu_init();
   evas_common_font_init();
   if (!file) file = "";
   if ((key) && (key[0] == 0)) key = NULL;
#ifdef BUILD_FONT_LOADER_EET
   if (key)
     {
	char *tmp;

	tmp = evas_file_path_join(file, key);
	if (tmp)
	  {
	     font = evas_common_font_hinting_load(tmp, size, _evas_hinting);
	     if (!font)
	       {
		  Eet_File *ef;

		  ef = eet_open((char *)file, EET_FILE_MODE_READ);
		  if (ef)
		    {
		       void *fdata;
		       int fsize = 0;

		       fdata = eet_read(ef, (char *)key, &fsize);
		       if ((fdata) && (fsize > 0))
			 {
			    font = evas_common_font_memory_hinting_load(tmp, size, fdata, fsize, _evas_hinting);
			    free(fdata);
			 }
		       eet_close(ef);
		    }
	       }
	     free(tmp);
	  }
     }
   else
#endif
     {
	font = evas_common_font_hinting_load((char *)file, size, _evas_hinting);
     }
   if (!font) return NULL;
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn) return NULL;
   fn->font = font;
   return fn;
}

EAPI void
evas_imaging_font_free(Evas_Imaging_Font *fn)
{
   if (!fn) return;
   evas_common_font_free(fn->font);
   free(fn);
}

EAPI int
evas_imaging_font_ascent_get(const Evas_Imaging_Font *fn)
{
   if (!fn) return 0;
   return evas_common_font_ascent_get(fn->font);
}

EAPI int
evas_imaging_font_descent_get(const Evas_Imaging_Font *fn)
{
   if (!fn) return 0;
   return evas_common_font_descent_get(fn->font);
}

EAPI int
evas_imaging_font_max_ascent_get(const Evas_Imaging_Font *fn)
{
   if (!fn) return 0;
   return evas_common_font_max_ascent_get(fn->font);
}

EAPI int
evas_imaging_font_max_descent_get(const Evas_Imaging_Font *fn)
{
   if (!fn) return 0;
   return evas_common_font_max_descent_get(fn->font);
}

EAPI int
evas_imaging_font_line_advance_get(const Evas_Imaging_Font *fn)
{
   if (!fn) return 0;
   return evas_common_font_get_line_advance(fn->font);
}

EAPI void
evas_imaging_font_string_advance_get(const Evas_Imaging_Font *fn, const char *str, int *x, int *y)
{
   if (!fn) return 0;
   evas_common_font_query_advance(fn->font, str, x, y);
}

EAPI void
evas_imaging_font_string_size_query(const Evas_Imaging_Font *fn, const char *str, int *w, int *h)
{
   if (!fn) return 0;
   evas_common_font_query_size(fn->font, str, w, h);
}

EAPI int
evas_imaging_font_string_inset_get(const Evas_Imaging_Font *fn, const char *str)
{
   if (!fn) return 0;
   return evas_common_font_query_inset(fn->font, str);
}

EAPI int
evas_imaging_font_string_char_coords_get(const Evas_Imaging_Font *fn, const char *str, int pos, int *cx, int *cy, int *cw, int *ch)
{
   if (!fn) return 0;
   return evas_common_font_query_char_coords(fn->font, str, pos, cx, cy, cw, ch);
}

EAPI int
evas_imaging_font_string_char_at_coords_get(const Evas_Imaging_Font *fn, const char *str, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   if (!fn) return -1;
   return evas_common_font_query_text_at_pos(fn->font, str, x, y, cx, cy, cw, ch);
}

EAPI void
evas_imaging_font_cache_set(int bytes)
{
   evas_common_font_cache_set(bytes);
}

EAPI int
evas_imaging_font_cache_get(void)
{
   return evas_common_font_cache_get();
}

