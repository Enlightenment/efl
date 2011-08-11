#include "evas_common.h"
#include "evas_private.h"

#include "evas_font_private.h"

#include FT_OUTLINE_H

FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

LK(lock_font_draw); // for freetype2 API calls
LK(lock_bidi); // for evas bidi internal usage.
LK(lock_ot); // for evas bidi internal usage.

EAPI void
evas_common_font_init(void)
{
   int error;

   initialised++;
   if (initialised != 1) return;
   error = FT_Init_FreeType(&evas_ft_lib);
   if (error) return;
   evas_common_font_load_init();
   evas_common_font_draw_init();
   LKI(lock_font_draw);
   LKI(lock_bidi);
   LKI(lock_ot);
}

EAPI void
evas_common_font_shutdown(void)
{
   int error;

   if (initialised < 1) return;
   initialised--;
   if (initialised != 0) return;

   LKD(lock_font_draw);
   LKD(lock_bidi);
   LKD(lock_ot);

   evas_common_font_load_shutdown();
   evas_common_font_cache_set(0);
   evas_common_font_flush();

   error = FT_Done_FreeType(evas_ft_lib);
#ifdef EVAS_FRAME_QUEUING
   evas_common_font_draw_finish();
#endif
   evas_ft_lib = 0;
}

EAPI void
evas_common_font_font_all_unload(void)
{
   evas_common_font_all_clear();
}

EAPI int
evas_common_font_ascent_get(RGBA_Font *fn)
{
   int val;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
#if 0
     {
        Eina_List *l;
        
        EINA_LIST_FOREACH(fn->fonts, l, fi)
          {
             if (!fi->src->ft.face) continue;
             if (fi->src->current_size != fi->size)
               {
		  FTLOCK();
                  FT_Activate_Size(fi->ft.size);
		  FTUNLOCK();
                  fi->src->current_size = fi->size;
               }
             val = (int)fi->src->ft.face->size->metrics.ascender;
             if (fi->src->ft.face->units_per_EM == 0)
               return val;
             dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
             ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
             printf(" ==== %p: %i\n", fi, ret);
          }
     }
#endif
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   if (!FT_IS_SCALABLE(fi->src->ft.face))
     {
        WRN("NOT SCALABLE!");
     }
   val = (int)fi->src->ft.face->size->metrics.ascender;
   return val >> 6;
//   printf("%i | %i\n", val, val >> 6);
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_descent_get(RGBA_Font *fn)
{
   int val;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   val = -(int)fi->src->ft.face->size->metrics.descender;
   return val >> 6;
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
  if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   val = (int)fi->src->ft.face->bbox.yMax;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_max_descent_get(RGBA_Font *fn)
{
   int val, dv;
   int ret;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   val = -(int)fi->src->ft.face->bbox.yMin;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
   return ret;
}

EAPI int
evas_common_font_get_line_advance(RGBA_Font *fn)
{
   int val;
   RGBA_Font_Int *fi;

//   evas_common_font_size_use(fn);
   fi = fn->fonts->data;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   val = (int)fi->src->ft.face->size->metrics.height;
   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   return val >> 6;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

/* Set of common functions that are used in a couple of places. */

static void
_fash_int2_free(Fash_Int_Map2 *fash)
{
   int i;

   for (i = 0; i < 256; i++) if (fash->bucket[i]) free(fash->bucket[i]);
   free(fash);
}

static void
_fash_int_free(Fash_Int *fash)
{
   int i;

   for (i = 0; i < 256; i++) if (fash->bucket[i]) _fash_int2_free(fash->bucket[i]);
   free(fash);
}

static Fash_Int *
_fash_int_new(void)
{
   Fash_Int *fash = calloc(1, sizeof(Fash_Int));
   fash->freeme = _fash_int_free;
   return fash;
}

static Fash_Item_Index_Map *
_fash_int_find(Fash_Int *fash, int item)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp]) return NULL;
   if (!fash->bucket[grp]->bucket[maj]) return NULL;
   return &(fash->bucket[grp]->bucket[maj]->item[min]);
}

static void
_fash_int_add(Fash_Int *fash, int item, RGBA_Font_Int *fint, int idx)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp])
     fash->bucket[grp] = calloc(1, sizeof(Fash_Int_Map2));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]);
   if (!fash->bucket[grp]->bucket[maj])
     fash->bucket[grp]->bucket[maj] = calloc(1, sizeof(Fash_Int_Map));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]->bucket[maj]);
   fash->bucket[grp]->bucket[maj]->item[min].fint = fint;
   fash->bucket[grp]->bucket[maj]->item[min].index = idx;
}

static void
_fash_gl2_free(Fash_Glyph_Map2 *fash)
{
   int i;

   for (i = 0; i < 256; i++) if (fash->bucket[i]) free(fash->bucket[i]);
   free(fash);
}

static void
_fash_gl_free(Fash_Glyph *fash)
{
   int i;

   for (i = 0; i < 256; i++) if (fash->bucket[i]) _fash_gl2_free(fash->bucket[i]);
   free(fash);
}

static Fash_Glyph *
_fash_gl_new(void)
{
   Fash_Glyph *fash = calloc(1, sizeof(Fash_Glyph));
   fash->freeme = _fash_gl_free;
   return fash;
}

static RGBA_Font_Glyph *
_fash_gl_find(Fash_Glyph *fash, int item)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp]) return NULL;
   if (!fash->bucket[grp]->bucket[maj]) return NULL;
   return fash->bucket[grp]->bucket[maj]->item[min];
}

static void
_fash_gl_add(Fash_Glyph *fash, int item, RGBA_Font_Glyph *glyph)
{
   int grp, maj, min;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   grp = (item >> 16) & 0xff;
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[grp])
     fash->bucket[grp] = calloc(1, sizeof(Fash_Glyph_Map2));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]);
   if (!fash->bucket[grp]->bucket[maj])
     fash->bucket[grp]->bucket[maj] = calloc(1, sizeof(Fash_Glyph_Map));
   EINA_SAFETY_ON_NULL_RETURN(fash->bucket[grp]->bucket[maj]);
   fash->bucket[grp]->bucket[maj]->item[min] = glyph;
}

EAPI RGBA_Font_Glyph *
evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt idx)
{
   RGBA_Font_Glyph *fg;
   FT_UInt hindex;
   FT_Error error;
   int size;
   const FT_Int32 hintflags[3] =
     { FT_LOAD_NO_HINTING, FT_LOAD_FORCE_AUTOHINT, FT_LOAD_NO_AUTOHINT };
   static FT_Matrix transform = {0x10000, 0x05000, 0x0000, 0x10000}; // about 12 degree.

   evas_common_font_int_promote(fi);
   if (fi->fash)
     {
        fg = _fash_gl_find(fi->fash, idx);
        if (fg == (void *)(-1)) return NULL;
        else if (fg) return fg;
     }

   hindex = idx + (fi->hinting * 500000000);

//   fg = eina_hash_find(fi->glyphs, &hindex);
//   if (fg) return fg;

   evas_common_font_int_reload(fi);
   FTLOCK();
   error = FT_Load_Glyph(fi->src->ft.face, idx,
                         FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP |
                         hintflags[fi->hinting]);
   FTUNLOCK();
   if (error)
     {
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, idx, (void *)(-1));
        return NULL;
     }

   /* Transform the outline of Glyph according to runtime_rend. */
   if (fi->runtime_rend & FONT_REND_SLANT)
      FT_Outline_Transform(&fi->src->ft.face->glyph->outline, &transform);
   /* Embolden the outline of Glyph according to rundtime_rend. */
   if (fi->runtime_rend & FONT_REND_WEIGHT)
      FT_Outline_Embolden(&fi->src->ft.face->glyph->outline,
            (fi->src->ft.face->size->metrics.x_ppem * 5 * 64) / 100);

   fg = malloc(sizeof(struct _RGBA_Font_Glyph));
   if (!fg) return NULL;
   memset(fg, 0, (sizeof(struct _RGBA_Font_Glyph)));

   FTLOCK();
   error = FT_Get_Glyph(fi->src->ft.face->glyph, &(fg->glyph));
   FTUNLOCK();
   if (error)
     {
        free(fg);
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, idx, (void *)(-1));
        return NULL;
     }

   FTLOCK();
   error = FT_Glyph_To_Bitmap(&(fg->glyph), FT_RENDER_MODE_NORMAL, 0, 1);
   if (error)
     {
        FT_Done_Glyph(fg->glyph);
        FTUNLOCK();
        free(fg);
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, idx, (void *)(-1));
        return NULL;
     }
   FTUNLOCK();

   fg->glyph_out = (FT_BitmapGlyph)fg->glyph;
   fg->index = hindex;
   fg->fi = fi;

   if (!fi->fash) fi->fash = _fash_gl_new();
   if (fi->fash) _fash_gl_add(fi->fash, idx, fg);
   /* This '+ 200' is just an estimation of how much memory freetype will use
    * on it's size. This value is not really used anywhere in code - it's
    * only for statistics. */
   size = sizeof(RGBA_Font_Glyph) + sizeof(Eina_List) +
    (fg->glyph_out->bitmap.width * fg->glyph_out->bitmap.rows) + 200;
   fi->usage += size;
   if (fi->inuse) evas_common_font_int_use_increase(size);

//   eina_hash_direct_add(fi->glyphs, &fg->index, fg);
   return fg;
}

typedef struct _Font_Char_Index Font_Char_Index;
struct _Font_Char_Index
{
   FT_UInt index;
   Eina_Unicode gl;
};

EAPI FT_UInt
evas_common_get_char_index(RGBA_Font_Int* fi, Eina_Unicode gl)
{
   Font_Char_Index result;
   //FT_UInt ret;

#ifdef HAVE_PTHREAD
///   pthread_mutex_lock(&fi->ft_mutex);
#endif

//   result = eina_hash_find(fi->indexes, &gl);
//   if (result) goto on_correct;
//
//   result = malloc(sizeof (Font_Char_Index));
//   if (!result)
//     {
//#ifdef HAVE_PTHREAD
//	pthread_mutex_unlock(&fi->ft_mutex);
//#endif
//	return FT_Get_Char_Index(fi->src->ft.face, gl);
//     }

   evas_common_font_int_reload(fi);
   FTLOCK();
   result.index = FT_Get_Char_Index(fi->src->ft.face, gl);
   FTUNLOCK();
   result.gl = gl;

//   eina_hash_direct_add(fi->indexes, &result->gl, result);
//
// on_correct:
#ifdef HAVE_PTHREAD
//   pthread_mutex_unlock(&fi->ft_mutex);
#endif
   return result.index;
}

EAPI int
evas_common_font_glyph_search(RGBA_Font *fn, RGBA_Font_Int **fi_ret, Eina_Unicode gl)
{
   Eina_List *l;

   if (fn->fash)
     {
        Fash_Item_Index_Map *fm = _fash_int_find(fn->fash, gl);
        if (fm)
          {
             if (fm->fint)
               {
                  *fi_ret = fm->fint;
                  return fm->index;
               }
             else if (fm->index == -1) return 0;
          }
     }

   for (l = fn->fonts; l; l = l->next)
     {
        RGBA_Font_Int *fi;
        int idx;

        fi = l->data;

#if 0 /* FIXME: charmap user is disabled and use a deprecated data type. */
/*        
	if (fi->src->charmap) // Charmap loaded, FI/FS blank
	  {
	     idx = evas_array_hash_search(fi->src->charmap, gl);
	     if (idx != 0)
	       {
		  evas_common_font_source_load_complete(fi->src);
		  evas_common_font_int_load_complete(fi);

		  evas_array_hash_free(fi->src->charmap);
		  fi->src->charmap = NULL;

		  *fi_ret = fi;
		  return idx;
               }
           }
        else
*/
#endif
        if (!fi->src->ft.face) /* Charmap not loaded, FI/FS blank */
          {
             evas_common_font_int_reload(fi);
          }
        if (fi->src->ft.face)
          {
             idx = evas_common_get_char_index(fi, gl);
             if (idx != 0)
               {
                  if (!fi->ft.size)
                    evas_common_font_int_load_complete(fi);
                  if (!fn->fash) fn->fash = _fash_int_new();
                  if (fn->fash) _fash_int_add(fn->fash, gl, fi, idx);
                  *fi_ret = fi;
                  return idx;
               }
             else
               {
                  if (!fn->fash) fn->fash = _fash_int_new();
                  if (fn->fash) _fash_int_add(fn->fash, gl, NULL, -1);
               }
          }
     }
   *fi_ret = NULL;
   return 0;
}
