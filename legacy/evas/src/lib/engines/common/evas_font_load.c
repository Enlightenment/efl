#include "evas_common.h"

extern FT_Library ft_lib;

static int                font_cache_usage = 0;
static int                font_cache = 0;
static Evas_Object_List * fonts = NULL;

static int font_modify_cache_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
static int font_flush_free_glyph_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

/* FIXME: */
/* we should share face handles and have different ft sizes from the same */
/* face (if applicable) */

RGBA_Font *
evas_common_font_load(const char *name, int size)
{
   int error;
   RGBA_Font *fn;
   char *file;

   fn = evas_common_font_find(name, size);
   if (fn) return fn;
   
   fn = malloc(sizeof(RGBA_Font));   
   file = (char *)name;
   
   error = FT_New_Face(ft_lib, file, 0, &(fn->ft.face));
   if (error)
     {
	free(fn);
	return NULL;
     }
   error = FT_Set_Char_Size(fn->ft.face, 0, (size * 64), 96, 96);
   if (error)
     error = FT_Set_Pixel_Sizes(fn->ft.face, 0, size);
   if (error)
     {
	int i;
	int chosen_size = 0;
	int chosen_width = 0;

	for (i = 0; i < fn->ft.face->num_fixed_sizes; i++)
	  {
	     int s;
	     int d, cd;
	     
	     s = fn->ft.face->available_sizes[i].height;
	     cd = chosen_size - size;
	     if (cd < 0) cd = -cd;
	     d = s - size;
	     if (d < 0) d = -d;
	     if (d < cd)
	       {
		  chosen_width = fn->ft.face->available_sizes[i].width;
		  chosen_size = s;
	       }
	     if (d == 0) break;
	  }
	error = FT_Set_Pixel_Sizes(fn->ft.face, chosen_width, chosen_size);
	if (error)
	  {
	     /* couldn't choose the size anyway... what now? */
	  }
     }

#if 0 /* debugging to look at charmaps in a ttf */
   printf("%i\n", fn->ft.face->num_charmaps);
     {
	int i;
	
	for (i = 0; i < fn->ft.face->num_charmaps; i++)
	  {
	     printf("%i: %x, %c\n", 
		    i, fn->ft.face->charmaps[i]->encoding,
		    fn->ft.face->charmaps[i]->encoding);
	  }
     }
#endif   
   error = FT_Select_Charmap(fn->ft.face, ft_encoding_unicode);
   if (error)
     {
/* disable this for now...
	error = FT_Select_Charmap(fn->ft.face, ft_encoding_latin_2);
	if (error)
	  {
	     error = FT_Select_Charmap(fn->ft.face, ft_encoding_sjis);
	     if (error)
	       {
		  error = FT_Select_Charmap(fn->ft.face, ft_encoding_gb2312);
		  if (error)
		    {
		       error = FT_Select_Charmap(fn->ft.face, ft_encoding_big5);
		       if (error)
			 {
			 }
		    }
	       }
	  }
 */
     }
   
   fn->file = strdup(file);
   fn->name = strdup(file);
   fn->size = size;

   fn->glyphs = NULL;
   
   fn->usage = 0;
   
   fn->references = 1;
   
   fonts = evas_object_list_prepend(fonts, fn);
   return fn;
}

void
evas_common_font_free(RGBA_Font *fn)
{
   fn->references--;
   if (fn->references == 0)
     {
	evas_common_font_modify_cache_by(fn, 1);
	evas_common_font_flush();
     }
}

static int
font_modify_cache_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   int *dir;
   RGBA_Font_Glyph *fg;
   
   fg = data;   
   dir = fdata;
   font_cache_usage += (*dir) * 
     ((fg->glyph_out->bitmap.width * fg->glyph_out->bitmap.rows) +
      sizeof(RGBA_Font_Glyph) + sizeof(Evas_List) + 400); /* fudge values */
   return 1;
   hash = 0;
   key = 0;
}

void
evas_common_font_modify_cache_by(RGBA_Font *fn, int dir)
{
   int sz_name = 0, sz_file = 0, sz_hash = 0;
   
   if (fn->name) sz_name = strlen(fn->name);
   if (fn->file) sz_file = strlen(fn->file);
   if (fn->glyphs) sz_hash = sizeof(Evas_Hash);
   evas_hash_foreach(fn->glyphs, font_modify_cache_cb, &dir);
   font_cache_usage += dir * (sizeof(RGBA_Font) + sz_name + sz_file + sz_hash +
			      sizeof(FT_FaceRec) + 16384); /* fudge values */
}

int
evas_common_font_cache_get(void)
{
   return font_cache;
}

void
evas_common_font_cache_set(int size)
{
   font_cache = size;
   evas_common_font_flush();
}

void
evas_common_font_flush(void)
{
   if (font_cache_usage < font_cache) return;
   while (font_cache_usage > font_cache) evas_common_font_flush_last();
}

static int
font_flush_free_glyph_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   RGBA_Font_Glyph *fg;
   
   fg = data;
   FT_Done_Glyph(fg->glyph);
   free(fg);
   return 1;
   hash = 0;
   key = 0;
   fdata = 0;
}

void
evas_common_font_flush_last(void)
{
   Evas_Object_List *l;
   RGBA_Font *fn = NULL;
   
   for (l = fonts; l; l = l->next)
     {
	RGBA_Font *fn_tmp;
	
	fn_tmp = (RGBA_Font *)l;
	if (fn_tmp->references == 0) fn = fn_tmp;
     }
   if (!fn) return;
   
   fonts = evas_object_list_remove(fonts, fn);
   evas_common_font_modify_cache_by(fn, -1);

   evas_hash_foreach(fn->glyphs, font_flush_free_glyph_cb, NULL);
   evas_hash_free(fn->glyphs);
   
   if (fn->file) free(fn->file);
   if (fn->name) free(fn->name);
   FT_Done_Face(fn->ft.face);
   free(fn);
}

RGBA_Font *
evas_common_font_find(const char *name, int size)
{
   Evas_Object_List *l;
   
   for (l = fonts; l; l = l->next)
     {
	RGBA_Font *fn;
	
	fn = (RGBA_Font *)l;
	if ((fn->size == size) && (!strcmp(name, fn->name)))
	  {
	     if (fn->references == 0) evas_common_font_modify_cache_by(fn, -1);
	     fn->references++;
	     fonts = evas_object_list_remove(fonts, fn);
	     fonts = evas_object_list_prepend(fonts, fn);
	     return fn;
	  }
     }
   return NULL;
}
