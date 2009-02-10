#include "evas_common.h"

EAPI int
evas_common_font_query_kerning(RGBA_Font_Int* fi,
			       FT_UInt prev, FT_UInt index,
			       int* kerning)
{
   int *result;
   FT_Vector delta;
   int key[2];
   int error = 1;

   key[0] = prev;
   key[1] = index;

#ifdef HAVE_PTHREAD
   pthread_mutex_lock(&fi->ft_mutex);
#endif

   result = eina_hash_find(fi->kerning, key);
   if (result)
     {
	*kerning = result[2];
	goto on_correct;
     }

   /* NOTE: ft2 seems to have a bug. and sometimes returns bizarre
    * values to kern by - given same font, same size and same
    * prev_index and index. auto/bytecode or none hinting doesnt
    * matter */
   if (FT_Get_Kerning(fi->src->ft.face,
		      key[0], key[1],
		      ft_kerning_default, &delta) == 0)
     {
	int *push;

	*kerning = delta.x >> 6;

	push = malloc(sizeof (int) * 3);
	if (!push) return 1;

	push[0] = key[0];
	push[1] = key[1];
	push[2] = *kerning;

	eina_hash_direct_add(fi->kerning, push, push);

	goto on_correct;
     }

   error = 0;

 on_correct:
#ifdef HAVE_PTHREAD
   pthread_mutex_unlock(&fi->ft_mutex);
#endif
   return error;
}

/* string extents */
EAPI void
evas_common_font_query_size(RGBA_Font *fn, const char *text, int *w, int *h)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x, end_x;
   int chr;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   start_x = 0;
   end_x = 0;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl, kern;

	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	kern = 0;
	if ((use_kerning) && (prev_index) && (index) &&
	    (pface == fi->src->ft.face))
	  if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
	    pen_x += kern;

	pface = fi->src->ft.face;
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	if (!fg) continue;

	if (kern < 0) kern = 0;
	chr_x = ((pen_x - kern) + (fg->glyph_out->left));
	chr_y = (pen_y + (fg->glyph_out->top));
//	chr_w = fg->glyph_out->bitmap.width;
	chr_w = fg->glyph_out->bitmap.width + kern;
	  {
	     int advw;

	     advw = ((fg->glyph->advance.x + (kern << 16)) >> 16);
	     if (chr_w < advw) chr_w = advw;
	  }

	if ((!prev_index) && (chr_x < 0))
	  start_x = chr_x;
	if ((chr_x + chr_w) > end_x)
	  end_x = chr_x + chr_w;

	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   if (w) *w = end_x - start_x;
   if (h) *h = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
}

/* text x inset */
EAPI int
evas_common_font_query_inset(RGBA_Font *fn, const char *text)
{
   FT_UInt index;
   RGBA_Font_Glyph *fg;
   int chr;
   int gl;
   RGBA_Font_Int *fi;

   fi = fn->fonts->data;

   chr = 0;
   if (!text[0]) return 0;
   gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
   if (gl == 0) return 0;
   evas_common_font_size_use(fn);
   index = evas_common_font_glyph_search(fn, &fi, gl);
   fg = evas_common_font_int_cache_glyph_get(fi, index);
   if (!fg) return 0;
/*   
   printf("fg->glyph_out->left = %i\n"
	  "fi->src->ft.face->glyph->bitmap_left = %i\n"
	  "fi->src->ft.face->glyph->metrics.horiBearingX = %i\n"
	  "fi->src->ft.face->glyph->metrics.horiBearingY = %i\n"
	  "fi->src->ft.face->glyph->metrics.horiAdvance = %i\n"
	  ,
	  (int)fg->glyph_out->left, 
	  (int)fi->src->ft.face->glyph->bitmap_left,
	  (int)fi->src->ft.face->glyph->metrics.horiBearingX >> 6,
	  (int)fi->src->ft.face->glyph->metrics.horiBearingY >> 6,
	  (int)fi->src->ft.face->glyph->metrics.horiAdvance >> 6
	  );
 */
   return fg->glyph_out->left;
}

/* h & v advance */
EAPI void
evas_common_font_query_advance(RGBA_Font *fn, const char *text, int *h_adv, int *v_adv)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x;
   int chr;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   start_x = 0;
   pen_x = 0;
   pen_y = 0;
   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   for (chr = 0; text[chr];)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
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

        chr_x = (pen_x + (fg->glyph_out->left));
	chr_y = (pen_y + (fg->glyph_out->top));
	chr_w = fg->glyph_out->bitmap.width;

	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   if (v_adv) *v_adv = evas_common_font_get_line_advance(fn);
   if (h_adv) *h_adv = pen_x - start_x;
}

/* x y w h for char at char pos */
EAPI int
evas_common_font_query_char_coords(RGBA_Font *fn, const char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int prev_chr_end;
   int chr;
   int asc, desc;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   prev_chr_end = 0;
   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
   for (chr = 0; text[chr];)
     {
	int pchr;
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl, kern;

	pchr = chr;
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	kern = 0;
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

	if (kern < 0) kern = 0;
        chr_x = ((pen_x - kern) + (fg->glyph_out->left));
	chr_y = (pen_y + (fg->glyph_out->top));
	chr_w = fg->glyph_out->bitmap.width + (kern);
/*	if (text[chr]) */
	  {
	     int advw;

	     advw = ((fg->glyph->advance.x + (kern << 16)) >> 16);
	     if (chr_w < advw) chr_w = advw;
	  }
	if (chr_x > prev_chr_end)
	  {
	     chr_w += (chr_x - prev_chr_end);
	     chr_x = prev_chr_end;
	  }
	if (pchr == pos)
	  {
	     if (cx) *cx = chr_x;
	     if (cy) *cy = -asc;
	     if (cw) *cw = chr_w;
	     if (ch) *ch = asc + desc;
	     return 1;
	  }
	prev_chr_end = chr_x + chr_w;
	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   return 0;
}

/* char pos of text at xy pos */
EAPI int
evas_common_font_query_text_at_pos(RGBA_Font *fn, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int prev_chr_end;
   int chr;
   int asc, desc;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   prev_chr_end = 0;
   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
   for (chr = 0; text[chr];)
     {
	int pchr;
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl, kern;

	pchr = chr;
	gl = evas_common_font_utf8_get_next((unsigned char *)text, &chr);
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	kern = 0;
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

	if (kern < 0) kern = 0;
        chr_x = ((pen_x - kern) + (fg->glyph_out->left));
	chr_y = (pen_y + (fg->glyph_out->top));
	chr_w = fg->glyph_out->bitmap.width + kern;
/*	if (text[chr]) */
	  {
	     int advw;

	     advw = ((fg->glyph->advance.x + (kern << 16)) >> 16);
	     if (chr_w < advw) chr_w = advw;
	  }
	if (chr_x > prev_chr_end)
	  {
	     chr_w += (chr_x - prev_chr_end);
	     chr_x = prev_chr_end;
	  }
	if ((x >= chr_x) && (x <= (chr_x + chr_w)) &&
	    (y >= -asc) && (y <= desc))
	  {
	     if (cx) *cx = chr_x;
	     if (cy) *cy = -asc;
	     if (cw) *cw = chr_w;
	     if (ch) *ch = asc + desc;
	     return pchr;
	  }
	prev_chr_end = chr_x + chr_w;
	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   return -1;
}
