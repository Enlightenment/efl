#include "evas_common.h"

RGBA_Font_Glyph *
evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt index)
{
   RGBA_Font_Glyph *fg;
   char key[6];   
   FT_Error error;

   key[0] = ((index       ) & 0x7f) + 1;
   key[1] = ((index >> 7  ) & 0x7f) + 1;
   key[2] = ((index >> 14 ) & 0x7f) + 1;
   key[3] = ((index >> 21 ) & 0x7f) + 1;
   key[4] = ((index >> 28 ) & 0x0f) + 1;
   key[5] = 0;

   fg = evas_hash_find(fi->glyphs, key);
   if (fg) return fg;
   
   error = FT_Load_Glyph(fi->src->ft.face, index, FT_LOAD_NO_BITMAP);
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
   if (fg->glyph->format != ft_glyph_format_bitmap)
     {
	error = FT_Glyph_To_Bitmap(&(fg->glyph), ft_render_mode_normal, 0, 1);
	if (error) 
	  {
	     FT_Done_Glyph(fg->glyph);
	     free(fg);
	     return NULL;
	  }
     }
   fg->glyph_out = (FT_BitmapGlyph)fg->glyph;
   
   fi->glyphs = evas_hash_add(fi->glyphs, key, fg);
   return fg;
}

int
evas_common_font_glyph_search(RGBA_Font *fn, RGBA_Font_Int **fi_ret, int gl)
{
   Evas_List *l;
   
   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;
	int index;
	
	fi = l->data;
        index = FT_Get_Char_Index(fi->src->ft.face, gl);
	if (index != 0)
	  {
	     *fi_ret = fi;
	     return index;
	  }
     }
   return 0;
}

void
evas_common_font_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text)
{
   int use_kerning;
   int pen_x, pen_y;
   int chr;
   FT_UInt prev_index;
   Gfx_Func_Blend_Src_Alpha_Mul_Dst func;
   int ext_x, ext_y, ext_w, ext_h;
   DATA32 *im;
   int im_w, im_h;
   int c;
   RGBA_Font_Int *fi;

   fi = fn->fonts->data;
   
   im = dst->image->data;
   im_w = dst->image->w;
   im_h = dst->image->h;
   
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
   
   pen_x = x << 8;
   pen_y = y << 8;
   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   func = evas_common_draw_func_blend_alpha_get(dst);
   for (c = 0, chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y;
	int gl;
	
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	/* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	if ((use_kerning) && (prev_index) && (index))
	  {
	     FT_Vector delta;
	     
	     FT_Get_Kerning(fi->src->ft.face, prev_index, index,
			    ft_kerning_default, &delta);
	     pen_x += delta.x << 2;
	  }
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	if (!fg) continue;
	
	if ((dc->font_ext.func.gl_new) && (!fg->ext_dat))
	  {
	     /* extension calls */
	     fg->ext_dat = dc->font_ext.func.gl_new(dc->font_ext.data, fg);
	     fg->ext_dat_free = dc->font_ext.func.gl_free;
	  }

	chr_x = (pen_x + (fg->glyph_out->left << 8)) >> 8;
	chr_y = (pen_y + (fg->glyph_out->top << 8)) >> 8;
	
	if (chr_x < (ext_x + ext_w))
	  {
	     DATA8 *data;
	     int i, j, w, h;
	     
	     data = fg->glyph_out->bitmap.buffer;
	     j = fg->glyph_out->bitmap.pitch;
	     w = fg->glyph_out->bitmap.width;
	     if (j < w) j = w;
	     h = fg->glyph_out->bitmap.rows;
	     if ((fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays) &&
		 (fg->glyph_out->bitmap.num_grays == 256))
	       {
		  if ((j > 0) && (chr_x + w > ext_x))
		    {
		       if (fg->ext_dat)
			 {
			    /* ext glyph draw */
			    dc->font_ext.func.gl_draw(dc->font_ext.data, 
						      (void *)c,
						      dc, fg, 
						      chr_x, 
						      y - (chr_y - y)
						      );
			 }
		       else
			 {
			    for (i = 0; i < h; i++)
			      {
				 int dx, dy;
				 int in_x, in_w;
				 
				 in_x = 0;
				 in_w = 0;
				 dx = chr_x;
				 dy = y - (chr_y - i - y);
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
					   func(data + (i * j) + in_x, 
						im + (dy * im_w) + dx,
						w - in_w, 
						dc->col.col);
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
	pen_x += fg->glyph->advance.x >> 8;
	prev_index = index;	
     }
}
