#include "evas_common.h"

extern FT_Library         evas_ft_lib;

static int                font_cache_usage = 0;
static int                font_cache = 0;
static Evas_Object_List * fonts_src = NULL;
static Evas_Object_List * fonts = NULL;

static int font_modify_cache_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
static int font_flush_free_glyph_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

RGBA_Font_Source *
evas_common_font_source_memory_load(const char *name, const void *data, int data_size)
{
   int error;
   RGBA_Font_Source *fs;
   
   fs = malloc(sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->name = strdup(name);
   fs->file = NULL;
   fs->data = malloc(data_size);
   if (!fs->data)
     {
	if (fs->name) free(fs->name);
	free(fs);
	return NULL;
     }
   memcpy(fs->data, data, data_size);
   fs->data_size = data_size;
   error = FT_New_Memory_Face(evas_ft_lib, fs->data, fs->data_size, 0, &(fs->ft.face));
   if (error)
     {
	if (fs->name) free(fs->name);
	if (fs->data) free(fs->data);
	free(fs);
	return NULL;
     }
   
   fs->references = 1;
   
   fonts_src = evas_object_list_prepend(fonts_src, fs);   
   return fs;
}

RGBA_Font_Source *
evas_common_font_source_load(const char *name)
{
   int error;
   RGBA_Font_Source *fs;
   
   fs = malloc(sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->name = strdup(name);
   fs->file = strdup(name);
   error = FT_New_Face(evas_ft_lib, fs->file, 0, &(fs->ft.face));
   if (error)
     {
	if (fs->name) free(fs->name);
	if (fs->file) free(fs->file);
	free(fs);
	return NULL;
     }
   
   fs->references = 1;
   
   fonts_src = evas_object_list_prepend(fonts_src, fs);   
   return fs;
}

RGBA_Font_Source *
evas_common_font_source_find(const char *name)
{
   Evas_Object_List *l;
   
   if (!name) return NULL;
   for (l = fonts_src; l; l = l->next)
     {
	RGBA_Font_Source *fs;
	
	fs = (RGBA_Font_Source *)l;
	if ((fs->name) && (!strcmp(name, fs->name)))
	  {
	     fs->references++;	     
	     fonts_src = evas_object_list_remove(fonts_src, fs);
	     fonts_src = evas_object_list_prepend(fonts_src, fs);	     
	     return fs;
	  }
     }
   return NULL;
}

void
evas_common_font_source_free(RGBA_Font_Source *fs)
{
   fs->references--;
   if (fs->references > 0) return;
   
   fonts_src = evas_object_list_remove(fonts_src, fs);
   if (fs->name) free(fs->name);
   if (fs->file) free(fs->file);
   if (fs->data) free(fs->data);
   FT_Done_Face(fs->ft.face);
   free(fs);
}

void
evas_common_font_size_use(RGBA_Font *fn)
{
   if (fn->src->current_size == fn->real_size) return;
   FT_Set_Char_Size(fn->src->ft.face, 0, fn->real_size, 96, 96);
   fn->src->current_size = fn->real_size;
}

RGBA_Font *
evas_common_font_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font *fn;

   fn = evas_common_font_find(name, size);
   if (fn) return fn;
   
   fn = malloc(sizeof(RGBA_Font));   
   if (!fn) return NULL;
   
   fn->src = evas_common_font_source_find(name);
   if (!fn->src) fn->src = evas_common_font_source_memory_load(name, data, data_size);

   if (!fn->src)
     {
	free(fn);
	return NULL;
     }
   
   fn->size = size;

   return evas_common_font_load_init(fn);
}

RGBA_Font *
evas_common_font_load(const char *name, int size)
{
   RGBA_Font *fn;

   fn = evas_common_font_find(name, size);
   if (fn) return fn;
   
   fn = malloc(sizeof(RGBA_Font));   
   if (!fn) return NULL;
   
   fn->src = evas_common_font_source_find(name);
   if (!fn->src) fn->src = evas_common_font_source_load(name);

   if (!fn->src)
     {
	free(fn);
	return NULL;
     }
   
   fn->size = size;

   return evas_common_font_load_init(fn);
}

RGBA_Font *
evas_common_font_load_init(RGBA_Font *fn)
{
   int error;
   
   fn->real_size = fn->size * 64;
   error = FT_Set_Char_Size(fn->src->ft.face, 0, (fn->size * 64), 96, 96);
   if (error)
     {
	error = FT_Set_Pixel_Sizes(fn->src->ft.face, 0, fn->size);
	fn->real_size = fn->size;
     }
   if (error)
     {
	int i;
	int chosen_size = 0;
	int chosen_width = 0;

	for (i = 0; i < fn->src->ft.face->num_fixed_sizes; i++)
	  {
	     int s;
	     int d, cd;
	     
	     s = fn->src->ft.face->available_sizes[i].height;
	     cd = chosen_size - fn->size;
	     if (cd < 0) cd = -cd;
	     d = s - fn->size;
	     if (d < 0) d = -d;
	     if (d < cd)
	       {
		  chosen_width = fn->src->ft.face->available_sizes[i].width;
		  chosen_size = s;
	       }
	     if (d == 0) break;
	  }
	error = FT_Set_Pixel_Sizes(fn->src->ft.face, chosen_width, chosen_size);
	if (error)
	  {
	     /* couldn't choose the size anyway... what now? */
	  }
	fn->real_size = chosen_size;
     }
   fn->src->current_size = fn->real_size;

#if 0 /* debugging to look at charmaps in a ttf */
   printf("%i\n", fn->src->ft.face->num_charmaps);
     {
	int i;
	
	for (i = 0; i < fn->src->ft.face->num_charmaps; i++)
	  {
	     printf("%i: %x, %c\n", 
		    i, fn->src->ft.face->charmaps[i]->encoding,
		    fn->src->ft.face->charmaps[i]->encoding);
	  }
     }
#endif   
   error = FT_Select_Charmap(fn->src->ft.face, ft_encoding_unicode);
   if (error)
     {
/* disable this for now...
	error = FT_Select_Charmap(fn->src->ft.face, ft_encoding_latin_2);
	if (error)
	  {
	     error = FT_Select_Charmap(fn->src->ft.face, ft_encoding_sjis);
	     if (error)
	       {
		  error = FT_Select_Charmap(fn->src->ft.face, ft_encoding_gb2312);
		  if (error)
		    {
		       error = FT_Select_Charmap(fn->src->ft.face, ft_encoding_big5);
		       if (error)
			 {
			 }
		    }
	       }
	  }
 */
     }
   
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
   int sz_hash = 0;
   
   if (fn->glyphs) sz_hash = sizeof(Evas_Hash);
   evas_hash_foreach(fn->glyphs, font_modify_cache_cb, &dir);
   font_cache_usage += dir * (sizeof(RGBA_Font) + sz_hash +
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
   /* extension calls */
   if (fg->ext_dat_free) fg->ext_dat_free(fg->ext_dat);
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
   
   evas_common_font_source_free(fn->src);
   
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
	if ((fn->size == size) && (!strcmp(name, fn->src->name)))
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
