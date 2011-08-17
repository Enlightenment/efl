#include "evas_common.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#include "evas_font_ot.h"

#define WORD_CACHE_MAXLEN	50
/* How many to cache */
#define WORD_CACHE_NWORDS 40

static int max_cached_words = WORD_CACHE_NWORDS;

struct prword 
{
   EINA_INLIST;
   struct cinfo *cinfo;
   Evas_Text_Props text_props;
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
static struct prword *evas_font_word_prerender(RGBA_Draw_Context *dc, const Evas_Text_Props *text_props);
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

/* 
 * BiDi handling: We receive the shaped string + other props from text_props,
 * we need to reorder it so we'll have the visual string (the way we draw)
 * and then for kerning we have to switch the order of the kerning query (as the prev
 * is on the right, and not on the left).
 */
static void
evas_common_font_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn __UNUSED__, int x, int y,
                               const Evas_Text_Props *text_props, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, 
                               int ext_h, int im_w, int im_h __UNUSED__)
{
   DATA32 *im;
   FT_Face pface = NULL;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();

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


#if defined(METRIC_CACHE) || defined(WORD_CACHE)
   unsigned int len;

   len = text_props->len;

   if (len > 2 && (len < WORD_CACHE_MAXLEN))
     {
        struct prword *word;

        word =
          evas_font_word_prerender(dc, text_props);
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
/* For some reason, metric and word chache are exactly the same except for
 * this piece of code that makes metric go nuts. ATM, we'll just go the
 * WORD_CACHE path. */
#if defined(METRIC_CACHE) || defined(WORD_CACHE)
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
             unsigned int ind;

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
#endif

   im = dst->image.data;

   EVAS_FONT_WALK_TEXT_START()
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        int chr_x, chr_y, chr_w;

        if (!EVAS_FONT_WALK_IS_VISIBLE) continue;

        idx = EVAS_FONT_WALK_INDEX;

        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, idx);
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
                                  (fg->glyph_out->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY))
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
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const Evas_Text_Props *text_props)
{
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;
   RGBA_Gfx_Func func;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch;
   int i;

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
//   evas_common_font_size_use(fn);
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   if (!dc->cutout.rects)
     {
        evas_common_font_draw_internal(dst, dc, fn, x, y, text_props,
                                       func, ext_x, ext_y, ext_w, ext_h,
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
                  evas_common_font_draw_internal(dst, dc, fn, x, y, text_props,
                                                 func, r->x, r->y, r->w, r->h,
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

/* Only used if cache is on */
#if defined(METRIC_CACHE) || defined(WORD_CACHE)

static Eina_Bool
_evas_font_word_prerender_text_props_equal(const Evas_Text_Props *_a, const Evas_Text_Props *_b)
{
   Evas_Font_Glyph_Info *gl1, *gl2;
   size_t i;

   if ((_a->len != _b->len) ||
       (_a->font_instance != _b->font_instance))
     return EINA_FALSE;

   gl1 = _a->info->glyph + _a->start;
   gl2 = _b->info->glyph + _b->start;
   i = _a->len;
   for ( ; (i > 0) && (gl1->index == gl2->index) ; i--, gl1++, gl2++)
      ;

   return (i == 0);
}

static struct prword *
evas_font_word_prerender(RGBA_Draw_Context *dc, const Evas_Text_Props *text_props)
{
   struct cinfo *metrics;
   unsigned char *im;
   int width;
   int height, above, below, baseline, descent;
   unsigned int i,j;
   struct prword *w;
   int last_delta = 0;
   Eina_Unicode gl;
   struct cinfo *ci;
   unsigned int len = text_props->len;
   RGBA_Font_Int *fi = (RGBA_Font_Int *) text_props->font_instance;
   EVAS_FONT_WALK_TEXT_INIT();

# ifndef METRIC_CACHE
   gl = dc->font_ext.func.gl_new ? 1: 0;
   if (gl) return NULL;
# endif

   LKL(lock_words);
   EINA_INLIST_FOREACH(words,w)
     {
        if (_evas_font_word_prerender_text_props_equal(&w->text_props,
                                                       text_props))
          {
             words = eina_inlist_promote(words, EINA_INLIST_GET(w));
             LKU(lock_words);
             return w;
          }
     }
   LKU(lock_words);

   gl = dc->font_ext.func.gl_new ? 1: 0;

   above = 0; below = 0; baseline = 0; height = 0; descent = 0;

   /* First pass: Work out how big and populate */
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
             struct cinfo *cin = metrics + i;

             for (j = 0 ; j < cin->bm.rows ; j ++)
               {
                  int correction; /* Used to remove negative inset and such */
                  if (cin->pos.x < 0)
                    correction = -cin->pos.x;
                  else
                    correction = 0;

                  memcpy(im + cin->pos.x + (j + baseline - cin->bm.h) * width +
                        correction,
                        cin->bm.data + j * cin->bm.w + correction,
                        cin->bm.w - correction);
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
   evas_common_text_props_content_copy_and_ref(&save->text_props, text_props);
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
             evas_common_text_props_content_unref(&last->text_props);
             words = eina_inlist_remove(words, EINA_INLIST_GET(last));
             free(last);
          }
     }
   LKU(lock_words);

   return save;
}
#endif
