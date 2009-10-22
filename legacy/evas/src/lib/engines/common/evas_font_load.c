/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"

#include <assert.h>

extern FT_Library         evas_ft_lib;

static int                font_cache_usage = 0;
static int                font_cache = 0;

static Eina_Hash * fonts_src = NULL;
static Eina_Hash * fonts = NULL;
static Eina_List * fonts_lru = NULL;

static Eina_Bool font_modify_cache_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static Eina_Bool font_flush_free_glyph_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata);

static int
_evas_font_cache_int_cmp(const RGBA_Font_Int *k1, int k1_length __UNUSED__,
			 const RGBA_Font_Int *k2, int k2_length __UNUSED__)
{
   /* RGBA_Font_Source->name is a stringshare */
   if (k1->src->name == k2->src->name)
     return k1->size - k2->size;
   return strcmp(k1->src->name, k2->src->name);;
}

static int
_evas_font_cache_int_hash(const RGBA_Font_Int *key, int key_length __UNUSED__)
{
   int hash;

   hash = eina_hash_djb2(key->src->name, eina_stringshare_strlen(key->src->name) + 1);
   hash ^= eina_hash_int32(&key->size, sizeof (int));

   return hash;
}

static void
_evas_common_font_source_free(RGBA_Font_Source *fs)
{
   FT_Done_Face(fs->ft.face);
#if 0 /* FIXME: Disable as it is only used by dead code using deprecated datatype. */
   if (fs->charmap) evas_array_hash_free(fs->charmap);
#endif
   if (fs->name) eina_stringshare_del(fs->name);
   free(fs);
}


static Eina_Bool
font_flush_free_glyph_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
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

static void
_evas_common_font_int_free(RGBA_Font_Int *fi)
{
   FT_Done_Size(fi->ft.size);

   evas_common_font_int_modify_cache_by(fi, -1);

   eina_hash_foreach(fi->glyphs, font_flush_free_glyph_cb, NULL);
   eina_hash_free(fi->glyphs);

   eina_hash_free(fi->kerning);
   eina_hash_free(fi->indexes);

#ifdef HAVE_PTHREAD
   pthread_mutex_destroy(&fi->ft_mutex);
#endif

   evas_common_font_source_free(fi->src);

   if (fi->references == 0)
     fonts_lru = eina_list_remove(fonts_lru, fi);

   free(fi);
}

void
evas_common_font_load_init(void)
{
   fonts_src = eina_hash_string_small_new(EINA_FREE_CB(_evas_common_font_source_free));
   fonts = eina_hash_new(NULL,
			 EINA_KEY_CMP(_evas_font_cache_int_cmp),
			 EINA_KEY_HASH(_evas_font_cache_int_hash),
			 EINA_FREE_CB(_evas_common_font_int_free),
			 5);
}

void
evas_common_font_load_shutdown(void)
{
   eina_hash_free(fonts);
   fonts = NULL;

   eina_hash_free(fonts_src);
   fonts_src = NULL;
}

EAPI RGBA_Font_Source *
evas_common_font_source_memory_load(const char *name, const void *data, int data_size)
{
   int error;
   RGBA_Font_Source *fs;

   assert(name != NULL);

   fs = calloc(1, sizeof(RGBA_Font_Source) + data_size);
   if (!fs) return NULL;
   fs->data = ((unsigned char *)fs) + sizeof(RGBA_Font_Source);
   fs->data_size = data_size;
   fs->current_size = 0;
   memcpy(fs->data, data, data_size);
   error = FT_New_Memory_Face(evas_ft_lib, fs->data, fs->data_size, 0, &(fs->ft.face));
   if (error)
     {
	free(fs);
	return NULL;
     }
   fs->name = eina_stringshare_add(name);
   fs->file = NULL;
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   fs->ft.orig_upem = fs->ft.face->units_per_EM;
   fs->references = 1;

   eina_hash_direct_add(fonts_src, fs->name, fs);
   return fs;
}

EAPI RGBA_Font_Source *
evas_common_font_source_load(const char *name)
{
   RGBA_Font_Source *fs;

   assert(name != NULL);

   fs = calloc(1, sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->data = NULL;
   fs->data_size = 0;
   fs->current_size = 0;
   fs->ft.face = NULL;

   fs->name = eina_stringshare_add(name);
   fs->file = fs->name;

   fs->ft.orig_upem = 0;

   fs->references = 1;

   eina_hash_direct_add(fonts_src, fs->name, fs);
   return fs;
}

EAPI int
evas_common_font_source_load_complete(RGBA_Font_Source *fs)
{
   int error;

   error = FT_New_Face(evas_ft_lib, fs->file, 0, &(fs->ft.face));
   if (error)
     {
	fs->ft.face = NULL;
	return error;
     }

   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   if (error)
     {
	FT_Done_Face(fs->ft.face);
	fs->ft.face = NULL;
	return error;
     }

   fs->ft.orig_upem = fs->ft.face->units_per_EM;
   return error;
}

EAPI RGBA_Font_Source *
evas_common_font_source_find(const char *name)
{
   RGBA_Font_Source *fs;

   if (!name) return NULL;
   fs = eina_hash_find(fonts_src, name);
   if (fs)
     {
	fs->references++;
	return fs;
     }
   return NULL;
}

EAPI void
evas_common_font_source_free(RGBA_Font_Source *fs)
{
   fs->references--;
   if (fs->references > 0) return;

   eina_hash_del(fonts_src, fs->name, fs);
}

EAPI void
evas_common_font_size_use(RGBA_Font *fn)
{
   RGBA_Font_Int *fi;
   Eina_List *l;

   EINA_LIST_FOREACH(fn->fonts, l, fi)
     {
	if (fi->src->current_size != fi->size)
	  {
	     FT_Activate_Size(fi->ft.size);
	     fi->src->current_size = fi->size;
	  }
     }
}

static int
_evas_common_font_int_cmp(const int *key1, __UNUSED__ int key1_length,
			  const int *key2, __UNUSED__ int key2_length)
{
   return *key1 - *key2;
}

static int
_evas_common_font_double_int_cmp(const int *key1, __UNUSED__ int key1_length,
				 const int *key2, __UNUSED__ int key2_length)
{
   if (key1[0] - key2[0] == 0)
     return key1[1] - key2[1];
   return key1[0] - key2[0];
}

static int
_evas_common_font_double_int_hash(const unsigned int key[2], int key_length)
{
   int tmp;

   tmp = eina_hash_int32(&key[0], key_length);
   tmp ^= eina_hash_int32(&key[1], key_length);

   return tmp;
}

static void
_evas_common_font_int_cache_init(RGBA_Font_Int *fi)
{
   /* Add some font kerning cache. */
   fi->indexes = eina_hash_new(NULL,
			       EINA_KEY_CMP(_evas_common_font_int_cmp),
			       EINA_KEY_HASH(eina_hash_int32),
			       free, 3);
   fi->kerning = eina_hash_new(NULL,
			       EINA_KEY_CMP(_evas_common_font_double_int_cmp),
			       EINA_KEY_HASH(_evas_common_font_double_int_hash),
			       free, 3);
#ifdef HAVE_PTHREAD
   pthread_mutex_init(&fi->ft_mutex, NULL);
#endif
}

EAPI RGBA_Font_Int *
evas_common_font_int_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src)
     fi->src = evas_common_font_source_memory_load(name, data, data_size);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   _evas_common_font_int_cache_init(fi);

   fi = evas_common_font_int_load_init(fi);
   evas_common_font_int_load_complete(fi);

   return fi;
}

EAPI RGBA_Font_Int *
evas_common_font_int_load(const char *name, int size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src && evas_file_path_is_file(name))
     fi->src = evas_common_font_source_load(name);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   _evas_common_font_int_cache_init(fi);

   return evas_common_font_int_load_init(fi);
}

EAPI RGBA_Font_Int *
evas_common_font_int_load_init(RGBA_Font_Int *fi)
{
   fi->ft.size = NULL;
   fi->glyphs = eina_hash_new(NULL,
			      EINA_KEY_CMP(_evas_common_font_int_cmp),
			      EINA_KEY_HASH(eina_hash_int32),
			      NULL,
			      6);
   fi->usage = 0;
   fi->references = 1;

   eina_hash_direct_add(fonts, fi, fi);

   return fi;
}

EAPI RGBA_Font_Int *
evas_common_font_int_load_complete(RGBA_Font_Int *fi)
{
   int val, dv;
   int ret;
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

   fi->max_h = 0;
   
   val = (int)fi->src->ft.face->bbox.yMax;
   if (fi->src->ft.face->units_per_EM != 0)
     {
        dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
        ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
     }
   else
     ret = val;
   fi->max_h += ret;
   
   val = -(int)fi->src->ft.face->bbox.yMin;
   if (fi->src->ft.face->units_per_EM != 0)
     {
        dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
        ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
     }
   else
     ret = val;
   fi->max_h += ret;
   
   return fi;
}

EAPI RGBA_Font *
evas_common_font_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (!fi) return NULL;
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
	free(fi);
	return NULL;
     }
   fn->fonts = eina_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   fn->references = 1;
   LKI(fn->lock);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_load(const char *name, int size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_load(name, size);
   if (!fi) return NULL;

   /* First font, complete load */
   if (!fi->ft.size)
     {
	if (!fi->src->ft.face)
	  {
	     if (evas_common_font_source_load_complete(fi->src))
	       {
		  fi->references--;
		  if (fi->references == 0)
		    {
		       fonts_lru = eina_list_prepend(fonts_lru, fi);
		       evas_common_font_int_modify_cache_by(fi, 1);
		       evas_common_font_flush();
		    }
		  return NULL;
	       }
	  }
	evas_common_font_int_load_complete(fi);
     }

   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
	fi->references--;
	if (fi->references == 0)
	  {
	     fonts_lru = eina_list_prepend(fonts_lru, fi);
	     evas_common_font_int_modify_cache_by(fi, 1);
	     evas_common_font_flush();
	  }
	return NULL;
     }
   fn->fonts = eina_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   fn->references = 1;
   LKI(fn->lock);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_add(RGBA_Font *fn, const char *name, int size)
{
   RGBA_Font_Int *fi;

   if (!fn)
      return NULL;
   fi = evas_common_font_int_load(name, size);
   if (fi)
     {
	fn->fonts = eina_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
	return fn;
     }
   return NULL;
}

EAPI RGBA_Font *
evas_common_font_memory_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   if (!fn)
      return NULL;
   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (fi)
     {
	fn->fonts = eina_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
	return fn;
     }
   return NULL;
}

EAPI void
evas_common_font_free(RGBA_Font *fn)
{
   Eina_List *l;
   RGBA_Font_Int *fi;

   if (!fn)
      return;
   fn->references--;
   if (fn->references > 0) return;
   EINA_LIST_FOREACH(fn->fonts, l, fi)
     {
	fi->references--;
	if (fi->references == 0)
	  {
	     fonts_lru = eina_list_append(fonts_lru, fi);
	     evas_common_font_int_modify_cache_by(fi, 1);
	  }
     }
   evas_common_font_flush();
   eina_list_free(fn->fonts);
   LKD(fn->lock);
   free(fn);
}

EAPI void
evas_common_font_hinting_set(RGBA_Font *fn, Font_Hint_Flags hinting)
{
   Eina_List *l;
   RGBA_Font_Int *fi;

   if (!fn)
     return;
   fn->hinting = hinting;
   EINA_LIST_FOREACH(fn->fonts, l, fi)
     fi->hinting = fn->hinting;
}

EAPI Eina_Bool
evas_common_hinting_available(Font_Hint_Flags hinting)
{
   switch (hinting)
     {
      case FONT_NO_HINT:
      case FONT_AUTO_HINT:
	 /* these two hinting modes are always available */
	 return EINA_TRUE;
      case FONT_BYTECODE_HINT:
	 /* Only use the bytecode interpreter if support for the _patented_
	  * algorithms is available because the free bytecode
	  * interpreter's results are too crappy.
	  *
	  * On freetyp 2.2+, we can ask the library about support for
	  * the patented interpreter. On older versions, we need to use
	  * macros to check for it.
	  */
#if FREETYPE_MINOR >= 2
	 return FT_Get_TrueType_Engine_Type(evas_ft_lib) >=
		FT_TRUETYPE_ENGINE_TYPE_PATENTED;
#else
	 /* we may not rely on TT_CONFIG_OPTION_BYTECODE_INTERPRETER
	  * here to find out whether it's supported.
	  *
	  * so, assume it is. o_O
	  */
	 return EINA_TRUE;
#endif
     }

   /* shouldn't get here - need to add another case statement */
   return EINA_FALSE;
}

EAPI RGBA_Font *
evas_common_font_memory_hinting_load(const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting)
{
   RGBA_Font *fn;

   fn = evas_common_font_memory_load(name, size, data, data_size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_hinting_load(const char *name, int size, Font_Hint_Flags hinting)
{
   RGBA_Font *fn;

   fn = evas_common_font_load(name, size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_hinting_add(RGBA_Font *fn, const char *name, int size, Font_Hint_Flags hinting)
{
   fn = evas_common_font_add(fn, name, size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_memory_hinting_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting)
{
   fn = evas_common_font_memory_add(fn, name, size, data, data_size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

static Eina_Bool
font_modify_cache_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   int *dir;
   RGBA_Font_Glyph *fg;

   fg = data;
   dir = fdata;
   font_cache_usage += (*dir) *
     ((fg->glyph_out->bitmap.width * fg->glyph_out->bitmap.rows) +
      sizeof(RGBA_Font_Glyph) + sizeof(Eina_List) + 400); /* fudge values */
   return 1;
   hash = 0;
   key = 0;
}

/* when the fi->references == 0 we increase this instead of really deleting
 * we then check if the cache_useage size is larger than allowed
 * !If the cache is NOT too large we dont delete font_int
 * !If the cache is too large we really delete font_int */
EAPI void
evas_common_font_int_modify_cache_by(RGBA_Font_Int *fi, int dir)
{
   int sz_hash = 0;

   if (fi->glyphs) sz_hash = eina_hash_population(fi->glyphs);
   eina_hash_foreach(fi->glyphs, font_modify_cache_cb, &dir);
   font_cache_usage += dir * (sizeof(RGBA_Font) + sz_hash +
			      sizeof(FT_FaceRec) + 16384); /* fudge values */
}

EAPI int
evas_common_font_cache_get(void)
{
   return font_cache;
}

EAPI void
evas_common_font_cache_set(int size)
{
   font_cache = size;
   evas_common_font_flush();
}

EAPI void
evas_common_font_flush(void)
{
   if (font_cache_usage < font_cache) return;
   while (font_cache_usage > font_cache) evas_common_font_flush_last();
}

/* We run this when the cache gets larger than allowed size
 * We check cache size each time a fi->references goes to 0
 * PERFORMS: Find font_int(s) with references == 0 and delete them */
EAPI void
evas_common_font_flush_last(void)
{
   RGBA_Font_Int *fi = NULL;

   if (!fonts_lru) return ;

   fi = eina_list_data_get(fonts_lru);
   fonts_lru = eina_list_remove_list(fonts_lru, fonts_lru);

   eina_hash_del(fonts, fi, fi);
}

EAPI RGBA_Font_Int *
evas_common_font_int_find(const char *name, int size)
{
   RGBA_Font_Int tmp_fi;
   RGBA_Font_Source tmp_fn;
   RGBA_Font_Int *fi;

   tmp_fn.name = (char*) eina_stringshare_add(name);
   tmp_fi.src = &tmp_fn;
   tmp_fi.size = size;

   fi = eina_hash_find(fonts, &tmp_fi);
   if (fi)
     {
	if (fi->references == 0)
	  {
	     evas_common_font_int_modify_cache_by(fi, -1);
	     fonts_lru = eina_list_remove(fonts_lru, fi);
	  }
	fi->references++;
     }

   eina_stringshare_del(tmp_fn.name);
   return fi;
}
