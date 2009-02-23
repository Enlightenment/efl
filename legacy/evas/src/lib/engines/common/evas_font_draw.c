/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

EAPI RGBA_Font_Glyph *
evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt index)
{
   RGBA_Font_Glyph *fg;
   FT_UInt hindex;
   FT_Error error;
   const FT_Int32 hintflags[3] =
     { FT_LOAD_NO_HINTING, FT_LOAD_FORCE_AUTOHINT, FT_LOAD_NO_AUTOHINT };

   hindex = index + (fi->hinting * 500000000);

   fg = eina_hash_find(fi->glyphs, &hindex);
   if (fg) return fg;

//   error = FT_Load_Glyph(fi->src->ft.face, index, FT_LOAD_NO_BITMAP);
   error = FT_Load_Glyph(fi->src->ft.face, index,
			 FT_LOAD_RENDER | hintflags[fi->hinting]);
   if (error) return NULL;

   fg = malloc(sizeof(struct _RGBA_Font_Glyph));
   if (!fg) return NULL;
   memset(fg, 0, (sizeof(struct _RGBA_Font_Glyph)));

   error = FT_Get_Glyph(fi->src->ft.face->glyph, &(fg->glyph));
   if (error)
     {
	free(fg);
	return NULL;
     }
   if (fg->glyph->format != FT_GLYPH_FORMAT_BITMAP)
     {
	error = FT_Glyph_To_Bitmap(&(fg->glyph), FT_RENDER_MODE_NORMAL, 0, 1);
	if (error)
	  {
	     FT_Done_Glyph(fg->glyph);
	     free(fg);
	     return NULL;
	  }
     }
   fg->glyph_out = (FT_BitmapGlyph)fg->glyph;

   eina_hash_add(fi->glyphs, &hindex, fg);
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
   Font_Char_Index *result;

#ifdef HAVE_PTHREAD
   pthread_mutex_lock(&fi->ft_mutex);
#endif

   result = eina_hash_find(fi->indexes, &gl);
   if (result) goto on_correct;

   result = malloc(sizeof (Font_Char_Index));
   if (!result)
     {
#ifdef HAVE_PTHREAD
	pthread_mutex_unlock(&fi->ft_mutex);
#endif
	return FT_Get_Char_Index(fi->src->ft.face, gl);
     }

   result->index = FT_Get_Char_Index(fi->src->ft.face, gl);
   result->gl = gl;

   eina_hash_direct_add(fi->indexes, &result->gl, result);

 on_correct:
#ifdef HAVE_PTHREAD
   pthread_mutex_unlock(&fi->ft_mutex);
#endif
   return result->index;
}

EAPI int
evas_common_font_glyph_search(RGBA_Font *fn, RGBA_Font_Int **fi_ret, int gl)
{
   Eina_List *l;

   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;
	int index;

	fi = l->data;

	if (fi->src->charmap) /* Charmap loaded, FI/FS blank */
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
	else if (!fi->src->ft.face) /* Charmap not loaded, FI/FS blank */
	  {
	     if (evas_common_font_source_load_complete(fi->src))
	       return 0;
#if 0 /* FIXME: disable this. this can eat a LOT of memory and in my tests with expedite at any rate shows no visible improvements */
	     index = FT_Get_Char_Index(fi->src->ft.face, gl);
	     if (index == 0)
	       {
		  /* Load Hash */
		  FT_ULong  charcode;
		  FT_UInt   gindex;

		  fi->src->charmap = evas_array_hash_new();
		  charcode = FT_Get_First_Char(fi->src->ft.face, &gindex);
		  while (gindex != 0)
		    {
		       evas_array_hash_add(fi->src->charmap, charcode, gindex);
		       charcode = FT_Get_Next_Char(fi->src->ft.face, charcode, &gindex);
		    }

		  /* Free face */
		  FT_Done_Face(fi->src->ft.face);
		  fi->src->ft.face = NULL;
	       }
	     else
	       {
		  evas_common_font_int_load_complete(fi);

		  *fi_ret = fi;
		  return index;
	       }
#endif
	  }
	else /* Charmap not loaded, FS loaded */
	  {
	     index = _evas_common_get_char_index(fi, gl);
	     if (index != 0)
	       {
		  if (!fi->ft.size)
		    evas_common_font_int_load_complete(fi);

		  *fi_ret = fi;
		  return index;
	       }
	  }
     }
   return 0;
}

static void
evas_common_font_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text,
                               RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, int ext_h, RGBA_Font_Int *fi,
                               int im_w, int im_h, int use_kerning
                               )
{
   int pen_x, pen_y;
   int chr;
   FT_Face pface = NULL;
   FT_UInt prev_index;
   DATA32 *im;
   int c;

   pen_x = x;
   pen_y = y;
   prev_index = 0;
   im = dst->image.data;
   for (c = 0, chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y;
	int gl, kern;

	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	/* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
        if ((use_kerning) && (prev_index) && (index) &&
	    (pface == fi->src->ft.face))
	  if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
	    pen_x += kern;

	pface = fi->src->ft.face;
	fg = evas_common_font_int_cache_glyph_get(fi, index);
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

   LKL(fn->lock);
   evas_common_font_size_use(fn);
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
   LKU(fn->lock);
}
