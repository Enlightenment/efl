#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include "evas_font_ot.h"

#ifdef USE_HARFBUZZ
# include <hb.h>
#endif

#include "evas_font_private.h" /* for Frame-Queuing support */

#include <ft2build.h>
#include FT_TRUETYPE_TABLES_H /* Freetype2 OS/2 font table. */

extern FT_Library         evas_ft_lib;

static int                font_cache_usage = 0;
static int                font_cache = 0;
static int                font_dpi_h = 75;
static int                font_dpi_v = 75;

static Eina_Hash   *fonts_src = NULL;
static Eina_Hash   *fonts = NULL;
static Eina_List   *fonts_lru = NULL;
static Eina_Inlist *fonts_use_lru = NULL;
static int          fonts_use_usage = 0;

static void _evas_common_font_int_clear(RGBA_Font_Int *fi);

static int
_evas_font_cache_int_cmp(const RGBA_Font_Int *k1, int k1_length EINA_UNUSED,
			 const RGBA_Font_Int *k2, int k2_length EINA_UNUSED)
{
   /* RGBA_Font_Source->name is a stringshare */
   if (k1->src->name == k2->src->name)
     {
        if (k1->size == k2->size)
          {
             if (k1->wanted_rend == k2->wanted_rend)
               return k1->bitmap_scalable - k2->bitmap_scalable;
             else
               return k1->wanted_rend - k2->wanted_rend;
          }
        else
          {
             return k1->size - k2->size;
          }
     }
   return strcmp(k1->src->name, k2->src->name);
}

static int
_evas_font_cache_int_hash(const RGBA_Font_Int *key, int key_length EINA_UNUSED)
{
   int hash;
   unsigned int wanted_rend = key->wanted_rend;
   hash = eina_hash_djb2(key->src->name, eina_stringshare_strlen(key->src->name) + 1);
   hash ^= eina_hash_int32(&key->size, sizeof (int));
   hash ^= eina_hash_int32(&wanted_rend, sizeof (int));
   hash ^= eina_hash_int32(&key->bitmap_scalable, sizeof (int));
   return hash;
}

static void
_evas_common_font_source_free(RGBA_Font_Source *fs)
{
   FTLOCK();
   FT_Done_Face(fs->ft.face);
   FTUNLOCK();
   if (fs->name) eina_stringshare_del(fs->name);
   if (fs->file) eina_stringshare_del(fs->file);
   free(fs);
}

static void
_evas_common_font_int_free(RGBA_Font_Int *fi)
{
   FTLOCK();
   FT_Done_Size(fi->ft.size);
   FTUNLOCK();

   evas_common_font_int_modify_cache_by(fi, -1);
   _evas_common_font_int_clear(fi);
   eina_hash_free(fi->kerning);

   LKD(fi->ft_mutex);
#ifdef USE_HARFBUZZ
   hb_font_destroy(fi->ft.hb_font);
#endif
   evas_common_font_source_free(fi->src);
   if (fi->references <= 0) fonts_lru = eina_list_remove(fonts_lru, fi);
   if (fi->fash) fi->fash->freeme(fi->fash);
   if (fi->inuse)
    {
      fonts_use_lru = eina_inlist_remove(fonts_use_lru, EINA_INLIST_GET(fi));
      fi->inuse = 0;
      fonts_use_usage -= fi->usage;
      fi->usage = 0;
    }
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

EAPI void
evas_common_font_dpi_set(int dpi_h, int dpi_v)
{
   if (dpi_v <= 0) dpi_v = dpi_h;
   font_dpi_h = dpi_h;
   font_dpi_v = dpi_v;
}

EAPI RGBA_Font_Source *
evas_common_font_source_memory_load(const char *name, const void *data, int data_size)
{
   int error;
   RGBA_Font_Source *fs;

   assert(name != NULL);
   if (data_size <= 0) return NULL;
   fs = calloc(1, sizeof(RGBA_Font_Source) + data_size);
   if (!fs) return NULL;
   fs->data = ((unsigned char *)fs) + sizeof(RGBA_Font_Source);
   fs->data_size = data_size;
   fs->current_size = 0;
   memcpy(fs->data, data, data_size);
   FTLOCK();
   error = FT_New_Memory_Face(evas_ft_lib, fs->data, fs->data_size, 0, &(fs->ft.face));
   FTUNLOCK();
   if (error)
     {
	free(fs);
	return NULL;
     }
   fs->name = eina_stringshare_add(name);
   fs->file = NULL;
   FTLOCK();
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);

   if (error)
     {
        FT_Done_Face(fs->ft.face);
        FTUNLOCK();
        fs->ft.face = NULL;
        free(fs);
        return NULL;
     }

   FTUNLOCK();
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
   fs->file = eina_stringshare_ref(fs->name);
   fs->ft.orig_upem = 0;
   fs->references = 1;
   eina_hash_direct_add(fonts_src, fs->name, fs);
   return fs;
}

void
evas_common_font_source_unload(RGBA_Font_Source *fs)
{
   FTLOCK();
   FT_Done_Face(fs->ft.face);
   fs->ft.face = NULL;
   FTUNLOCK();
}

void
evas_common_font_source_reload(RGBA_Font_Source *fs)
{
  if (fs->ft.face) return;
  if (fs->data)
    {
      int error;

      FTLOCK();
      error = FT_New_Memory_Face(evas_ft_lib, fs->data, fs->data_size, 0, &(fs->ft.face));
      FTUNLOCK();
      if (error) return;
      FTLOCK();
      error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
      if (error)
        {
          FT_Done_Face(fs->ft.face);
          fs->ft.face = NULL;
        }
      FTUNLOCK();
    }
  else
    evas_common_font_source_load_complete(fs);
}

EAPI int
evas_common_font_source_load_complete(RGBA_Font_Source *fs)
{
   int error;

   FTLOCK();
   error = FT_New_Face(evas_ft_lib, fs->file, 0, &(fs->ft.face));
   if (error)
     {
        FTUNLOCK();
	fs->ft.face = NULL;
	return error;
     }
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   if (error)
     {
	FT_Done_Face(fs->ft.face);
        FTUNLOCK();
	fs->ft.face = NULL;
	return error;
     }
   FTUNLOCK();
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
   fs->current_size = 0;
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
             evas_common_font_source_reload(fi->src);
             FTLOCK();
	     FT_Activate_Size(fi->ft.size);
             FTUNLOCK();
	     fi->src->current_size = fi->size;
	  }
     }
}

static int
_evas_common_font_double_int_cmp(const int *key1, EINA_UNUSED int key1_length,
				 const int *key2, EINA_UNUSED int key2_length)
{
   if (key1[0] - key2[0] == 0) return key1[1] - key2[1];
   return key1[0] - key2[0];
}

static int
_evas_common_font_double_int_hash(const unsigned int key[2], int key_length)
{
   return
    eina_hash_int32(&key[0], key_length) ^
    eina_hash_int32(&key[1], key_length);
}

static void
_evas_common_font_int_cache_init(RGBA_Font_Int *fi)
{
   /* Add some font kerning cache. */
  fi->kerning = eina_hash_new(NULL,
			       EINA_KEY_CMP(_evas_common_font_double_int_cmp),
			       EINA_KEY_HASH(_evas_common_font_double_int_hash),
			       free, 3);
   LKI(fi->ft_mutex);
}

EAPI RGBA_Font_Int *
evas_common_font_int_memory_load(const char *source, const char *name, int size, const void *data, int data_size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font_Int *fi;
   char fake_name[PATH_MAX];

   eina_file_path_join(fake_name, sizeof(fake_name), source, name);
   fi = evas_common_font_int_find(fake_name, size, wanted_rend, bitmap_scalable);
   if (fi)
     {
        return fi;
     }
   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi)
     {
        return NULL;
     }
   fi->src = evas_common_font_source_find(fake_name);
   if (!fi->src)
    fi->src = evas_common_font_source_memory_load(fake_name, data, data_size);
   if (!fi->src)
     {
        free(fi);
        return NULL;
     }
   fi->size = size;
   fi->bitmap_scalable = bitmap_scalable;
   _evas_common_font_int_cache_init(fi);
   fi = evas_common_font_int_load_init(fi);
   evas_common_font_int_load_complete(fi);
   return fi;
}

static int
_file_path_is_file_helper(const char *path)
{
   struct stat st;

   if (stat(path, &st) == -1) return 0;
   if (S_ISREG(st.st_mode)) return 1;
   return 0;
}

EAPI RGBA_Font_Int *
evas_common_font_int_load(const char *name, int size,
                          Font_Rend_Flags wanted_rend,
						  Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size, wanted_rend, bitmap_scalable);
   if (fi) return fi;
   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;
   fi->src = evas_common_font_source_find(name);
   if (!fi->src && _file_path_is_file_helper(name))
     fi->src = evas_common_font_source_load(name);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }
   fi->size = size;
   fi->wanted_rend = wanted_rend;
   fi->bitmap_scalable = bitmap_scalable;
   _evas_common_font_int_cache_init(fi);
   fi = evas_common_font_int_load_init(fi);
//   evas_common_font_int_load_complete(fi);
   return fi;
}

EAPI RGBA_Font_Int *
evas_common_font_int_load_init(RGBA_Font_Int *fi)
{
   fi->ft.size = NULL;
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

   FTLOCK();
   error = FT_New_Size(fi->src->ft.face, &(fi->ft.size));
   if (!error)
     {
	FT_Activate_Size(fi->ft.size);
     }
   fi->real_size = fi->size * 64;
   fi->scale_factor = 1.0;
   error = FT_Set_Char_Size(fi->src->ft.face, 0, fi->real_size, font_dpi_h, font_dpi_v);
   if (error)
     error = FT_Set_Pixel_Sizes(fi->src->ft.face, 0, fi->real_size);
   FTUNLOCK();
   if (error)
     {
	int i, maxd = 0x7fffffff;
	int chosen_size = 0;
	int chosen_size2 = 0;
        FT_Int strike_index = 0;

	for (i = 0; i < fi->src->ft.face->num_fixed_sizes; i++)
	  {
	     int s, cd;

	     s = fi->src->ft.face->available_sizes[i].size;
	     cd = chosen_size - fi->real_size;
	     if (cd < 0) cd = -cd;
             if (cd < maxd)
               {
                  maxd = cd;
		  chosen_size = s;
		  chosen_size2 = fi->src->ft.face->available_sizes[i].y_ppem;
                  strike_index = (FT_Int)i;
                  if (maxd == 0) break;
	       }
	  }
	fi->real_size = chosen_size;
        FTLOCK();

        if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
          {
             error = FT_Select_Size(fi->src->ft.face, strike_index);

             if (!error)
               {
                  if (FT_HAS_COLOR(fi->src->ft.face) &&
                      fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
                    {
                       if (fi->real_size > 0)
                         fi->scale_factor = (float)fi->size * 64.0 / (float)fi->real_size;
                       else
                         fi->scale_factor = (float)64.0;
                    }
               }
          }
        else
          {
             error = FT_Set_Pixel_Sizes(fi->src->ft.face, 0, fi->real_size);
          }

        FTUNLOCK();
	if (error)
	  {
             error = FT_Set_Char_Size(fi->src->ft.face, 0, fi->real_size, font_dpi_h, font_dpi_v);
             if (error)
               {
                  /* hack around broken fonts */
                  fi->real_size = (chosen_size2 / 64) * 60;
                  error = FT_Set_Char_Size(fi->src->ft.face, 0, fi->real_size, font_dpi_h, font_dpi_v);
                  if (error)
                    {
                       /* couldn't choose the size anyway... what now? */
                    }
               }
	  }
     }
   fi->src->current_size = 0;
   fi->max_h = 0;
   val = (int)fi->src->ft.face->bbox.yMax;
   if (fi->src->ft.face->units_per_EM != 0)
     {
        dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
        ret = FONT_METRIC_CONV(val, dv, fi->src->ft.face->size->metrics.y_scale);
     }
   else
     {
        if ((fi->src->ft.face->bbox.yMax == 0) &&
            (fi->src->ft.face->bbox.yMin == 0))
          ret = FONT_METRIC_ROUNDUP((int)fi->ft.size->metrics.ascender);
        else
          ret = val;
     }
   fi->max_h += ret;
   val = -(int)fi->src->ft.face->bbox.yMin;
   if (fi->src->ft.face->units_per_EM != 0)
     {
        dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
        ret = FONT_METRIC_CONV(val, dv, fi->src->ft.face->size->metrics.y_scale);
     }
   else
     {
        if ((fi->src->ft.face->bbox.yMax == 0) &&
            (fi->src->ft.face->bbox.yMin == 0))
          ret = FONT_METRIC_ROUNDUP(-(int)fi->ft.size->metrics.descender);
        else
          ret = val;
     }
   fi->max_h += ret;

   /* If the loaded font doesn't match with wanted_rend value requested by
    * textobject and textblock, Set the runtime_rend value as FONT_REND_SLANT
    * or FONT_REND_WEIGHT for software rendering. */
   fi->runtime_rend = FONT_REND_REGULAR;
   if ((fi->wanted_rend & FONT_REND_SLANT) &&
       !(fi->src->ft.face->style_flags & FT_STYLE_FLAG_ITALIC))
      fi->runtime_rend |= FONT_REND_SLANT;

   if ((fi->wanted_rend & FONT_REND_WEIGHT) &&
       !(fi->src->ft.face->style_flags & FT_STYLE_FLAG_BOLD))
     {
        TT_OS2 *tt_os2 = FT_Get_Sfnt_Table(fi->src->ft.face, ft_sfnt_os2);
        if (!tt_os2 || (tt_os2->usWeightClass < 600))
          {
             fi->runtime_rend |= FONT_REND_WEIGHT;
          }
     }

   return fi;
}

EAPI RGBA_Font *
evas_common_font_memory_load(const char *source, const char *name, int size, const void *data, int data_size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_memory_load(source, name, size, data, data_size,
                                         wanted_rend, bitmap_scalable);
   if (!fi) return NULL;
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
        evas_common_font_int_unref(fi);
	return NULL;
     }
   fn->fonts = eina_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   fn->references = 1;
   LKI(fn->lock);
   if (fi->inuse) evas_common_font_int_promote(fi);
   else
    {
      fi->inuse = 1;
      fonts_use_lru = eina_inlist_prepend(fonts_use_lru, EINA_INLIST_GET(fi));
    }
   return fn;
}


//ZZZ: font struct looks like:
// fn->(fi, fi, fi, ...)
//   fi->fs

EAPI RGBA_Font *
evas_common_font_load(const char *name, int size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_load(name, size, wanted_rend, bitmap_scalable);
   if (!fi) return NULL;
   /* First font, complete load */
   if (!fi->ft.size)
     {
	if (!fi->src->ft.face)
	  {
	     if (evas_common_font_source_load_complete(fi->src))
	       {
                  evas_common_font_int_unref(fi);
		  return NULL;
	       }
	  }
	evas_common_font_int_load_complete(fi);
     }
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
        evas_common_font_int_unref(fi);
	return NULL;
     }

   fn->fonts = eina_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   fn->references = 1;
   LKI(fn->lock);
   if (fi->inuse) evas_common_font_int_promote(fi);
   else
    {
      fi->inuse = 1;
      fonts_use_lru = eina_inlist_prepend(fonts_use_lru, EINA_INLIST_GET(fi));
    }
   return fn;
}

EAPI RGBA_Font *
evas_common_font_add(RGBA_Font *fn, const char *name, int size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font_Int *fi;

   if (!fn) return NULL;
   fi = evas_common_font_int_load(name, size, wanted_rend, bitmap_scalable);
   if (fi)
     {
	fn->fonts = eina_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
        if (fi->inuse) evas_common_font_int_promote(fi);
        else
         {
           fi->inuse = 1;
           fonts_use_lru = eina_inlist_prepend(fonts_use_lru, EINA_INLIST_GET(fi));
         }
	return fn;
     }
   return NULL;
}

EAPI RGBA_Font *
evas_common_font_memory_add(RGBA_Font *fn, const char *source, const char *name, int size, const void *data, int data_size, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font_Int *fi;

   if (!fn)
      return NULL;
   fi = evas_common_font_int_memory_load(source, name, size, data, data_size, wanted_rend, bitmap_scalable);
   if (fi)
     {
	fn->fonts = eina_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
        if (fi->inuse) evas_common_font_int_promote(fi);
        else
         {
           fi->inuse = 1;
           fonts_use_lru = eina_inlist_prepend(fonts_use_lru, EINA_INLIST_GET(fi));
         }
	return fn;
     }
   return NULL;
}

EAPI void
evas_common_font_int_unref(RGBA_Font_Int *fi)
{
   fi->references--;
   if (fi->references == 0)
     {
        fonts_lru = eina_list_append(fonts_lru, fi);
        evas_common_font_int_modify_cache_by(fi, 1);
        evas_common_font_flush();
     }
}

EAPI void
evas_common_font_free(RGBA_Font *fn)
{
   Eina_List *l;
   RGBA_Font_Int *fi;

   if (!fn) return;
   fn->references--;
   if (fn->references > 0) return;
   EINA_LIST_FOREACH(fn->fonts, l, fi)
     evas_common_font_int_unref(fi);
   evas_common_font_flush();
   eina_list_free(fn->fonts);
   if (fn->fash) fn->fash->freeme(fn->fash);
   LKD(fn->lock);
   free(fn);
}

EAPI void
evas_common_font_hinting_set(RGBA_Font *fn, Font_Hint_Flags hinting)
{
   Eina_List *l;
   RGBA_Font_Int *fi;

   if (!fn) return;
   fn->hinting = hinting;
   EINA_LIST_FOREACH(fn->fonts, l, fi)
     {
        if (fi->hinting != fn->hinting)
          _evas_common_font_int_clear(fi);
        fi->hinting = fn->hinting;
     }
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
evas_common_font_memory_hinting_load(const char *source, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font *fn;

   fn = evas_common_font_memory_load(source, name, size, data, data_size, wanted_rend, bitmap_scalable);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_hinting_load(const char *name, int size, Font_Hint_Flags hinting, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font *fn;

   fn = evas_common_font_load(name, size, wanted_rend, bitmap_scalable);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_hinting_add(RGBA_Font *fn, const char *name, int size, Font_Hint_Flags hinting, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   fn = evas_common_font_add(fn, name, size, wanted_rend, bitmap_scalable);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

EAPI RGBA_Font *
evas_common_font_memory_hinting_add(RGBA_Font *fn, const char *source, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting, Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   fn = evas_common_font_memory_add(fn, source, name, size, data, data_size,
                                    wanted_rend, bitmap_scalable);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

static void
_evas_common_font_int_clear(RGBA_Font_Int *fi)
{
   LKL(fi->ft_mutex);
   if (!fi->fash)
     {
        LKU(fi->ft_mutex);
        return;
     }
   evas_common_font_int_modify_cache_by(fi, -1);
   if (fi->references <= 1)
     {
        if (fi->fash)
          {
             fi->fash->freeme(fi->fash);
             fi->fash = NULL;
          }
     }
   if (fi->inuse) fonts_use_usage -= fi->usage;
   fi->usage = 0;
   fi->generation++;
   LKU(fi->ft_mutex);
}

static Eina_Bool
_evas_common_font_all_clear_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   RGBA_Font_Int *fi = data;
   _evas_common_font_int_clear(fi);
   return 1;
}

EAPI void
evas_common_font_all_clear(void)
{
   eina_hash_foreach(fonts, _evas_common_font_all_clear_cb, NULL);
}

void
evas_common_font_int_promote(RGBA_Font_Int *fi EINA_UNUSED)
{
  return;
/* unused - keep for reference
  if (fonts_use_lru == (Eina_Inlist *)fi) return;
  if (!fi->inuse) return;
  fonts_use_lru = eina_inlist_remove(fonts_use_lru, EINA_INLIST_GET(fi));
  fonts_use_lru = eina_inlist_prepend(fonts_use_lru, EINA_INLIST_GET(fi));
 */
}

void
evas_common_font_int_use_increase(int size)
{
  fonts_use_usage += size;
}

void
evas_common_font_int_use_trim(void)
{
  return;
/* unused - keep for reference
  Eina_Inlist *l;

  if (fonts_use_usage <= (font_cache << 1)) return;
  if (!fonts_use_lru) return;
  l = fonts_use_lru->last;
  while (l)
    {
      RGBA_Font_Int *fi = (RGBA_Font_Int *)l;
      if (fonts_use_usage <= (font_cache << 1)) break;
      // FIXME: del fi->kerning content
      _evas_common_font_int_clear(fi);
      evas_common_font_int_unload(fi);
      evas_common_font_int_promote(fi);
      l = l->prev;
    }
 */
}

void
evas_common_font_int_unload(RGBA_Font_Int *fi EINA_UNUSED)
{
  return;
/* unused - keep for reference
  if (!fi->src->ft.face) return;
  _evas_common_font_int_clear(fi);
  FT_Done_Size(fi->ft.size);
  fi->ft.size = NULL;
  evas_common_font_source_unload(fi->src);
 */
}

void
evas_common_font_int_reload(RGBA_Font_Int *fi)
{
  if (fi->src->ft.face) return;
  evas_common_font_source_load_complete(fi->src);
  return;
/* unused - keep for reference
  evas_common_font_source_reload(fi->src);
  evas_common_font_int_load_complete(fi);
 */
}

/* when the fi->references == 0 we increase this instead of really deleting
 * we then check if the cache_useage size is larger than allowed
 * !If the cache is NOT too large we dont delete font_int
 * !If the cache is too large we really delete font_int */
EAPI void
evas_common_font_int_modify_cache_by(RGBA_Font_Int *fi, int dir)
{
   font_cache_usage += dir * (sizeof(RGBA_Font) + fi->usage +
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
   evas_common_font_int_use_trim();
}

EAPI void
evas_common_font_flush(void)
{
   if (font_cache_usage < font_cache) return;
   while (font_cache_usage > font_cache)
     {
        int pfont_cache_usage;

        pfont_cache_usage = font_cache_usage;
        evas_common_font_flush_last();
        if (pfont_cache_usage == font_cache_usage) break;
     }
}

/* We run this when the cache gets larger than allowed size
 * We check cache size each time a fi->references goes to 0
 * PERFORMS: Find font_int(s) with references == 0 and delete them */
EAPI void
evas_common_font_flush_last(void)
{
   RGBA_Font_Int *fi = NULL;

   if (!fonts_lru) return;
   fi = eina_list_data_get(fonts_lru);
   fonts_lru = eina_list_remove_list(fonts_lru, fonts_lru);
   eina_hash_del(fonts, fi, fi);
}

EAPI RGBA_Font_Int *
evas_common_font_int_find(const char *name, int size,
                          Font_Rend_Flags wanted_rend,
						  Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   RGBA_Font_Int tmp_fi;
   RGBA_Font_Source tmp_fn;
   RGBA_Font_Int *fi;

   tmp_fn.name = (char*) eina_stringshare_add(name);
   tmp_fi.src = &tmp_fn;
   tmp_fi.size = size;
   tmp_fi.wanted_rend = wanted_rend;
   tmp_fi.bitmap_scalable = bitmap_scalable;
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

static void
_font_int_ext_clear(RGBA_Font_Int *fi)
{
   RGBA_Font_Glyph *fg;
   Fash_Glyph_Map *fmap;
   Fash_Glyph_Map2 *fash2;
   Fash_Glyph *fash;
   int i, j, k;

   fash = fi->fash;
   if (!fash) return;
   for (k = 0; k <= 0xff; k++)
     {
        fash2 = fash->bucket[k];
        if (fash2)
          {
             for (j = 0; j <= 0xff; j++)
               {
                  fmap = fash2->bucket[j];
                  if (fmap)
                    {
                       for (i = 0; i <= 0xff; i++)
                         {
                            fg = fmap->item[i];
                            if ((fg) && (fg != (void *)(-1)))
                              {
                                 if (fg->ext_dat)
                                   {
                                      if (fg->ext_dat_free)
                                        fg->ext_dat_free(fg->ext_dat);
                                      fg->ext_dat = NULL;
                                      fg->ext_dat_free = NULL;
                                   }
                              }
                         }
                    }
               }
          }
     }
}

static Eina_Bool
_cb_hash_font_ext(const Eina_Hash *hash EINA_UNUSED,
                  const void *key EINA_UNUSED,
                  void *data EINA_UNUSED,
                  void *fdata EINA_UNUSED)
{
   _font_int_ext_clear(data);
   return EINA_TRUE;
}

EAPI void
evas_common_font_ext_clear(void)
{
   eina_hash_foreach(fonts, _cb_hash_font_ext, NULL);
}
