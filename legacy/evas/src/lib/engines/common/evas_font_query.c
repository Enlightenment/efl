#include "evas_common.h"
#include "evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */

EAPI int
evas_common_font_query_kerning(RGBA_Font_Int* fi,
			       FT_UInt left, FT_UInt right,
			       int* kerning)
{
   int *result;
   FT_Vector delta;
   int key[2];
   int error = 1;

//   return 0;
   key[0] = left;
   key[1] = right;

   result = eina_hash_find(fi->kerning, key);
   if (result)
     {
	*kerning = result[2];
	goto on_correct;
     }

   /* NOTE: ft2 seems to have a bug. and sometimes returns bizarre
    * values to kern by - given same font, same size and same
    * prev_index and index. auto/bytecode or none hinting doesn't
    * matter */
   evas_common_font_int_reload(fi);
   FTLOCK();
   if (FT_Get_Kerning(fi->src->ft.face,
		      key[0], key[1],
		      ft_kerning_default, &delta) == 0)
     {
	int *push;

        FTUNLOCK();
	*kerning = delta.x >> 6;

	push = malloc(sizeof (int) * 3);
	if (!push) return 1;

	push[0] = key[0];
	push[1] = key[1];
	push[2] = *kerning;

	eina_hash_direct_add(fi->kerning, push, push);

	goto on_correct;
     }

        FTUNLOCK();
   error = 0;

 on_correct:
   return error;
}

/* size of the string (width and height) in pixels
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * We only care about the size, and the size does not depend on the visual order.
 * As long as we follow the logical string and get kerning data like we should,
 * we are fine.
 */

EAPI void
evas_common_font_query_size(RGBA_Font *fn, const Eina_Unicode *text, const Evas_BiDi_Props *intl_props __UNUSED__, int *w, int *h)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x, end_x;
   int chr;
   int char_index;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   start_x = 0;
   end_x = 0;

   pen_x = 0;
   pen_y = 0;
//   evas_common_font_size_use(fn);
   evas_common_font_int_reload(fi);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   for (chr = 0, char_index = 0; *text; text++, char_index ++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg = NULL;
	int chr_x, chr_y, advw;
        int gl, kern;

	gl = *text;
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
        if (fi->src->current_size != fi->size)
          {
	     FTLOCK();
             FT_Activate_Size(fi->ft.size);
	     FTUNLOCK();
             fi->src->current_size = fi->size;
          }
	kern = 0;
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	if ((use_kerning) && (prev_index) && (index) && (fg) &&
	     (pface == fi->src->ft.face))
	   {
#ifdef BIDI_SUPPORT
              /* if it's rtl, the kerning matching should be reversed, i.e prev
               * index is now the index and the other way around. 
               * There is a slight exception when there are compositing chars
               * involved.*/
              if (intl_props && 
                  evas_bidi_is_rtl_char(intl_props, char_index) &&
                  ((fg->glyph->advance.x >> 16) > 0))
                {
                   if (evas_common_font_query_kerning(fi, index, prev_index, &kern))
                      pen_x += kern;
                }
              else
                {
                   if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                      pen_x += kern;
                }
#else                 
              if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                 pen_x += kern;
#endif
           }

	pface = fi->src->ft.face;
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	LKU(fi->ft_mutex);
	if (!fg || !fg->glyph) continue;

	if (kern < 0) kern = 0;

        /* We care about advancing the whole string size, and not the actual
         * paint size of each string, so we only care about advancing correctly
         * and not the actual glyph width */
        advw = ((fg->glyph->advance.x + (kern << 16)) >> 16);
        chr_x = pen_x - kern;
	chr_y = pen_y;
        /* If it's not a compositing char, i.e it advances, we should also add
         * the left/top padding of the glyph. As we don't care about the padding
         * as the drawing location remains the same.
         */
        if (advw > 0)
          {
             chr_x += fg->glyph_out->left;
	     chr_y += fg->glyph_out->top;
          }


	if ((!prev_index) && (chr_x < 0))
	  start_x = chr_x;
	if ((chr_x + advw) > end_x)
	  end_x = chr_x + advw;

	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   if (w) *w = end_x - start_x;
   if (h) *h = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
  evas_common_font_int_use_trim();
}

/* text x inset */
EAPI int
evas_common_font_query_inset(RGBA_Font *fn, const Eina_Unicode *text)
{
   FT_UInt index;
   RGBA_Font_Glyph *fg;
   int gl;
   RGBA_Font_Int *fi;

   fi = fn->fonts->data;

   if (!*text) return 0;
   gl = *text;
   if (gl == 0) return 0;
//   evas_common_font_size_use(fn);
   index = evas_common_font_glyph_search(fn, &fi, gl);
   LKL(fi->ft_mutex);
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   fg = evas_common_font_int_cache_glyph_get(fi, index);
   LKU(fi->ft_mutex);
   if (!fg) return 0;
/*
   INF("fg->glyph_out->left = %i, "
	  "fi->src->ft.face->glyph->bitmap_left = %i, "
	  "fi->src->ft.face->glyph->metrics.horiBearingX = %i, "
	  "fi->src->ft.face->glyph->metrics.horiBearingY = %i, "
	  "fi->src->ft.face->glyph->metrics.horiAdvance = %i"
	  ,
	  (int)fg->glyph_out->left,
	  (int)fi->src->ft.face->glyph->bitmap_left,
	  (int)fi->src->ft.face->glyph->metrics.horiBearingX >> 6,
	  (int)fi->src->ft.face->glyph->metrics.horiBearingY >> 6,
	  (int)fi->src->ft.face->glyph->metrics.horiAdvance >> 6
	  );
 */
  evas_common_font_int_use_trim();
   return fg->glyph_out->left;
}

/* h & v advance
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * We don't care about the order, as heights will remain the same (we already did
 * shaping) and as long as we go through the logical string and match the kerning
 * this way, we are safe.
 */

EAPI void
evas_common_font_query_advance(RGBA_Font *fn, const Eina_Unicode *text, const Evas_BiDi_Props *intl_props, int *h_adv, int *v_adv)
{
   int use_kerning;
   int pen_x, pen_y;
   int start_x;
   int char_index;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   start_x = 0;
   pen_x = 0;
   pen_y = 0;
//   evas_common_font_size_use(fn);
   evas_common_font_int_reload(fi);
   FTLOCK();
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   FTUNLOCK();
   prev_index = 0;
   for (char_index = 0 ; *text ; text++, char_index++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
        int gl, kern;

	gl = *text;
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
        if (fi->src->current_size != fi->size)
          {
	     FTLOCK();
             FT_Activate_Size(fi->ft.size);
	     FTUNLOCK();
             fi->src->current_size = fi->size;
          }
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	if (!fg) 
          {
             LKU(fi->ft_mutex);
             continue;
          }
	// FIXME: Why no FT_Activate_Size here ?
	kern = 0;
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	if ((use_kerning) && (prev_index) && (index) && (fg) &&
	     (pface == fi->src->ft.face))
	   {
#ifdef BIDI_SUPPORT
              /* if it's rtl, the kerning matching should be reversed, i.e prev
               * index is now the index and the other way around. 
               * There is a slight exception when there are compositing chars
               * involved.*/
              if (intl_props && 
                    evas_bidi_is_rtl_char(intl_props, char_index) &&
                    fg->glyph->advance.x >> 16 > 0)
                {
                   if (evas_common_font_query_kerning(fi, index, prev_index, &kern))
                     pen_x += kern;
                }
              else
                {
                   if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                     pen_x += kern;
                }
#else              
              if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                 pen_x += kern;
#endif
           }

	pface = fi->src->ft.face;
	LKU(fi->ft_mutex);

	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
   if (v_adv) *v_adv = evas_common_font_get_line_advance(fn);
   if (h_adv) *h_adv = pen_x - start_x;
#ifndef BIDI_SUPPORT
   intl_props = NULL;
#endif
  evas_common_font_int_use_trim();
}

/* x y w h for char at char pos for null it returns the position right after
 * the last char with 0 as width and height.
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * We care about the actual drawing location of the string, this is why we need
 * the visual string. We need to know how it's printed. After that we need to calculate
 * the reverse kerning in case of rtl parts. "pos" passed to this function is an
 * index in bytes, that is the actual byte location of the string, we need to find
 * the index in order to find it in the visual string.
 */

EAPI int
evas_common_font_query_char_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_BiDi_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int prev_chr_end;
   int asc, desc;
   int char_index = 0; /* the index of the current char */
   int position = 0;
   const Eina_Unicode *text = in_text;
   int ret_val = 0;
   int last_adv;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

#ifdef BIDI_SUPPORT
   int len = 0;
   EvasBiDiStrIndex *visual_to_logical = NULL;
   Eina_Unicode *visual_text;

   visual_text = eina_unicode_strdup(in_text);
   if (visual_text)
     {
        evas_bidi_props_reorder_line(visual_text, intl_props, &visual_to_logical);
        text = visual_text;
     }
   else
     {
        text = in_text;
     }
   len = eina_unicode_strlen(text);
#endif

   fi = fn->fonts->data;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_int_reload(fi);
//   evas_common_font_size_use(fn);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   prev_chr_end = 0;
   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

#ifdef BIDI_SUPPORT
   /* Get the position in the visual string because those are the coords we care about */
   position = evas_bidi_position_logical_to_visual(visual_to_logical, len, pos);
#else
   position = pos;
#endif
   /* If it's the null, choose location according to the direction. */
   if (!text[position])
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (evas_bidi_is_rtl_char(intl_props, 0))
          {
             if (cx) *cx = 0;
             if (ch) *ch = asc + desc;
          }
        else
#endif
          {
             evas_common_font_query_size(fn, text, intl_props, cx, ch);
          }
        if (cy) *cy = 0;
        if (cw) *cw = 0;
        ret_val = 1;
        goto end;
     }

   last_adv = 0;
   for (char_index = 0; *text ; text++, char_index++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
   int gl, kern;

	gl = *text;
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	if (!fg) 
          {
             LKU(fi->ft_mutex);
             continue;
          }
	// FIXME: Why no FT_Activate_Size here ?
	kern = 0;
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	if ((use_kerning) && (prev_index) && (index) &&
	     (pface == fi->src->ft.face))
	   {
              if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                pen_x += kern;
           }

	pface = fi->src->ft.face;
	LKU(fi->ft_mutex);
	/* If the current one is not a compositing char, do the previous advance
	 * and set the current advance as the next advance to do */
	if (fg->glyph->advance.x >> 16 > 0) 
	  {
	     pen_x += last_adv;
	     last_adv = fg->glyph->advance.x >> 16;
	  }
	if (kern < 0) kern = 0;
        chr_x = (pen_x - kern) + (fg->glyph_out->left);
	chr_y = (pen_y) + (fg->glyph_out->top);
        chr_w = fg->glyph_out->bitmap.width + (kern);
/*	if (text[chr]) */
	  {
	     int advw;
	     advw = ((fg->glyph->advance.x + (kern << 16)) >> 16);
	     if (chr_w < advw) chr_w = advw;
	  }
#if 0 /* This looks like a hack, we don't want it. - leaving it here in case
       * I'm wrong */
	if (chr_x > prev_chr_end)
	  {
	     chr_w += (chr_x - prev_chr_end);
	     chr_x = prev_chr_end;
	  }
#endif
	/* we need to see if the char at the visual position is the char wanted */
	if (char_index == position)
	  { 
	     if (cx) *cx = chr_x;
	     if (cy) *cy = -asc;
	     if (cw) *cw = chr_w;
	     if (ch) *ch = asc + desc;
	     ret_val = 1;
	     goto end;
	  }
	prev_chr_end = chr_x + chr_w;
	prev_index = index;
     }
end:

#ifdef BIDI_SUPPORT
   if (visual_to_logical) free(visual_to_logical);
   if (visual_text) free(visual_text);
#endif

  evas_common_font_int_use_trim();
   return ret_val;
}

/* char pos of text at xy pos
 * BiDi handling: Since we are looking for the char at the specific coords,
 * we have to get the visual string (to which the coords relate to), do
 * reverse kerning query because we are working on the visual string, and then
 * we need to get the logical position of the char we found from the visual string.
 */

EAPI int
evas_common_font_query_char_at_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_BiDi_Props *intl_props, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int use_kerning;
   int pen_x, pen_y;
   int prev_chr_end;
   int asc, desc;
   int char_index = 0; /* the index of the current char */
   const Eina_Unicode *text = in_text;
   int last_adv;
   int ret_val = -1;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

#ifdef BIDI_SUPPORT
   int len = 0;
   EvasBiDiStrIndex *visual_to_logical = NULL;
   Eina_Unicode *visual_text;

   visual_text = eina_unicode_strdup(in_text);

   if (visual_text)
     {
        evas_bidi_props_reorder_line(visual_text, intl_props, &visual_to_logical);
        text = visual_text;
     }
   else
     {
        text = in_text;
     }
   len = eina_unicode_strlen(text);
#endif

   fi = fn->fonts->data;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_int_reload(fi);
//   evas_common_font_size_use(fn);
   if (fi->src->current_size != fi->size)
     {
	FTLOCK();
        FT_Activate_Size(fi->ft.size);
	FTUNLOCK();
        fi->src->current_size = fi->size;
     }
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   last_adv = 0;
   prev_index = 0;
   prev_chr_end = 0;
   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   for (char_index = 0; *text; text++, char_index++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg;
	int chr_x, chr_y, chr_w;
        int gl, kern;

	gl = *text;
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	if (!fg) 
          {
             LKU(fi->ft_mutex);
             continue;
          }
           
	// FIXME: Why not FT_Activate_Size here ?
	kern = 0;
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
	/* grrr - this means font face sharing is kinda... not an option if */
	/* you want performance */
	if ((use_kerning) && (prev_index) && (index) &&
	     (pface == fi->src->ft.face))
	   {
              if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                pen_x += kern;
           }
	pface = fi->src->ft.face;
	LKU(fi->ft_mutex);
	/* If the current one is not a compositing char, do the previous advance 
	 * and set the current advance as the next advance to do */
	if (fg->glyph->advance.x >> 16 > 0) 
	  {
	     pen_x += last_adv;
	     last_adv = fg->glyph->advance.x >> 16;
	  }         
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
#if 0 /* This looks like a hack, we don't want it. - leaving it here in case
       * I'm wrong */
	if (chr_x > prev_chr_end)
	  {
	     chr_w += (chr_x - prev_chr_end);
	     chr_x = prev_chr_end;
	  }
#endif
	if ((x >= chr_x) && (x <= (chr_x + chr_w)) &&
	    (y >= -asc) && (y <= desc))
	  {
             int position = char_index;
	     if (cx) *cx = chr_x;
	     if (cy) *cy = -asc;
	     if (cw) *cw = chr_w;
	     if (ch) *ch = asc + desc;
#ifdef BIDI_SUPPORT
             /* we found the char position of the wanted char in the
              * visual string, we now need to translate it to the
              * position in the logical string */
             position = evas_bidi_position_visual_to_logical(visual_to_logical, position);
#endif
	     ret_val = position;
	     goto end;
	  }
	prev_chr_end = chr_x + chr_w;
	prev_index = index;
     }
     
end:
   
#ifdef BIDI_SUPPORT
   if (visual_to_logical) free(visual_to_logical);
   if (visual_text) free(visual_text);
#else
   intl_props = NULL;
#endif

  evas_common_font_int_use_trim();
   return ret_val;
}

/* position of the last char in thext text that will fit in xy.
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * All we care about is char sizes + kerning so we only really need to get the
 * shaped string to utf8, and then just go through it like in english, as it's
 * just the logical string, nothing special about that.
 */

EAPI int
evas_common_font_query_last_up_to_pos(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_BiDi_Props *intl_props __UNUSED__, int x, int y)
{
   int use_kerning;
   int pen_x, pen_y;
   int prev_chr_end;
   int char_index;
   int asc, desc;
   int ret=-1;
   const Eina_Unicode *text = in_text;
   FT_UInt prev_index;
   RGBA_Font_Int *fi;
   FT_Face pface = NULL;

   fi = fn->fonts->data;

   pen_x = 0;
   pen_y = 0;
   evas_common_font_int_reload(fi);
//   evas_common_font_size_use(fn);
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   prev_index = 0;
   prev_chr_end = 0;
   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
   for (char_index = 0; *text; text++, char_index++)
     {
	FT_UInt index;
	RGBA_Font_Glyph *fg = NULL;
	int chr_x, chr_y, chr_w;
        int gl, kern;

	gl = *text;
	if (gl == 0) break;
	index = evas_common_font_glyph_search(fn, &fi, gl);
	LKL(fi->ft_mutex);
        if (fi->src->current_size != fi->size)
          {
	     FTLOCK();
             FT_Activate_Size(fi->ft.size);
	     FTUNLOCK();
             fi->src->current_size = fi->size;
          }
        kern = 0;
        /* hmmm kerning means i can't sanely do my own cached metric tables! */
        /* grrr - this means font face sharing is kinda... not an option if */
        /* you want performance */
	if ((use_kerning) && (prev_index) && (index) && (fg) &&
            (pface == fi->src->ft.face))
          {
#ifdef BIDI_SUPPORT
             /* if it's rtl, the kerning matching should be reversed, i.e prev
              * index is now the index and the other way around.
              * There is a slight exception when there are compositing chars
              * involved.*/
             if (intl_props &&
                   evas_bidi_is_rtl_char(intl_props, char_index) &&
                   ((fg->glyph->advance.x >> 16) > 0))
               {
                  if (evas_common_font_query_kerning(fi, index, prev_index,
                           &kern))
                    pen_x += kern;
               }
             else
               {
                  if (evas_common_font_query_kerning(fi, prev_index, index,
                           &kern))
                    pen_x += kern;
               }
#else
             if (evas_common_font_query_kerning(fi, prev_index, index,
                                                &kern))
                pen_x += kern;
#endif
          }
	pface = fi->src->ft.face;
	fg = evas_common_font_int_cache_glyph_get(fi, index);
	LKU(fi->ft_mutex);

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
	     ret = char_index;
             goto end;
	  }
	prev_chr_end = chr_x + chr_w;
	pen_x += fg->glyph->advance.x >> 16;
	prev_index = index;
     }
end:

  evas_common_font_int_use_trim();
   return ret;
}
