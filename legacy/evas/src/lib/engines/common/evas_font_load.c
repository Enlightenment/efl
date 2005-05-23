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

   fs = calloc(1, sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->name = strdup(name);
   fs->file = NULL;
   fs->data = malloc(data_size);
   fs->current_size = 0;
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

   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);

   fs->ft.orig_upem = fs->ft.face->units_per_EM;

   fs->references = 1;

   fonts_src = evas_object_list_prepend(fonts_src, fs);
   return fs;
}

RGBA_Font_Source *
evas_common_font_source_load(const char *name)
{
   int error;
   RGBA_Font_Source *fs;

   fs = calloc(1, sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->name = strdup(name);
   fs->file = strdup(name);
   fs->data = NULL;
   fs->data_size = 0;
   fs->current_size = 0;
   error = FT_New_Face(evas_ft_lib, fs->file, 0, &(fs->ft.face));
   if (error)
     {
	if (fs->name) free(fs->name);
	if (fs->file) free(fs->file);
	free(fs);
	return NULL;
     }
#if 0 /* debugging to look at charmaps in a ttf */
   printf("charmaps [%s]: %i\n", name, fs->ft.face->num_charmaps);
     {
	int i;

	for (i = 0; i < fs->ft.face->num_charmaps; i++)
	  {
	     printf("  %i: %x, ",
		    i, fs->ft.face->charmaps[i]->encoding);
	     if      (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_NONE)           printf("none\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_UNICODE)        printf("unicode\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_MS_SYMBOL)      printf("ms_symbol\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_SJIS)           printf("sjis\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_GB2312)         printf("gb3212\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_BIG5)           printf("big5\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_WANSUNG)        printf("wansung\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_JOHAB)          printf("johab\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_ADOBE_LATIN_1)  printf("adobe_latin_1\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_ADOBE_STANDARD) printf("adobe_standard\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_ADOBE_EXPERT)   printf("adobe_expert\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_ADOBE_CUSTOM)   printf("adobe_custom\n");
	     else if (fs->ft.face->charmaps[i]->encoding == FT_ENCODING_APPLE_ROMAN)    printf("apple_roman\n");
	     else                                                                       printf("UNKNOWN");
	     printf("\n");
	  }
     }
#endif
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   if (error)
     {
	printf("cant select unicode!\n");
/* disable this for now...
	error = FT_Select_Charmap(fs->ft.face, ft_encoding_latin_2);
	if (error)
	  {
	     error = FT_Select_Charmap(fs->ft.face, ft_encoding_sjis);
	     if (error)
	       {
		  error = FT_Select_Charmap(fs->ft.face, ft_encoding_gb2312);
		  if (error)
		    {
		       error = FT_Select_Charmap(fs->ft.face, ft_encoding_big5);
		       if (error)
			 {
			 }
		    }
	       }
	  }
 */
     }

   fs->ft.orig_upem = fs->ft.face->units_per_EM;

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
   Evas_List *l;

   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = l->data;
	if (fi->src->current_size != fi->size)
	  {
	     FT_Activate_Size(fi->ft.size);
	     fi->src->current_size = fi->size;
	  }
     }
}

RGBA_Font_Int *
evas_common_font_int_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src) fi->src = evas_common_font_source_memory_load(name, data, data_size);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   return evas_common_font_int_load_init(fi);
}

RGBA_Font_Int *
evas_common_font_int_load(const char *name, int size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src)
     {
/*	printf("REAL LOAD FILE %s %i\n", name, size);*/
	fi->src = evas_common_font_source_load(name);
     }
/*   else*/
/*     printf("REAL LOAD SIZE %s %i\n", name, size);*/

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   return evas_common_font_int_load_init(fi);
}

RGBA_Font_Int *
evas_common_font_int_load_init(RGBA_Font_Int *fi)
{
   int error;

   error = FT_New_Size(fi->src->ft.face, &(fi->ft.size));
   if (!error)
     {
	FT_Activate_Size(fi->ft.size);
     }
   fi->real_size = fi->size * 64;
   error = FT_Set_Char_Size(fi->src->ft.face, 0, fi->real_size, 75, 75);
   if (error)
     {
	fi->real_size = fi->size;
	error = FT_Set_Pixel_Sizes(fi->src->ft.face, 0, fi->real_size);
     }
   if (error)
     {
	int i;
	int chosen_size = 0;
	int chosen_width = 0;

	for (i = 0; i < fi->src->ft.face->num_fixed_sizes; i++)
	  {
	     int s;
	     int d, cd;

	     s = fi->src->ft.face->available_sizes[i].height;
	     cd = chosen_size - fi->size;
	     if (cd < 0) cd = -cd;
	     d = s - fi->size;
	     if (d < 0) d = -d;
	     if (d < cd)
	       {
		  chosen_width = fi->src->ft.face->available_sizes[i].width;
		  chosen_size = s;
	       }
	     if (d == 0) break;
	  }
	fi->real_size = chosen_size;
	error = FT_Set_Pixel_Sizes(fi->src->ft.face, chosen_width, fi->real_size);
	if (error)
	  {
	     /* couldn't choose the size anyway... what now? */
	  }
     }
   fi->src->current_size = fi->size;

   fi->glyphs = NULL;
   fi->usage = 0;
   fi->references = 1;
   fonts = evas_object_list_prepend(fonts, fi);
   return fi;
}

RGBA_Font *
evas_common_font_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (!fi) return NULL;
//   printf("LOAD FONT MEM  %s %i\n", name, size);
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn) return NULL;
   fn->fonts = evas_list_append(fn->fonts, fi);
   return fn;
}

RGBA_Font *
evas_common_font_load(const char *name, int size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_load(name, size);
   if (!fi) return NULL;
//   printf("LOAD FONT FILE  %s %i\n", name, size);
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn) return NULL;
   fn->fonts = evas_list_append(fn->fonts, fi);
   return fn;
}

RGBA_Font *
evas_common_font_add(RGBA_Font *fn, const char *name, int size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_load(name, size);
   if (fi)
     {
	fn->fonts = evas_list_append(fn->fonts, fi);
	return fn;
     }
   return NULL;
}

RGBA_Font *
evas_common_font_memory_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (fi)
     {
	fn->fonts = evas_list_append(fn->fonts, fi);
	return fn;
     }
   return NULL;
}

void
evas_common_font_free(RGBA_Font *fn)
{
   Evas_List *l;

   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = l->data;
	fi->references--;
	if (fi->references == 0)
	  {
	     evas_common_font_int_modify_cache_by(fi, 1);
	     evas_common_font_flush();
	  }
     }
   evas_list_free(fn->fonts);
   free(fn);
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
evas_common_font_int_modify_cache_by(RGBA_Font_Int *fi, int dir)
{
   int sz_hash = 0;

   if (fi->glyphs) sz_hash = sizeof(Evas_Hash);
   evas_hash_foreach(fi->glyphs, font_modify_cache_cb, &dir);
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
   RGBA_Font_Int *fi = NULL;

   for (l = fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi_tmp;

	fi_tmp = (RGBA_Font_Int *)l;
	if (fi_tmp->references == 0) fi = fi_tmp;
     }
   if (!fi) return;

   FT_Done_Size(fi->ft.size);

   fonts = evas_object_list_remove(fonts, fi);
   evas_common_font_int_modify_cache_by(fi, -1);

   evas_hash_foreach(fi->glyphs, font_flush_free_glyph_cb, NULL);
   evas_hash_free(fi->glyphs);

   evas_common_font_source_free(fi->src);

   free(fi);
}

RGBA_Font_Int *
evas_common_font_int_find(const char *name, int size)
{
   Evas_Object_List *l;

//   printf("SEARCH!\n");
   for (l = fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = (RGBA_Font_Int *)l;
//	printf("%s == %s, %i == %i\n", name, fi->src->name, size, fi->size);
	if ((fi->size == size) && (!strcmp(name, fi->src->name)))
	  {
	     if (fi->references == 0) evas_common_font_int_modify_cache_by(fi, -1);
	     fi->references++;
	     fonts = evas_object_list_remove(fonts, fi);
	     fonts = evas_object_list_prepend(fonts, fi);
	     return fi;
	  }
     }
   return NULL;
}
