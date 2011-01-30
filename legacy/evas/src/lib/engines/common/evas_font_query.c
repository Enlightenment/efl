#include "evas_common.h"
#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
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

/**
 * @def _INIT_FI_AND_KERNINNG()
 * @internal
 * This macro inits fi and use_kerning.
 * Assumes the following variables exist:
 * fi, fn and use_kerning.
 */
#define _INIT_FI_AND_KERNING() \
   do \
     { \
        fi = fn->fonts->data; \
        /* evas_common_font_size_use(fn); */ \
        evas_common_font_int_reload(fi); \
        if (fi->src->current_size != fi->size) \
          { \
             FTLOCK(); \
             FT_Activate_Size(fi->ft.size); \
             FTUNLOCK(); \
             fi->src->current_size = fi->size; \
          } \
        FTLOCK(); \
        use_kerning = FT_HAS_KERNING(fi->src->ft.face); \
        FTUNLOCK(); \
     } \
   while (0)

/* size of the string (width and height) in pixels
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * We only care about the size, and the size does not depend on the visual order.
 * As long as we follow the logical string and get kerning data like we should,
 * we are fine.
 */

EAPI void
evas_common_font_query_size(RGBA_Font *fn, const Eina_Unicode *text, const Evas_BiDi_Props *intl_props __UNUSED__, int *w, int *h)
{
   int keep_width = 0;
   int prev_pen_x = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK(EINA_FALSE);
        /* Keep the width because we'll need it for the last char */
        keep_width = width + bear_x;
        /* Keep the previous pen_x, before it's advanced in TEXT_END */
        prev_pen_x = pen_x;
     }
   EVAS_FONT_WALK_TEXT_END();
   if (w) *w = prev_pen_x + keep_width;
   if (h) *h = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
  evas_common_font_int_use_trim();
}
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
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();
#ifndef BIDI_SUPPORT
   /* Suppress warnings */
   (void) intl_props;
#endif

   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK(EINA_FALSE);
     }
   EVAS_FONT_WALK_TEXT_END();

   if (v_adv) *v_adv = evas_common_font_get_line_advance(fn);
   if (h_adv) *h_adv = pen_x;
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
   int asc, desc;
   int position = 0;
   const Eina_Unicode *text = in_text;
   int ret_val = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

#ifdef BIDI_SUPPORT
   Eina_Unicode *visual_text = NULL;
   int len;

   if (intl_props && (intl_props->dir == EVAS_BIDI_DIRECTION_RTL))
     {
        visual_text = eina_unicode_strdup(in_text);

        if (visual_text)
          {
             evas_bidi_reverse_string(visual_text);
             text = visual_text;
          }
     }
   if (!visual_text)
     {
        text = in_text;
     }
   len = eina_unicode_strlen(text);
#endif

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

#ifdef BIDI_SUPPORT
   /* Get the position in the visual string because those are the coords we care about */
   position = evas_bidi_position_reverse(intl_props, len, pos);
#else
   position = pos;
#endif
   /* If it's the null, choose location according to the direction. */
   if (!text[position])
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (intl_props->dir == EVAS_BIDI_DIRECTION_RTL)
          {
             if (cx) *cx = 0;
             if (ch) *ch = asc + desc;
          }
        else
#endif
          {
             evas_common_font_query_advance(fn, in_text, intl_props, cx, ch);
          }
        if (cy) *cy = 0;
        if (cw) *cw = 0;
        ret_val = 1;
        goto end;
     }

   EVAS_FONT_WALK_TEXT_START()
     {
	int chr_x, chr_y, chr_w;

        EVAS_FONT_WALK_TEXT_WORK(EINA_TRUE);

        chr_x = (pen_x) + bear_x;
	chr_y = (pen_y) + bear_y;
        chr_w = width;
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
     }
   EVAS_FONT_WALK_TEXT_END();
end:

#ifdef BIDI_SUPPORT
   if (visual_text) free(visual_text);
#endif

  evas_common_font_int_use_trim();
   return ret_val;
}

/* x y w h for pen at char pos for null it returns the position right after
 * the last char with 0 as width and height. This is the same as char_coords
 * but it returns the pen_x and adv instead of x and w.
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * We care about the actual drawing location of the string, this is why we need
 * the visual string. We need to know how it's printed. After that we need to calculate
 * the reverse kerning in case of rtl parts. "pos" passed to this function is an
 * index in bytes, that is the actual byte location of the string, we need to find
 * the index in order to find it in the visual string.
 */

EAPI int
evas_common_font_query_pen_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_BiDi_Props *intl_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch)
{
   int asc, desc;
   int position = 0;
   const Eina_Unicode *text = in_text;
   int ret_val = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

#ifdef BIDI_SUPPORT
   Eina_Unicode *visual_text = NULL;
   int len;

   if (intl_props && (intl_props->dir == EVAS_BIDI_DIRECTION_RTL))
     {
        visual_text = eina_unicode_strdup(in_text);

        if (visual_text)
          {
             evas_bidi_reverse_string(visual_text);
             text = visual_text;
          }
     }
   if (!visual_text)
     {
        text = in_text;
     }
   len = eina_unicode_strlen(text);
#endif

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

#ifdef BIDI_SUPPORT
   /* Get the position in the visual string because those are the coords we care about */
   position = evas_bidi_position_reverse(intl_props, len, pos);
#else
   position = pos;
#endif
   /* If it's the null, choose location according to the direction. */
   if (!text[position])
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (intl_props->dir == EVAS_BIDI_DIRECTION_RTL)
          {
             if (cpen_x) *cpen_x = 0;
             if (ch) *ch = asc + desc;
          }
        else
#endif
          {
             evas_common_font_query_advance(fn, in_text, intl_props, cpen_x, ch);
          }
        if (cy) *cy = 0;
        if (cadv) *cadv = 0;
        ret_val = 1;
        goto end;
     }

   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK(EINA_TRUE);
	/* we need to see if the char at the visual position is the char wanted */
	if (char_index == position)
	  {
	     if (cpen_x) *cpen_x = pen_x;
	     if (cy) *cy = -asc;
	     if (cadv) *cadv = adv;
	     if (ch) *ch = asc + desc;
	     ret_val = 1;
	     goto end;
	  }
     }
   EVAS_FONT_WALK_TEXT_END();
end:

#ifdef BIDI_SUPPORT
   if (visual_text) free(visual_text);
#endif

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
   int asc, desc;
   const Eina_Unicode *text = in_text;
   int ret_val = -1;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();
#ifndef BIDI_SUPPORT
   /* Suppress warnings */
   (void) intl_props;
#endif

#ifdef BIDI_SUPPORT
   Eina_Unicode *visual_text = NULL;
   int len;

   if (intl_props && (intl_props->dir == EVAS_BIDI_DIRECTION_RTL))
     {
        visual_text = eina_unicode_strdup(in_text);

        if (visual_text)
          {
             evas_bidi_reverse_string(visual_text);
             text = visual_text;
          }
     }
   if (!visual_text)
     {
        text = in_text;
     }
   len = eina_unicode_strlen(text);
#endif

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   EVAS_FONT_WALK_TEXT_START()
     {
	int chr_x, chr_w;

        EVAS_FONT_WALK_TEXT_WORK(EINA_TRUE);

        chr_x = (pen_x) + bear_x;
        chr_w = width;
	/* we need to see if the char at the visual position is the char,
         * we check that by checking if it's before the current pen position
         * and the next */
	if ((x >= pen_x) && (x <= (pen_x + adv)) &&
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
             position = evas_bidi_position_reverse(intl_props, len, position);
#endif
	     ret_val = position;
	     goto end;
	  }
     }
   EVAS_FONT_WALK_TEXT_END();

end:
#ifdef BIDI_SUPPORT
   if (visual_text) free(visual_text);
#endif

  evas_common_font_int_use_trim();
   return ret_val;
}

/* position of the char after the last char in the text that will fit in xy.
 * BiDi handling: We receive the shaped string + other props from intl_props,
 * All we care about is char sizes + kerning so we only really need to get the
 * shaped string to utf8, and then just go through it like in english, as it's
 * just the logical string, nothing special about that.
 */

EAPI int
evas_common_font_query_last_up_to_pos(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_BiDi_Props *intl_props __UNUSED__, int x, int y)
{
   int asc, desc;
   int ret=-1;
   const Eina_Unicode *text = in_text;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK(EINA_FALSE);

	if ((x >= pen_x) && (x <= (pen_x + adv)) &&
	    (y >= -asc) && (y <= desc))
	  {
	     ret = char_index;
             goto end;
	  }
     }
   EVAS_FONT_WALK_TEXT_END();

end:

  evas_common_font_int_use_trim();
  return ret;
}
