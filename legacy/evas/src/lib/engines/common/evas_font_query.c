#include "evas_common.h"

extern FT_Library ft_lib;

/* string extents */
void
evas_common_font_query_size(RGBA_Font *fn, const char *text, int *w, int *h)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x, end_x;
   int chr;
   FT_UInt prev_index;

   start_x = 0;
   end_x = 0;
   pen_x = 0;
   pen_y = 0;
   use_kerning = FT_HAS_KERNING(fn->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl;
	
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = FT_Get_Char_Index(fn->ft.face, gl);
	if ((use_kerning) && (prev_index) && (index))
	  {
	     FT_Vector delta;
	     
	     FT_Get_Kerning(fn->ft.face, prev_index, index,
			    ft_kerning_default, &delta);
	     pen_x += delta.x << 2;
	  }
	fg = evas_common_font_cache_glyph_get(fn, index);
	if (!fg) continue;

	chr_x = (pen_x >> 8) + fg->glyph_out->left;
	chr_y = (pen_y >> 8)  + fg->glyph_out->top;
	chr_w = fg->glyph_out->bitmap.width;
	
	if (chr_x < start_x) start_x = chr_x;
	if ((chr_x + chr_w) > end_x) end_x = chr_x + chr_w;
	
	pen_x += fg->glyph->advance.x >> 8;
	prev_index = index;	
     }
   if (w) *w = end_x - start_x;
   if (h) *h = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
}

/* text x inset */
int
evas_common_font_query_inset(RGBA_Font *fn, const char *text)
{
   FT_UInt index;
   RGBA_Font_Glyph *fg;
   int chr;
   int gl;
   
   chr = 0;
   if (!text[0]) return 0;
   gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);   
   if (gl == 0) return 0;
   index = FT_Get_Char_Index(fn->ft.face, gl);
   fg = evas_common_font_cache_glyph_get(fn, index);
   if (!fg) return 0;
   return fg->glyph_out->left;
}

/* h & v advance */
void
evas_common_font_query_advance(RGBA_Font *fn, const char *text, int *h_adv, int *v_adv)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x;
   int chr;
   FT_UInt prev_index;
   
   start_x = 0;
   pen_x = 0;
   pen_y = 0;
   use_kerning = FT_HAS_KERNING(fn->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)     
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl;
	
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = FT_Get_Char_Index(fn->ft.face, gl);
	if ((use_kerning) && (prev_index) && (index))
	  {
	     FT_Vector delta;
	     
	     FT_Get_Kerning(fn->ft.face, prev_index, index,
			    ft_kerning_default, &delta);
	     pen_x += delta.x << 2;
	  }
	fg = evas_common_font_cache_glyph_get(fn, index);
	if (!fg) continue;
	
	chr_x = (pen_x >> 8) + fg->glyph_out->left;
	chr_y = (pen_y >> 8) + fg->glyph_out->top;
	chr_w = fg->glyph_out->bitmap.width;
	
	pen_x += fg->glyph->advance.x >> 8;
	prev_index = index;
     }
   if (v_adv) *v_adv = evas_common_font_get_line_advance(fn);
   if (h_adv) *h_adv = (pen_x >> 8) - start_x;
}

/* x y w h for char at char pos */
int
evas_common_font_query_char_coords(RGBA_Font *fn, const char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int chr;
   FT_UInt prev_index;
   
   pen_x = 0;
   pen_y = 0;
   use_kerning = FT_HAS_KERNING(fn->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;	
        int gl;
	
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = FT_Get_Char_Index(fn->ft.face, gl);
	if ((use_kerning) && (prev_index) && (index))
	  {
	     FT_Vector delta;
	     
	     FT_Get_Kerning(fn->ft.face, prev_index, index,
			    ft_kerning_default, &delta);
	     pen_x += delta.x << 2;
	  }
	fg = evas_common_font_cache_glyph_get(fn, index);
	if (!fg) continue;
	
	chr_x = (pen_x >> 8) + fg->glyph_out->left;
	chr_y = (pen_y >> 8) + fg->glyph_out->top;
	chr_w = fg->glyph_out->bitmap.width;
	
	if (gl == pos)
	  {
	     if (cx) *cx = chr_x;
	     if (cy) *cy = - evas_common_font_max_ascent_get(fn);
	     if (cw) *cw = chr_w;
	     if (ch) *ch = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
	     return 1;
	  }
	pen_x += fg->glyph->advance.x >> 8;
	prev_index = index;
     }
   return 0;
}

/* char pos of text at xy pos */
int
evas_common_font_query_text_at_pos(RGBA_Font *fn, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int chr;
   FT_UInt prev_index;
   
   pen_x = 0;
   pen_y = 0;
   use_kerning = FT_HAS_KERNING(fn->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl;
	
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = FT_Get_Char_Index(fn->ft.face, gl);
	if ((use_kerning) && (prev_index) && (index))
	  {
	     FT_Vector delta;
	     
	     FT_Get_Kerning(fn->ft.face, prev_index, index,
			    ft_kerning_default, &delta);
	     pen_x += delta.x << 6;
	  }
	fg = evas_common_font_cache_glyph_get(fn, index);
	if (!fg) continue;
	
	chr_x = (pen_x >> 8) + fg->glyph_out->left;
	chr_y = (pen_y >> 8) + fg->glyph_out->top;
	chr_w = fg->glyph_out->bitmap.width;
	
	if ((x >= chr_x) && (x < (chr_x + chr_w)) &&
	    (y > - evas_common_font_max_ascent_get(fn)) &&
	    (y < evas_common_font_max_descent_get(fn)))
	  {
	     if (cx) *cx = chr_x;
	     if (cy) *cy = - evas_common_font_max_ascent_get(fn);
	     if (cw) *cw = chr_w;
	     if (ch) *ch = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
	     return gl;
	  }
	pen_x += fg->glyph->advance.x >> 8;
	prev_index = index;
     }
   return -1;
}
