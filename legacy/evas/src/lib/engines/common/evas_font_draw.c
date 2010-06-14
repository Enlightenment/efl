/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
#include "evas_blend_private.h"

#include "evas_intl_utils.h" /*defines INTERNATIONAL_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

#define WORD_CACHE_MAXLEN	50
/* How many to cache */
#define WORD_CACHE_NWORDS	20

struct prword {
	EINA_INLIST;
	/* FIXME: Need to save font/size et al */
	int size;
	struct cinfo *cinfo;
	RGBA_Font *font;
	const char *str;
	int len;
	DATA8 *im;
	int roww;
	int width;
	int height;
	int baseline;
};

struct cinfo {
	int gl;
	FT_UInt index;
	struct { int x, y; } pos;
	int posx;
	char chr;
	RGBA_Font_Glyph *fg;
	struct {
		int w,h;
		int rows;
		unsigned char *data;
	} bm;
};




static Eina_Inlist *words = NULL;
static struct prword *evas_font_word_prerender(RGBA_Draw_Context *dc, const char *text, int len, RGBA_Font *fn, RGBA_Font_Int *fi,int use_kerning);

#ifdef EVAS_FRAME_QUEUING
EAPI void
evas_common_font_draw_init(void)
{
   LKI(lock_font_draw);
   LKI(lock_fribidi);
}

EAPI void
evas_common_font_draw_finish(void)
{
   LKD(lock_font_draw);
   LKD(lock_fribidi);
}
#endif

static void
_fash_int_free(Fash_Int *fash)
{
   int i;
   
   for (i = 0; i < 256; i++) if (fash->bucket[i]) free(fash->bucket[i]);
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
   int maj, min;

   item &= 0xffff; // fixme: to do > 65k
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[maj]) return NULL;
   return &(fash->bucket[maj]->item[min]);
}

static void
_fash_int_add(Fash_Int *fash, int item, RGBA_Font_Int *fint, int index)
{
   int maj, min;
   
   item &= 0xffff; // fixme: to do > 65k
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[maj])
     fash->bucket[maj] = calloc(1, sizeof(Fash_Int_Map));
   fash->bucket[maj]->item[min].fint = fint;
   fash->bucket[maj]->item[min].index = index;
}





static void
_fash_gl_free(Fash_Glyph *fash)
{
   int i;
   
   for (i = 0; i < 256; i++) if (fash->bucket[i]) free(fash->bucket[i]);
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
   int maj, min;

   item &= 0xffff; // fixme: to do > 65k
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[maj]) return NULL;
   return fash->bucket[maj]->item[min];
}

static void
_fash_gl_add(Fash_Glyph *fash, int item, RGBA_Font_Glyph *glyph)
{
   int maj, min;
   
   item &= 0xffff; // fixme: to do > 65k
   maj = (item >> 8) & 0xff;
   min = item & 0xff;
   if (!fash->bucket[maj])
     fash->bucket[maj] = calloc(1, sizeof(Fash_Int_Map));
   fash->bucket[maj]->item[min] = glyph;
}





EAPI RGBA_Font_Glyph *
evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt index)
{
   RGBA_Font_Glyph *fg;
   FT_UInt hindex;
   FT_Error error;
   const FT_Int32 hintflags[3] =
     { FT_LOAD_NO_HINTING, FT_LOAD_FORCE_AUTOHINT, FT_LOAD_NO_AUTOHINT };

   if (fi->fash)
     {
        fg = _fash_gl_find(fi->fash, index);
        if (fg == (void *)(-1)) return NULL;
        else if (fg) return fg;
     }
   
   hindex = index + (fi->hinting * 500000000);
   
//   fg = eina_hash_find(fi->glyphs, &hindex);
//   if (fg) return fg;

   FTLOCK();
//   error = FT_Load_Glyph(fi->src->ft.face, index, FT_LOAD_NO_BITMAP);
   error = FT_Load_Glyph(fi->src->ft.face, index,
			 FT_LOAD_RENDER | hintflags[fi->hinting]);
   FTUNLOCK();
   if (error)
     {
        if (!fi->fash) fi->fash = _fash_gl_new();
        if (fi->fash) _fash_gl_add(fi->fash, index, (void *)(-1));
        return NULL;
     }

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
	     if (evas_common_font_source_load_complete(fi->src))
	       return 0;
#if 0 /* FIXME: disable this. this can eat a LOT of memory and in my tests with expedite at any rate shows no visible improvements */
/*             
	     index = FT_Get_Char_Index(fi->src->ft.face, gl);
	     if (index == 0)
	       {
		  // Load Hash
		  FT_ULong  charcode;
		  FT_UInt   gindex;

		  fi->src->charmap = evas_array_hash_new();
		  charcode = FT_Get_First_Char(fi->src->ft.face, &gindex);
		  while (gindex != 0)
		    {
		       evas_array_hash_add(fi->src->charmap, charcode, gindex);
		       charcode = FT_Get_Next_Char(fi->src->ft.face, charcode, &gindex);
		    }

		  // Free face
		  FT_Done_Face(fi->src->ft.face);
		  fi->src->ft.face = NULL;
	       }
	     else
	       {
		  evas_common_font_int_load_complete(fi);

		  *fi_ret = fi;
		  return index;
	       }
 */
#endif
	  }
	else /* Charmap not loaded, FS loaded */
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



static void
evas_common_font_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *in_text,
                               RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, int ext_h, RGBA_Font_Int *fi,
                               int im_w, int im_h __UNUSED__, int use_kerning
                               )
{
   int pen_x, pen_y;
   int chr;
   const char *text = in_text;
   int len;
   FT_Face pface = NULL;
   FT_UInt prev_index;
   DATA32 *im;
   int c;
   int char_index = 0; /* the index of the current char */

#ifdef INTERNATIONAL_SUPPORT
   int len = 0;
   /*FIXME: should get the direction by parmater */
   EvasIntlParType direction = FRIBIDI_TYPE_ON;
   EvasIntlLevel *level_list;

   /* change the text to visual ordering and update the level list
    * for as minimum impact on the code as possible just use text as an
    * holder, will change in the future.*/
   char *visual_text = evas_intl_utf8_to_visual(in_text, &len, &direction, NULL, NULL, &level_list);
   text = (visual_text) ? visual_text : in_text;
   
#endif

#if defined(METRICCACHE) || defined(WORDCACHE)
   /* A fast strNlen would be nice (there is a wcsnlen strangely) */
   for (len = 0 ; text[len] && len < WORD_CACHE_MAXLEN ; len ++)
     ;


   if (len < WORD_CACHE_MAXLEN){
     struct prword *word = evas_font_word_prerender(dc, text, len, fn, fi,
	   use_kerning);
     if (word){
	  int j,rowstart,rowend,xstart,xrun;
	  im = dst->image.data;
	  xrun = word->width;
	  y -= word->baseline;
	  xstart = 0;
	  rowstart = 0;
	  rowend = word->height;
	  /* Clip to extent */
	  if (x + xrun > ext_x + ext_w){
	       xrun -= x + xrun - ext_x - ext_w;
	  }
	  if (x < ext_x) {
	       int excess = ext_x - x;
	       xstart = excess - 1;
	       xrun -= excess;
	       x = ext_x;
	  }
	  if (y + rowend > ext_y + ext_h){
	      rowend -= (y - ext_y + rowend - ext_h);
	  }
	  if (y < ext_y){
	      int excess = ext_y - y;
	      rowstart += excess;
	      //rowend -= excess;
	//      y = ext_y;
	  }

#ifdef WORDCACHE
	  for (j = rowstart ; j < rowend ; j ++){
	       func(NULL, word->im + (word->roww * j) + xstart, dc->col.col,
		     im + ((y + j) * im_w) + x, xrun);
	  }
#elif defined(METRICCACHE)
	  int ind;
	  y += word->baseline;
	  for (ind = 0 ; ind < len ; ind ++){
	     // FIXME Do we need to draw?
	     struct cinfo *ci = word->cinfo + ind;
		 if ((ci->fg->ext_dat) && (dc->font_ext.func.gl_draw))
			   {
			      /* ext glyph draw */
			      dc->font_ext.func.gl_draw(dc->font_ext.data,
				    (void *)dst,
				    dc, ci->fg,
				    x + ci->pos.x,
				    y - ci->bm.h
				    );
			   }
	        else {
		     func(NULL, word->im + (word->roww * j) + xstart, dc->col.col, im + ((y + j) * im_w) + x, xrun);
	}
	  }
#endif
	  return;
     }

}
#endif


   pen_x = x;
   pen_y = y;
   prev_index = 0;
   im = dst->image.data;
   for (char_index = 0, c = 0, chr = 0; text[chr]; char_index++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y;
	int gl, kern;

	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);

	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
        if (fi->src->current_size != fi->size)
          {
             FT_Activate_Size(fi->ft.size);
             fi->src->current_size = fi->size;
          }
	/* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	  if ((use_kerning) && (prev_index) && (index) &&
	     (pface == fi->src->ft.face))
	    {
#ifdef INTERNATIONAL_SUPPORT
	       /* if it's rtl, the kerning matching should be reversed, i.e prev
		* index is now the index and the other way around. */
	       if (evas_intl_is_rtl_char(level_list, char_index))
		 {
		    if (evas_common_font_query_kerning(fi, index, prev_index, &kern))
		      pen_x += kern;
		 }
	       else
#endif
		 {

		    if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
		      pen_x += kern;
		 }
	    }

	  pface = fi->src->ft.face;
	  fg = evas_common_font_int_cache_glyph_get(fi, index);
	  LKU(fi->ft_mutex);
	  if (!fg) continue;

	  if (dc->font_ext.func.gl_new)
	    {
	       /* extension calls */
	       fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
	       fg->ext_dat_free = dc->font_ext.func.gl_free;
	    }

	  chr_x = (pen_x + (fg->glyph_out->left));
	  chr_y = (pen_y + (fg->glyph_out->top));

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
				    dc, fg,
				    chr_x,
				    y - (chr_y - y)
				    );
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
			 c++;
		      }
		 }
	    }
	  else
	    break;
	  pen_x += fg->glyph->advance.x >> 16;
	  prev_index = index;
     }
#ifdef INTERNATIONAL_SUPPORT
   if (level_list) free(level_list);
   if (visual_text) free(visual_text);
#endif
}

EAPI void
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text)
{
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;
   int use_kerning;
   RGBA_Gfx_Func func;
   RGBA_Font_Int *fi;
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;

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

#ifndef EVAS_FRAME_QUEUING
   LKL(fn->lock);
#endif
//   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst, 1, dc->render_op);

   if (!dc->cutout.rects)
     {
        evas_common_font_draw_internal(dst, dc, fn, x, y, text,
                                       func, ext_x, ext_y, ext_w, ext_h, fi,
                                       im_w, im_h, use_kerning
                                       );
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
                  evas_common_font_draw_internal(dst, dc, fn, x, y, text,
                                                 func, r->x, r->y, r->w, r->h, fi,
                                                 im_w, im_h, use_kerning
                                                 );
               }
             evas_common_draw_context_apply_clear_cutouts(rects);
          }
        dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
     }
#ifndef EVAS_FRAME_QUEUING
   LKU(fn->lock);
#endif
}



struct prword *
evas_font_word_prerender(RGBA_Draw_Context *dc, const char *text, int len, RGBA_Font *fn, RGBA_Font_Int *fi,int use_kerning){
   int pen_x, pen_y;
   struct cinfo *metrics;
   int chr;
   FT_Face pface = NULL;
   FT_UInt prev_index;
   unsigned char *im;
   int width;
   int height, above, below, baseline, descent;
   int c;
   int i,j;
   int char_index = 0; /* the index of the current char */
   struct prword *w;
   int gl;

   EINA_INLIST_FOREACH(words,w){
	if (w->len == len && w->font == fn && fi->size == w->size &&
	      (w->str == text || strcmp(w->str, text) == 0)){
	  words = eina_inlist_promote(words, EINA_INLIST_GET(w));
	  return w;
	}
   }

   gl = dc->font_ext.func.gl_new ? 1: 0;

   pen_x = pen_y = 0;
   above = 0; below = 0; baseline = 0; height = 0; descent = 0;
   metrics = malloc(sizeof(struct cinfo) * len);
   /* First pass: Work out how big */
   for (char_index = 0, c = 0, chr = 0 ; text[chr] ; char_index ++){
	struct cinfo *ci = metrics + char_index;
	ci->chr = text[chr];
	ci->gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (ci->gl == 0) break;
	ci->index = evas_common_font_glyph_search(fn, &fi, ci->gl);
	if (fi->src->current_size != fi->size)
	  {
	     FT_Activate_Size(fi->ft.size);
             fi->src->current_size = fi->size;
          }
	if (use_kerning && char_index && (pface == fi->src->ft.face))
	  {
	     int kern;
	     if (evas_common_font_query_kerning(fi, prev_index,ci->index,&kern))
		 ci->pos.x += kern;
	  }
       pface = fi->src->ft.face;
       ci->fg = evas_common_font_int_cache_glyph_get(fi, ci->index);
       if (!ci->fg) continue;
       if (gl){
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
       ci->pos.x = pen_x + ci->fg->glyph_out->left;
       ci->pos.y = pen_y + ci->fg->glyph_out->top;
       pen_x += ci->fg->glyph->advance.x >> 16;
       prev_index = ci->index;
  }

  /* First loop done */
  width = pen_x;
  width = (width & 0x7) ? width + (8 - (width & 0x7)) : width;

  height = baseline + descent;
  if (!gl){
     im = calloc(height, width);
     for (i = 0 ; i  < char_index ; i ++){
	  struct cinfo *ci = metrics + i;
	  for (j = 0 ; j < ci->bm.rows ; j ++){
	    memcpy(im + ci->pos.x + (j + baseline - ci->bm.h) * width, ci->bm.data + j * ci->bm.w, ci->bm.w);
	  }

     }
  } else {
       im = NULL;
  }
   /* Save it */
   struct prword *save;


   save = malloc(sizeof(struct prword));
   save->cinfo = metrics;
   save->str = eina_stringshare_add(text);
   save->font = fn;
   save->size = fi->size;
   save->len = len;
   save->im = im;
   save->width = pen_x;
   save->roww = width;
   save->height = height;
   save->baseline = baseline;
   words = eina_inlist_prepend(words, EINA_INLIST_GET(save));

   /* Clean up if too long */
   if (0 && eina_inlist_count(words) > 20){
	struct prword *last = (struct prword *)(words->last);
	if (last->im) free(last->im);
	if (last->cinfo) free(last->cinfo);
	eina_stringshare_del(last->str);
	words = eina_inlist_remove(words,EINA_INLIST_GET(last));
	free(last);
   }

   return save;
}



