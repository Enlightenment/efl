#include "evas_font_private.h"
#include "evas_font_draw.h"

#ifdef EVAS_CSERVE2
# include "../cserve2/evas_cs2_private.h"
#endif

#include FT_OUTLINE_H
#include FT_SYNTHESIS_H
#include FT_BITMAP_H
#include FT_TRUETYPE_DRIVER_H

FT_Library      evas_ft_lib = 0;
static int      initialised = 0;

LK(lock_font_draw); // for freetype2 API calls
LK(lock_bidi); // for evas bidi internal usage.
LK(lock_ot); // for evas bidi internal usage.

#define F_DOM _evas_font_log_dom_global
#define F_CRI(...) EINA_LOG_DOM_CRIT(F_DOM, __VA_ARGS__)
#define F_ERR(...) EINA_LOG_DOM_ERR(F_DOM, __VA_ARGS__)
#define F_WRN(...) EINA_LOG_DOM_WARN(F_DOM, __VA_ARGS__)
#define F_INF(...) EINA_LOG_DOM_INFO(F_DOM, __VA_ARGS__)
#define F_DBG(...) EINA_LOG_DOM_DBG(F_DOM, __VA_ARGS__)

int F_DOM = -1;

EAPI void
evas_common_font_init(void)
{
   int error;
   const char *s;
   FT_UInt interpreter_version =
#ifndef TT_INTERPRETER_VERSION_35
   TT_INTERPRETER_VERSION_35;
#else
   35;
#endif
   F_DOM = eina_log_domain_register
     ("evas_font_main", EVAS_FONT_DEFAULT_LOG_COLOR);
   if (F_DOM < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
     }

   initialised++;
   if (initialised != 1) return;
   error = FT_Init_FreeType(&evas_ft_lib);
   if (error) return;
   FT_Property_Set(evas_ft_lib, "truetype", "interpreter-version",
                   &interpreter_version);
   evas_common_font_load_init();
   evas_common_font_draw_init();
   s = getenv("EVAS_FONT_DPI");
   if (s)
     {
        int dpi_h = 75, dpi_v = 0;

        if (sscanf(s, "%dx%d", &dpi_h, &dpi_v) < 2)
          dpi_h = dpi_v = atoi(s);

        if (dpi_h > 0) evas_common_font_dpi_set(dpi_h, dpi_v);
     }
   LKI(lock_font_draw);
   LKI(lock_bidi);
   LKI(lock_ot);
}

EAPI void
evas_common_font_shutdown(void)
{
   if (initialised < 1) return;
   initialised--;
   if (initialised != 0) return;

   evas_common_font_load_shutdown();
   evas_common_font_cache_set(0);
   evas_common_font_flush();

   FT_Done_FreeType(evas_ft_lib);
   evas_ft_lib = 0;

   LKD(lock_font_draw);
   LKD(lock_bidi);
   LKD(lock_ot);
   eina_log_domain_unregister(F_DOM);
}

EAPI void
evas_common_font_font_all_unload(void)
{
   evas_common_font_all_clear();
}

/* FIXME: This function should not be used. It's a short-cut fix that is meant
 * to improve font fallback for in-theme fonts. It will break if we stop using
 * freetype (unlikely) or do anything else fancy. */
void *
evas_common_font_freetype_face_get(RGBA_Font *font)
{
   RGBA_Font_Int *fi = font->fonts->data;

   if (!fi)
      return NULL;

   evas_common_font_int_reload(fi);

   return fi->src->ft.face;
}

EAPI int
evas_common_font_instance_ascent_get(RGBA_Font_Int *fi)
{
   int val;
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

   if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
     {
        if (FT_HAS_COLOR(fi->src->ft.face) &&
            fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
          val *= fi->scale_factor;
     }

   return FONT_METRIC_ROUNDUP(val);
//   printf("%i | %i\n", val, val >> 6);
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_instance_descent_get(RGBA_Font_Int *fi)
{
   int val;
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   val = -(int)fi->src->ft.face->size->metrics.descender;

   if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
     {
        if (FT_HAS_COLOR(fi->src->ft.face) &&
            fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
          val *= fi->scale_factor;
     }

   return FONT_METRIC_ROUNDUP(val);
//   if (fi->src->ft.face->units_per_EM == 0)
//     return val;
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_instance_max_ascent_get(RGBA_Font_Int *fi)
{
   int val, dv;
   int ret;

   evas_common_font_int_reload(fi);
  if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   if ((fi->src->ft.face->bbox.yMax == 0) &&
       (fi->src->ft.face->bbox.yMin == 0) &&
       (fi->src->ft.face->units_per_EM == 0))
     val = FONT_METRIC_ROUNDUP((int)fi->src->ft.face->size->metrics.ascender);
   else
     val = (int)fi->src->ft.face->bbox.yMax;

   if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
     {
        if (FT_HAS_COLOR(fi->src->ft.face) &&
            fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
          val *= fi->scale_factor;
     }

   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = FONT_METRIC_CONV(val, dv, fi->src->ft.face->size->metrics.y_scale);
   return ret;
}

EAPI int
evas_common_font_instance_max_descent_get(RGBA_Font_Int *fi)
{
   int val, dv;
   int ret;

   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   if ((fi->src->ft.face->bbox.yMax == 0) &&
       (fi->src->ft.face->bbox.yMin == 0) &&
       (fi->src->ft.face->units_per_EM == 0))
     val = FONT_METRIC_ROUNDUP(-(int)fi->src->ft.face->size->metrics.descender);
   else
     val = -(int)fi->src->ft.face->bbox.yMin;

   if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
     {
        if (FT_HAS_COLOR(fi->src->ft.face) &&
            fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
          val *= fi->scale_factor;
     }

   if (fi->src->ft.face->units_per_EM == 0)
     return val;
   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
   ret = FONT_METRIC_CONV(val, dv, fi->src->ft.face->size->metrics.y_scale);
   return ret;
}

EAPI int
evas_common_font_ascent_get(RGBA_Font *fn)
{
//   evas_common_font_size_use(fn);
   return evas_common_font_instance_ascent_get(fn->fonts->data);
}

EAPI int
evas_common_font_descent_get(RGBA_Font *fn)
{
//   evas_common_font_size_use(fn);
   return evas_common_font_instance_descent_get(fn->fonts->data);
}

EAPI int
evas_common_font_max_ascent_get(RGBA_Font *fn)
{
//   evas_common_font_size_use(fn);
   return evas_common_font_instance_max_ascent_get(fn->fonts->data);
}

EAPI int
evas_common_font_max_descent_get(RGBA_Font *fn)
{
//   evas_common_font_size_use(fn);
   return evas_common_font_instance_max_descent_get(fn->fonts->data);
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

   if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
     {
        if ((fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR) &&
            FT_HAS_COLOR(fi->src->ft.face))
          val *= fi->scale_factor;
     }

   if ((fi->src->ft.face->bbox.yMax == 0) &&
       (fi->src->ft.face->bbox.yMin == 0) &&
       (fi->src->ft.face->units_per_EM == 0))
     return FONT_METRIC_ROUNDUP(val);
   else if (fi->src->ft.face->units_per_EM == 0)
     return val;
   return FONT_METRIC_ROUNDUP(val);
//   dv = (fi->src->ft.orig_upem * 2048) / fi->src->ft.face->units_per_EM;
//   ret = (val * fi->src->ft.face->size->metrics.y_scale) / (dv * dv);
//   return ret;
}

EAPI int
evas_common_font_instance_underline_position_get(RGBA_Font_Int *fi)
{
   int position = 0;

   if (!fi) goto end;

   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   position = FT_MulFix(fi->src->ft.face->underline_position,
         fi->src->ft.face->size->metrics.x_scale);
   position = FONT_METRIC_ROUNDUP(abs(position));

end:
   /* This almost surely means a broken font, offset at least by one pixel. */
   if (position == 0)
      position = 1;

   return position;
}

EAPI int
evas_common_font_instance_underline_thickness_get(RGBA_Font_Int *fi)
{
   int thickness = 0;

   if (!fi) goto end;

   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   thickness = FT_MulFix(fi->src->ft.face->underline_thickness,
         fi->src->ft.face->size->metrics.x_scale);
   thickness = FONT_METRIC_ROUNDUP(thickness);

end:
   /* This almost surely means a broken font, make it at least one pixel. */
   if (thickness == 0)
      thickness = 1;

   return thickness;
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
_glyph_free(RGBA_Font_Glyph *fg)
{
   if ((!fg) || (fg == (void *)(-1))) return;

   if (fg->glyph_out)
     {
        if ((!fg->glyph_out->rle) && (!fg->glyph_out->bitmap.rle_alloc))
          {
             FT_BitmapGlyph fbg = (FT_BitmapGlyph)fg->glyph;
             FT_Bitmap_Done(evas_ft_lib, &(fbg->bitmap));
          }

        if ((fg->glyph_out->rle) && (fg->glyph_out->bitmap.rle_alloc))
          free(fg->glyph_out->rle);
        fg->glyph_out->rle = NULL;
        if (!fg->glyph_out->bitmap.no_free_glout) free(fg->glyph_out);
        fg->glyph_out = NULL;
     }
   FT_Done_Glyph(fg->glyph);
   /* extension calls */
   if (fg->ext_dat_free) fg->ext_dat_free(fg->ext_dat);
   free(fg);
}

static void
_fash_glyph_free(Fash_Glyph_Map *fmap)
{
   int i;

   for (i = 0; i <= 0xff; i++)
     {
        RGBA_Font_Glyph *fg = fmap->item[i];
        if ((fg) && (fg != (void *)(-1)))
          {
             _glyph_free(fg);
             fmap->item[i] = NULL;
          }
     }
  free(fmap);
}

static void
_fash_gl2_free(Fash_Glyph_Map2 *fash)
{
   int i;

   // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
   for (i = 0; i < 256; i++) if (fash->bucket[i]) _fash_glyph_free(fash->bucket[i]);
   free(fash);
}

static void
_fash_gl_free(Fash_Glyph *fash)
{
   int i;

    // 24bits for unicode - v6 up to E01EF (chrs) & 10FFFD for private use (plane 16)
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
   FT_Error error;
   const FT_Int32 hintflags[3] =
     { FT_LOAD_NO_HINTING, FT_LOAD_FORCE_AUTOHINT, FT_LOAD_NO_AUTOHINT };
   static FT_Matrix transform = {0x10000, _EVAS_FONT_SLANT_TAN * 0x10000,
        0x00000, 0x10000};

   evas_common_font_int_promote(fi);
   if (fi->fash)
     {
        fg = _fash_gl_find(fi->fash, idx);
        if (fg == (void *)(-1)) return NULL;
        else if (fg)
          {
#ifdef EVAS_CSERVE2
             if (fi->cs2_handler)
               {
                  if (evas_cserve2_font_glyph_used(fi->cs2_handler, idx,
                                                   fi->hinting))
                    return fg;
                  else
                    {
                       _glyph_free(fg);
                       _fash_gl_add(fi->fash, idx, NULL);
                    }
               }
             else return fg;
#else
             return fg;
#endif
          }
     }
//   fg = eina_hash_find(fi->glyphs, &hindex);
//   if (fg) return fg;

   evas_common_font_int_reload(fi);
   FTLOCK();
   error = FT_Load_Glyph(fi->src->ft.face, idx,
                         (FT_HAS_COLOR(fi->src->ft.face) ?
                          (FT_LOAD_COLOR | hintflags[fi->hinting]) :
                          (FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP | hintflags[fi->hinting])));

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
      FT_GlyphSlot_Embolden(fi->src->ft.face->glyph);

   fg = calloc(1, sizeof(RGBA_Font_Glyph));
   if (!fg) return NULL;

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

     {
        FT_BBox outbox;
        FT_Glyph_Get_CBox(fg->glyph,
              ((fi->hinting == 0) ? FT_GLYPH_BBOX_UNSCALED :
               FT_GLYPH_BBOX_GRIDFIT),
              &outbox);
        fg->width = EVAS_FONT_ROUND_26_6_TO_INT(outbox.xMax - outbox.xMin);
        fg->x_bear = EVAS_FONT_ROUND_26_6_TO_INT(outbox.xMin);
        fg->y_bear = EVAS_FONT_ROUND_26_6_TO_INT(outbox.yMax);

        if (FT_HAS_FIXED_SIZES(fi->src->ft.face))
          {
             if (FT_HAS_COLOR(fi->src->ft.face) &&
                 fi->bitmap_scalable & EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR)
               {
                  fg->glyph->advance.x *= fi->scale_factor;
                  fg->glyph->advance.y *= fi->scale_factor;
                  fg->width *= fi->scale_factor;
                  fg->x_bear *= fi->scale_factor;
                  fg->y_bear *= fi->scale_factor;
               }
          }
     }

   fg->index = idx;
   fg->fi = fi;

   if (!fi->fash) fi->fash = _fash_gl_new();
   if (fi->fash) _fash_gl_add(fi->fash, idx, fg);

#ifdef EVAS_CSERVE2
   if (fi->cs2_handler)
     evas_cserve2_font_glyph_request(fi->cs2_handler, idx, fi->hinting);
#endif

//   eina_hash_direct_add(fi->glyphs, &fg->index, fg);
   return fg;
}

EAPI Eina_Bool
evas_common_font_int_cache_glyph_render(RGBA_Font_Glyph *fg)
{
   int size;
   FT_Error error;
   RGBA_Font_Int *fi = fg->fi;
   FT_BitmapGlyph fbg;

#ifdef EVAS_CSERVE2
   if (fi->cs2_handler)
     {
        fg->glyph_out = evas_cserve2_font_glyph_bitmap_get(fi->cs2_handler,
                                                           fg->index,
                                                           fg->fi->hinting);
        if (!fg->glyph_out)
          {
             if (!fi->fash) fi->fash = _fash_gl_new();
             if (fi->fash) _fash_gl_add(fi->fash, fg->index, (void *)(-1));
             free(fg);
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
#endif

   /* no cserve2 case */
   if (fg->glyph_out)
     return EINA_TRUE;

   FTLOCK();
   error = FT_Glyph_To_Bitmap(&(fg->glyph), FT_RENDER_MODE_NORMAL, 0, 1);
   if (error)
     {
        FT_Done_Glyph(fg->glyph);
        FTUNLOCK();
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, fg->index, (void *)(-1));
        free(fg);
        return EINA_FALSE;
     }
   FTUNLOCK();

   fbg = (FT_BitmapGlyph)fg->glyph;

   fg->glyph_out = calloc(1, sizeof(RGBA_Font_Glyph_Out));
   fg->glyph_out->bitmap.rows = fbg->bitmap.rows;
   fg->glyph_out->bitmap.width = fbg->bitmap.width;
   fg->glyph_out->bitmap.pitch = fbg->bitmap.pitch;
   fg->glyph_out->bitmap.buffer = fbg->bitmap.buffer;
   fg->glyph_out->bitmap.rle_alloc = EINA_TRUE;

   /* This '+ 100' is just an estimation of how much memory freetype will use
    * on it's size. This value is not really used anywhere in code - it's
    * only for statistics. */
   size = sizeof(RGBA_Font_Glyph) + sizeof(Eina_List) +
    (fg->glyph_out->bitmap.width * fg->glyph_out->bitmap.rows / 2) + 100;
   fi->usage += size;
   if (fi->inuse) evas_common_font_int_use_increase(size);

   if (!FT_HAS_COLOR(fi->src->ft.face))
     {
        fg->glyph_out->rle = evas_common_font_glyph_compress
           (fbg->bitmap.buffer, fbg->bitmap.num_grays, fbg->bitmap.pixel_mode,
            fbg->bitmap.pitch, fbg->bitmap.width, fbg->bitmap.rows,
            &(fg->glyph_out->rle_size));
        fg->glyph_out->bitmap.rle_alloc = EINA_TRUE;

        fg->glyph_out->bitmap.buffer = NULL;

        // this may be technically incorrect as we go and free a bitmap buffer
        // behind the ftglyph's back...
        FT_Bitmap_Done(evas_ft_lib, &(fbg->bitmap));
     }
   else
     {
        fg->glyph_out->rle = NULL;
        fg->glyph_out->bitmap.rle_alloc = EINA_FALSE;
     }

   return EINA_TRUE;
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
   static const unsigned short mapfix[] =
     {
        0x00b0, 0x7,
        0x00b1, 0x8,
        0x00b7, 0x1f,
        0x03c0, 0x1c,
        0x20a4, 0xa3,
        0x2260, 0x1d,
        0x2264, 0x1a,
        0x2265, 0x1b,
        0x23ba, 0x10,
        0x23bb, 0x11,
        0x23bc, 0x13,
        0x23bd, 0x14,
        0x2409, 0x3,
        0x240a, 0x6,
        0x240b, 0xa,
        0x240c, 0x4,
        0x240d, 0x5,
        0x2424, 0x9,
        0x2500, 0x12,
        0x2502, 0x19,
        0x250c, 0xd,
        0x2510, 0xc,
        0x2514, 0xe,
        0x2518, 0xb,
        0x251c, 0x15,
        0x2524, 0x16,
        0x252c, 0x18,
        0x2534, 0x17,
        0x253c, 0xf,
        0x2592, 0x2,
        0x25c6, 0x1,
     };
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
   /*
    * There is no point in locking FreeType at this point as all caller
    * are running in the main loop at a time where there is zero chance
    * that something else try to use it.
    */
   /* FTLOCK(); */
   result.index = FT_Get_Char_Index(fi->src->ft.face, gl);
   /* FTUNLOCK(); */
   result.gl = gl;

//   eina_hash_direct_add(fi->indexes, &result->gl, result);
//
// on_correct:
#ifdef HAVE_PTHREAD
//   pthread_mutex_unlock(&fi->ft_mutex);
#endif
   // this is a workaround freetype bugs where for a bitmap old style font
   // even if it has unicode information and mappings, they are not used
   // to find terminal line/drawing chars, so do this by hand with a table
   if ((result.index <= 0) && (fi->src->ft.face->num_fixed_sizes == 1) &&
      (fi->src->ft.face->num_glyphs < 512))
     {
        int i, min = 0, max;

        // binary search through sorted table of codepoints to new
        // codepoints with a guess that bitmap font is playing the old
        // game of putting line drawing chars in specific ranges
        max = sizeof(mapfix) / (sizeof(mapfix[0]) * 2);
        i = (min + max) / 2;
        for (;;)
          {
             unsigned short v;

             v = mapfix[i << 1];
             if (gl == v)
               {
                  gl = mapfix[(i << 1) + 1];
                  FTLOCK();
                  result.index = FT_Get_Char_Index(fi->src->ft.face, gl);
                  FTUNLOCK();
                  break;
               }
             // failure to find at all
             if ((max - min) <= 2) break;
             // if glyph above out position...
             if (gl > v)
               {
                  min = i;
                  if ((max - min) == 1) i = max;
                  else i = (min + max) / 2;
               }
             // if glyph below out position
             else if (gl < v)
               {
                  max = i;
                  if ((max - min) == 1) i = min;
                  else i = (min + max) / 2;
               }
          }
     }
   return result.index;
}

EAPI int
evas_common_font_glyph_search(RGBA_Font *fn, RGBA_Font_Int **fi_ret, Eina_Unicode gl)
{
   F_DBG("Searching glyph: %x", gl);
   Eina_List *l;

   if (fn->fash)
     {
        Fash_Item_Index_Map *fm = _fash_int_find(fn->fash, gl);
        if (fm)
          {
             if (fm->fint)
               {
                  *fi_ret = fm->fint;
                  //printf("Found in fash: %s\n", fm->fint->src->name);
                  return fm->index;
               }
             else if (fm->index == -1) return 0;
          }
     }

   F_DBG("Not found in fash. Checking font set (total: %d)",
         eina_list_count(fn->fonts));

   for (l = fn->fonts; l; l = l->next)
     {
        RGBA_Font_Int *fi;
        int idx;

        fi = l->data;
        F_DBG("Trying: %s\n", fi->src->name);

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
                  F_DBG("Found!");
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
