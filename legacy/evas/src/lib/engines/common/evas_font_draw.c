#include "evas_common.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"

#include FT_OUTLINE_H

#define WORD_CACHE_MAXLEN	50
/* How many to cache */
#define WORD_CACHE_NWORDS 40

static int max_cached_words = WORD_CACHE_NWORDS;

struct prword 
{
   EINA_INLIST;
   /* FIXME: Need to save font/size et al */
   int size;
   struct cinfo *cinfo;
   RGBA_Font *font;
   const Eina_Unicode *str;
   int len;
   DATA8 *im;
   int roww;
   int width;
   int height;
   int baseline;
};

struct cinfo 
{
   FT_UInt index;
   struct 
     {
        int x, y;
     } pos;
   int posx;
   RGBA_Font_Glyph *fg;
   struct 
     {
        int w,h;
        int rows;
        unsigned char *data;
     } bm;
};


#if defined(METRIC_CACHE) || defined(WORD_CACHE)
LK(lock_words); // for word cache call
static Eina_Inlist *words = NULL;
static struct prword *evas_font_word_prerender(RGBA_Draw_Context *dc, const Eina_Unicode *text, const Evas_Text_Props *text_props, int len, RGBA_Font *fn, RGBA_Font_Int *fi);
#endif

EAPI void
evas_common_font_draw_init(void)
{
   char *p;
   int tmp;

   if ((p = getenv("EVAS_WORD_CACHE_MAX_WORDS")))
     {
	tmp = strtol(p,NULL,10);
	/* 0 to disable of course */
	if (tmp > -1 && tmp < 500){
	     max_cached_words = tmp;
	}
     }
}

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_font_draw_finish(void)
{ 
}
#endif

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
_fash_int_add(Fash_Int *fash, int item, RGBA_Font_Int *fint, int index)
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
   fash->bucket[grp]->bucket[maj]->item[min].index = index;
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
evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt index)
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
        fg = _fash_gl_find(fi->fash, index);
        if (fg == (void *)(-1)) return NULL;
        else if (fg) return fg;
     }

   hindex = index + (fi->hinting * 500000000);

//   fg = eina_hash_find(fi->glyphs, &hindex);
//   if (fg) return fg;

   evas_common_font_int_reload(fi);
   FTLOCK();
   error = FT_Load_Glyph(fi->src->ft.face, index,
                         FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP |
                         hintflags[fi->hinting]);
   FTUNLOCK();
   if (error)
     {
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, index, (void *)(-1));
        return NULL;
     }

   /* Transform the outline of Glyph according to runtime_rend. */
   if (fi->runtime_rend & FONT_REND_ITALIC)
      FT_Outline_Transform(&fi->src->ft.face->glyph->outline, &transform);
   /* Embolden the outline of Glyph according to rundtime_rend. */
   if (fi->runtime_rend & FONT_REND_BOLD)
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
        if (fi->fash) _fash_gl_add(fi->fash, index, (void *)(-1));
	return NULL;
     }
   if (fg->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
        FTLOCK();
	error = FT_Glyph_To_Bitmap(&(fg->glyph), FT_RENDER_MODE_NORMAL, 0, 1);
	if (error)
	  {
	     FT_Done_Glyph(fg->glyph);
             FTUNLOCK();
	     free(fg);
             if (!fi->fash) fi->fash = _fash_gl_new();
             if (fi->fash) _fash_gl_add(fi->fash, index, (void *)(-1));
	     return NULL;
	  }
        FTUNLOCK();
     }
   fg->glyph_out = (FT_BitmapGlyph)fg->glyph;
   fg->index = hindex;
   fg->fi = fi;

   if (!fi->fash) fi->fash = _fash_gl_new();
   if (fi->fash) _fash_gl_add(fi->fash, index, fg);
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
   int gl;
};

static FT_UInt
_evas_common_get_char_index(RGBA_Font_Int* fi, int gl)
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
evas_common_font_glyph_search(RGBA_Font *fn, RGBA_Font_Int **fi_ret, int gl)
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
	int index;

	fi = l->data;

#if 0 /* FIXME: charmap user is disabled and use a deprecated data type. */
/*        
	if (fi->src->charmap) // Charmap loaded, FI/FS blank
	  {
	     index = evas_array_hash_search(fi->src->charmap, gl);
	     if (index != 0)
	       {
		  evas_common_font_source_load_complete(fi->src);
		  evas_common_font_int_load_complete(fi);

		  evas_array_hash_free(fi->src->charmap);
		  fi->src->charmap = NULL;

		  *fi_ret = fi;
		  return index;
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
	     index = _evas_common_get_char_index(fi, gl);
	     if (index != 0)
	       {
		  if (!fi->ft.size)
                     evas_common_font_int_load_complete(fi);
                  if (!fn->fash) fn->fash = _fash_int_new();
                  if (fn->fash) _fash_int_add(fn->fash, gl, fi, index);
		  *fi_ret = fi;
		  return index;
	       }
             else
               {
                  if (!fn->fash) fn->fash = _fash_int_new();
                  if (fn->fash) _fash_int_add(fn->fash, gl, NULL, -1);
               }
	  }
     }
   return 0;
}

/* 
 * BiDi handling: We receive the shaped string + other props from text_props,
 * we need to reorder it so we'll have the visual string (the way we draw)
 * and then for kerning we have to switch the order of the kerning query (as the prev
 * is on the right, and not on the left).
 */
static void
evas_common_font_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const Eina_Unicode *in_text,
                               const Evas_Text_Props *text_props, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, 
                               int ext_h, RGBA_Font_Int *fi, int im_w, int im_h __UNUSED__)
{
#if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   const Eina_Unicode *text = (text_props->info) ?
      text_props->info->shaped_text + text_props->text_offset :
      in_text;
#else
   const Eina_Unicode *text = in_text;
   /* Not relevant in the Harfbuzz case, and will soon will not be relevant
    * at all */
   (void) text;
   /* Should be marked as unused? or should I do something fancy about it to
    * make sure the given fi is relevant? */
   (void) fn;
#endif
   DATA32 *im;
   FT_Face pface = NULL;
   EVAS_FONT_WALK_TEXT_INIT();

#if defined(METRIC_CACHE) || defined(WORD_CACHE)
   unsigned int len;

   len = text_props->text_len;

   if (len > 2 && (len < WORD_CACHE_MAXLEN))
     {
        struct prword *word;

        word =
          evas_font_word_prerender(dc, text, text_props,
                                   len, fn, fi);
        if (word)
          {
             int j, rowstart, rowend, xstart, xrun;

             im = dst->image.data;
             xrun = word->width;
             y -= word->baseline;
             xstart = 0;
             rowstart = 0;
             rowend = word->height;
             /* Clip to extent */
             if (x + xrun > ext_x + ext_w)
               {
                  xrun -= x + xrun - ext_x - ext_w;
               }
             if (x < ext_x) 
               {
                  int excess = ext_x - x;
                  xstart = excess - 1;
                  xrun -= excess;
                  x = ext_x;
               }
             if (y + rowend > ext_y + ext_h)
               {
                  rowend -= (y - ext_y + rowend - ext_h);
               }
             if (y < ext_y)
               {
                  int excess = ext_y - y;
                  rowstart += excess;
                //rowend -= excess;
                //      y = ext_y;
               }

             if (xrun < 1) return;
# ifdef WORD_CACHE
             if (word->im)
               {
                  for (j = rowstart ; j < rowend ; j ++)
                    {
                       func(NULL, word->im + (word->roww * j) + xstart, dc->col.col,
                            im + ((y + j) * im_w) + x, xrun);
                    }
                  return;
               }
# elif defined(METRIC_CACHE)
             int ind;

             y += word->baseline;
             for (ind = 0 ; ind < len ; ind ++)
               {
                  // FIXME Do we need to draw?
                  struct cinfo *ci = word->cinfo + ind;
                  for (j = rowstart ; j < rowend ; j ++)
                    {
                       if ((ci->fg->ext_dat) && (dc->font_ext.func.gl_draw))
                         {
                            /* ext glyph draw */
                            dc->font_ext.func.gl_draw(dc->font_ext.data,
                                                      (void *)dst,
                                                      dc, ci->fg,
                                                      x + ci->pos.x,
                                                      y - ci->bm.h + j);
                         }
                       else
                         {
                            func(NULL, word->im + (word->roww * j) + xstart,
                                 dc->col.col, im + ((y + j) * im_w) + x, xrun);
                         }
                    }
               }
             return;
# endif
          }
     }
#else
   (void) text;
   (void) fn;
#endif

   im = dst->image.data;

   fi = text_props->font_instance;
   if (fi)
     {
        evas_common_font_int_reload(fi);

        if (fi->src->current_size != fi->size)
          {
             FTLOCK();
             FT_Activate_Size(fi->ft.size);
             FTUNLOCK();
             fi->src->current_size = fi->size;
          }
     }

   EVAS_FONT_WALK_TEXT_START()
     {
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        int chr_x, chr_y, chr_w;
        if (!EVAS_FONT_WALK_IS_VISIBLE) continue;

        index = EVAS_FONT_WALK_INDEX;

        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, index);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }

        pface = fi->src->ft.face;
        LKU(fi->ft_mutex);

        if (dc->font_ext.func.gl_new)
          {
             /* extension calls */
             fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
             fg->ext_dat_free = dc->font_ext.func.gl_free;
          }

        chr_x = x + EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR;
        chr_y = y + EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR;
        chr_w = EVAS_FONT_WALK_WIDTH;

        if (chr_x < (ext_x + ext_w))
          {
             DATA8 *data;
             int i, j, w, h;

             data = fg->glyph_out->bitmap.buffer;
             j = fg->glyph_out->bitmap.pitch;
             w = fg->glyph_out->bitmap.width;
             if (j < w) j = w;
             h = fg->glyph_out->bitmap.rows;
             /*
                if ((fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays)
                && (fg->glyph_out->bitmap.num_grays == 256)
                )
                */
               {
                  if ((j > 0) && (chr_x + w > ext_x))
                    {
                       if ((fg->ext_dat) && (dc->font_ext.func.gl_draw))
                         {
                            /* ext glyph draw */
                            dc->font_ext.func.gl_draw(dc->font_ext.data,
                                  (void *)dst,
                                  dc, fg, chr_x,
                                  y - (chr_y - y));
                         }
                       else
                         {
                            if ((fg->glyph_out->bitmap.num_grays == 256) &&
                                  (fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays))
                              {
                                 for (i = 0; i < h; i++)
                                   {
                                      int dx, dy;
                                      int in_x, in_w;

                                      in_x = 0;
                                      in_w = 0;
                                      dx = chr_x;
                                      dy = y - (chr_y - i - y);
#ifdef EVAS_SLI
                                      if (((dy) % dc->sli.h) == dc->sli.y)
#endif
                                        {
                                           if ((dx < (ext_x + ext_w)) &&
                                                 (dy >= (ext_y)) &&
                                                 (dy < (ext_y + ext_h)))
                                             {
                                                if (dx + w > (ext_x + ext_w))
                                                  in_w += (dx + w) - (ext_x + ext_w);
                                                if (dx < ext_x)
                                                  {
                                                     in_w += ext_x - dx;
                                                     in_x = ext_x - dx;
                                                     dx = ext_x;
                                                  }
                                                if (in_w < w)
                                                  {
                                                     func(NULL, data + (i * j) + in_x, dc->col.col,
                                                           im + (dy * im_w) + dx, w - in_w);
                                                  }
                                             }
                                        }
                                   }
                              }
                            else
                              {
                                 DATA8 *tmpbuf = NULL, *dp, *tp, bits;
                                 int bi, bj;
                                 const DATA8 bitrepl[2] = {0x0, 0xff};

                                 tmpbuf = alloca(w);
                                 for (i = 0; i < h; i++)
                                   {
                                      int dx, dy;
                                      int in_x, in_w, end;

                                      in_x = 0;
                                      in_w = 0;
                                      dx = chr_x;
                                      dy = y - (chr_y - i - y);
#ifdef EVAS_SLI
                                      if (((dy) % dc->sli.h) == dc->sli.y)
#endif
                                        {
                                           tp = tmpbuf;
                                           dp = data + (i * fg->glyph_out->bitmap.pitch);
                                           for (bi = 0; bi < w; bi += 8)
                                             {
                                                bits = *dp;
                                                if ((w - bi) < 8) end = w - bi;
                                                else end = 8;
                                                for (bj = 0; bj < end; bj++)
                                                  {
                                                     *tp = bitrepl[(bits >> (7 - bj)) & 0x1];
                                                     tp++;
                                                  }
                                                dp++;
                                             }
                                           if ((dx < (ext_x + ext_w)) &&
                                                 (dy >= (ext_y)) &&
                                                 (dy < (ext_y + ext_h)))
                                             {
                                                if (dx + w > (ext_x + ext_w))
                                                  in_w += (dx + w) - (ext_x + ext_w);
                                                if (dx < ext_x)
                                                  {
                                                     in_w += ext_x - dx;
                                                     in_x = ext_x - dx;
                                                     dx = ext_x;
                                                  }
                                                if (in_w < w)
                                                  {
                                                     func(NULL, tmpbuf + in_x, dc->col.col,
                                                           im + (dy * im_w) + dx, w - in_w);
                                                  }
                                             }
                                        }
                                   }
                              }
                         }
                    }
               }
          }
        else
          break;
     }
   EVAS_FONT_WALK_TEXT_END();
  evas_common_font_int_use_trim();
}

EAPI void
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const Eina_Unicode *text,
                      const Evas_Text_Props *text_props)
{
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;
   RGBA_Gfx_Func func;
   RGBA_Font_Int *fi;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch;
   int i;

   fi = fn->fonts->data;

   im_w = dst->cache_entry.w;
   im_h = dst->cache_entry.h;

   ext_x = 0; ext_y = 0; ext_w = im_w; ext_h = im_h;
   if (dc->clip.use)
     {
	ext_x = dc->clip.x;
	ext_y = dc->clip.y;
	ext_w = dc->clip.w;
	ext_h = dc->clip.h;
	if (ext_x < 0)
	  {
	     ext_w += ext_x;
	     ext_x = 0;
	  }
	if (ext_y < 0)
	  {
	     ext_h += ext_y;
	     ext_y = 0;
	  }
	if ((ext_x + ext_w) > im_w)
	  ext_w = im_w - ext_x;
	if ((ext_y + ext_h) > im_h)
	  ext_h = im_h - ext_y;
     }
   if (ext_w <= 0) return;
   if (ext_h <= 0) return;

#ifdef EVAS_FRAME_QUEUING
   LKL(fn->lock);
#endif
   evas_common_font_int_reload(fi);
//   evas_common_font_size_use(fn);
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   if (!dc->cutout.rects)
     {
        evas_common_font_draw_internal(dst, dc, fn, x, y, text, text_props,
                                       func, ext_x, ext_y, ext_w, ext_h, fi,
                                       im_w, im_h);
     }
   else
     {
        c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
        evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
        /* our clip is 0 size.. abort */
        if ((dc->clip.w > 0) && (dc->clip.h > 0))
          {
             rects = evas_common_draw_context_apply_cutouts(dc);
             for (i = 0; i < rects->active; ++i)
               {
                  r = rects->rects + i;
                  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
                  evas_common_font_draw_internal(dst, dc, fn, x, y, text, text_props,
                                                 func, r->x, r->y, r->w, r->h, fi,
                                                 im_w, im_h);
               }
             evas_common_draw_context_apply_clear_cutouts(rects);
          }
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
     }
#ifdef EVAS_FRAME_QUEUING
   LKU(fn->lock);
#endif
}

/* FIXME: Where is it freed at? */
/* Only used if cache is on */
#if defined(METRIC_CACHE) || defined(WORD_CACHE)
struct prword *
evas_font_word_prerender(RGBA_Draw_Context *dc, const Eina_Unicode *in_text, const Evas_Text_Props *text_props, int len, RGBA_Font *fn, RGBA_Font_Int *fi)
{
   struct cinfo *metrics;
   const Eina_Unicode *text = in_text;
   unsigned char *im;
   int width;
   int height, above, below, baseline, descent;
   int i,j;
   struct prword *w;
   int last_delta = 0;
   int gl;
   struct cinfo *ci;
   EVAS_FONT_WALK_TEXT_INIT();

# ifndef METRIC_CACHE
   gl = dc->font_ext.func.gl_new ? 1: 0;
   if (gl) return NULL;
# endif

   LKL(lock_words);
   EINA_INLIST_FOREACH(words,w)
     {
	if (w->len == len && w->font == fn && fi->size == w->size &&
            (w->str == in_text || memcmp(w->str, in_text, len * sizeof(Eina_Unicode)) == 0)){
           words = eina_inlist_promote(words, EINA_INLIST_GET(w));
           LKU(lock_words);
           return w;
	}
     }
   LKU(lock_words);

   gl = dc->font_ext.func.gl_new ? 1: 0;

   above = 0; below = 0; baseline = 0; height = 0; descent = 0;
   fi = text_props->font_instance;
   if (fi)
     {
        evas_common_font_int_reload(fi);

        if (fi->src->current_size != fi->size)
          {
             FTLOCK();
             FT_Activate_Size(fi->ft.size);
             FTUNLOCK();
             fi->src->current_size = fi->size;
          }
     }

   /* First pass: Work out how big and populate */
   /* It's a bit hackish to use index and fg here as they are internal,
    * but that'll have to be good enough ATM */
   len = text_props->len;
   metrics = malloc(sizeof(struct cinfo) * len);
   ci = metrics;
   EVAS_FONT_WALK_TEXT_START()
     {
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        index = EVAS_FONT_WALK_INDEX;
        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, index);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }

        LKU(fi->ft_mutex);
        EVAS_FONT_WALK_TEXT_WORK();
        /* Currently broken with invisible chars if (!EVAS_FONT_WALK_IS_VISIBLE) continue; */
        ci->index = index;
        ci->fg = fg;

        if (gl)
          {
             ci->fg->ext_dat =dc->font_ext.func.gl_new(dc->font_ext.data,ci->fg);
             ci->fg->ext_dat_free = dc->font_ext.func.gl_free;
          }
        ci->bm.data = ci->fg->glyph_out->bitmap.buffer;
        ci->bm.w = MAX(ci->fg->glyph_out->bitmap.pitch,
                       ci->fg->glyph_out->bitmap.width);
        ci->bm.rows = ci->fg->glyph_out->bitmap.rows;
        ci->bm.h = ci->fg->glyph_out->top;
        above = ci->bm.rows - (ci->bm.rows - ci->bm.h);
        below = ci->bm.rows - ci->bm.h;
        if (below > descent) descent = below;
        if (above > baseline) baseline = above;
        ci->pos.x = EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF + EVAS_FONT_WALK_X_BEAR;
        ci->pos.y = EVAS_FONT_WALK_PEN_Y + EVAS_FONT_WALK_Y_OFF + EVAS_FONT_WALK_Y_BEAR;
        last_delta = EVAS_FONT_WALK_X_ADV -
           (ci->bm.w + ci->fg->glyph_out->left);
        ci++;
     }
   EVAS_FONT_WALK_TEXT_END();

   /* First loop done */
   width = EVAS_FONT_WALK_PEN_X;
   if (last_delta < 0)
     width -= last_delta;
   width = (width & 0x7) ? width + (8 - (width & 0x7)) : width;

   height = baseline + descent;
   if (!gl)
     {
        im = calloc(height, width);
        for (i = 0 ; i  < len ; i ++)
          {
             struct cinfo *ci = metrics + i;

             for (j = 0 ; j < ci->bm.rows ; j ++)
               {
                  int correction; /* Used to remove negative inset and such */
                  if (ci->pos.x < 0)
                    correction = -ci->pos.x;
                  else
                    correction = 0;

                  memcpy(im + ci->pos.x + (j + baseline - ci->bm.h) * width +
                        correction,
                        ci->bm.data + j * ci->bm.w + correction,
                        ci->bm.w - correction);
               }
          }
     }
   else 
     {
        im = NULL;
     }

   /* Save it */
   struct prword *save;

   save = malloc(sizeof(struct prword));
   save->cinfo = metrics;
   save->str = eina_ustringshare_add(in_text);
   save->font = fn;
   save->size = fi->size;
   save->len = len;
   save->im = im;
   save->width = EVAS_FONT_WALK_PEN_X;
   if (last_delta < 0)
     save->width += last_delta;
   save->roww = width;
   save->height = height;
   save->baseline = baseline;
   LKL(lock_words);
   words = eina_inlist_prepend(words, EINA_INLIST_GET(save));

   /* Clean up if too long */
   if (eina_inlist_count(words) > max_cached_words)
     {
	struct prword *last = (struct prword *)(words->last);

        if (last) 
          {
             if (last->im) free(last->im);
             if (last->cinfo) free(last->cinfo);
             eina_ustringshare_del(last->str);
             words = eina_inlist_remove(words, EINA_INLIST_GET(last));
             free(last);
          }
     }
   LKU(lock_words);

   return save;
}
#endif
