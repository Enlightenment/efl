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
/* FIXME: should use OT info when available. */
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
evas_common_font_query_size(RGBA_Font *fn, const Eina_Unicode *text, const Evas_Text_Props *intl_props __UNUSED__, int *w, int *h)
{
   int keep_width = 0;
   int prev_pen_x = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_VISUAL_START()
          {
             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;
             /* Keep the width because we'll need it for the last char */
             keep_width = EVAS_FONT_WALK_OT_WIDTH + EVAS_FONT_WALK_OT_X_OFF +
                EVAS_FONT_WALK_OT_X_BEAR;
             /* Keep the previous pen_x, before it's advanced in TEXT_END */
             prev_pen_x = pen_x;
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START()
          {
             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;
             /* Keep the width because we'll need it for the last char */
             keep_width = EVAS_FONT_WALK_DEFAULT_WIDTH +
                EVAS_FONT_WALK_DEFAULT_X_OFF +
                EVAS_FONT_WALK_DEFAULT_X_BEAR;
             /* Keep the previous pen_x, before it's advanced in TEXT_END */
             prev_pen_x = pen_x;
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }
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
evas_common_font_query_advance(RGBA_Font *fn, const Eina_Unicode *text, const Evas_Text_Props *intl_props, int *h_adv, int *v_adv)
{
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();
#ifndef BIDI_SUPPORT
   /* Suppress warnings */
   (void) intl_props;
#endif

#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_VISUAL_START()
          {
             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START()
          {
             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }

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
evas_common_font_query_char_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch)
{
   int asc, desc;
   int position = 0;
   const Eina_Unicode *text = in_text;
   int ret_val = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   position = pos;
   /* If it's the null, choose location according to the direction. */
   if (!text[position])
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
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

#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_VISUAL_START()
          {
             int chr_x, chr_w;
             int found = 0, items = 1, item_pos = 1;

             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_TRUE);
             if (visible)
               {
                  chr_x = (pen_x) + EVAS_FONT_WALK_OT_X_OFF +
                     EVAS_FONT_WALK_OT_X_BEAR;
                  chr_w = EVAS_FONT_WALK_OT_WIDTH;
               }
             else
               {
                  chr_x = pen_x;
                  chr_w = 0;
               }
             /* we need to see if the char at the visual position is the char wanted */
             if ((intl_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR) &&
                   (EVAS_FONT_WALK_OT_POS <= (size_t) position) &&
                   ((((size_t) position) < EVAS_FONT_WALK_OT_POS_NEXT) ||
                    (EVAS_FONT_WALK_OT_IS_LAST)))
               {
                  found = 1;
                  items = EVAS_FONT_WALK_OT_POS_NEXT - EVAS_FONT_WALK_OT_POS;
                  if (EVAS_FONT_WALK_OT_POS == EVAS_FONT_WALK_OT_POS_NEXT)
                    {
                    /* If there was only one char, take the original lens
                     * for the number of items. */
                       items = EVAS_FONT_WALK_ORIG_LEN -
                          EVAS_FONT_WALK_OT_POS;
                    }
                  item_pos = position - EVAS_FONT_WALK_OT_POS + 1;
               }
             else if ((intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) &&
                   ((EVAS_FONT_WALK_OT_POS_PREV > (size_t) position) ||
                    (EVAS_FONT_WALK_OT_IS_LAST)) &&
                   (((size_t) position) >= EVAS_FONT_WALK_OT_POS))
               {
                  found = 1;
                  items = EVAS_FONT_WALK_OT_POS_PREV - EVAS_FONT_WALK_OT_POS;
                  if (EVAS_FONT_WALK_OT_POS == EVAS_FONT_WALK_OT_POS_PREV)
                    {
                    /* If there was only one char, take the original lens
                     * for the number of items. */
                       items = EVAS_FONT_WALK_ORIG_LEN -
                          EVAS_FONT_WALK_OT_POS;
                    }
                  item_pos = items - (position - EVAS_FONT_WALK_OT_POS);
               }

             if (found)
               {
                  if (cx) *cx = chr_x +
                    (EVAS_FONT_WALK_OT_WIDTH / items) * (item_pos - 1);
                  if (cy) *cy = -asc;
                  if (cw) *cw = chr_w / items;
                  if (ch) *ch = asc + desc;
                  ret_val = 1;
                  goto end;
               }
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START()
          {
             int chr_x, chr_w;

             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_TRUE);
             if (visible)
               {
                  chr_x = (pen_x) + EVAS_FONT_WALK_DEFAULT_X_OFF +
                     EVAS_FONT_WALK_DEFAULT_X_BEAR;
                  chr_w = EVAS_FONT_WALK_DEFAULT_WIDTH;
               }
             else
               {
                  chr_x = pen_x;
                  chr_w = 0;
               }
             /* we need to see if the char at the visual position is the char wanted */
             if (EVAS_FONT_WALK_DEFAULT_POS == (size_t) position)
               {
                  if (cx) *cx = chr_x;
                  if (cy) *cy = -asc;
                  if (cw) *cw = chr_w;
                  if (ch) *ch = asc + desc;
                  ret_val = 1;
                  goto end;
               }
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }
end:

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
evas_common_font_query_pen_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_Text_Props *intl_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch)
{
   int asc, desc;
   int position = 0;
   const Eina_Unicode *text = in_text;
   int ret_val = 0;
   int use_kerning;
   RGBA_Font_Int *fi;
   EVAS_FONT_WALK_TEXT_INIT();
   _INIT_FI_AND_KERNING();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   position = pos;
   /* If it's the null, choose location according to the direction. */
   if (!text[position])
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
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
#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_VISUAL_START()
          {
             int found = 0, items = 1, item_pos = 1;
             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_TRUE);
             /* we need to see if the char at the visual position is the char wanted */
             if ((intl_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR) &&
                   (EVAS_FONT_WALK_OT_POS <= (size_t) position) &&
                   ((((size_t) position) < EVAS_FONT_WALK_OT_POS_NEXT) ||
                    (EVAS_FONT_WALK_OT_IS_LAST)))
               {
                  found = 1;
                  items = EVAS_FONT_WALK_OT_POS_NEXT - EVAS_FONT_WALK_OT_POS;
                  if (EVAS_FONT_WALK_OT_POS == EVAS_FONT_WALK_OT_POS_NEXT)
                    {
                    /* If there was only one char, take the original lens
                     * for the number of items. */
                       items = EVAS_FONT_WALK_ORIG_LEN -
                          EVAS_FONT_WALK_OT_POS;
                    }
                  item_pos = position - EVAS_FONT_WALK_OT_POS + 1;
               }
             else if ((intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) &&
                   ((EVAS_FONT_WALK_OT_POS_PREV > (size_t) position) ||
                    (EVAS_FONT_WALK_OT_IS_LAST)) &&
                   (((size_t) position) >= EVAS_FONT_WALK_OT_POS))
               {
                  found = 1;
                  items = EVAS_FONT_WALK_OT_POS_PREV - EVAS_FONT_WALK_OT_POS;
                  if (EVAS_FONT_WALK_OT_POS == EVAS_FONT_WALK_OT_POS_PREV)
                    {
                    /* If there was only one char, take the original lens
                     * for the number of items. */
                       items = EVAS_FONT_WALK_ORIG_LEN -
                          EVAS_FONT_WALK_OT_POS;
                    }
                  item_pos = items - (position - EVAS_FONT_WALK_OT_POS);
               }

             if (found)
               {
                  if (cy) *cy = -asc;
                  if (ch) *ch = asc + desc;
                  /* FIXME: A hack to make combining chars work nice, should
                   * change to take the base char's adv. */
                  if (visible)
                    {
                       if (EVAS_FONT_WALK_OT_X_ADV > 0)
                         {
                            if (cpen_x) *cpen_x = pen_x +
                              (EVAS_FONT_WALK_OT_X_ADV / items) *
                                 (item_pos - 1);
                            if (cadv) *cadv = (EVAS_FONT_WALK_OT_X_ADV / items);
                         }
                       else
                         {
                            if (cpen_x) *cpen_x = pen_x +
                              EVAS_FONT_WALK_OT_X_OFF +
                              EVAS_FONT_WALK_OT_X_BEAR +
                                 (EVAS_FONT_WALK_OT_WIDTH / items) *
                                 (item_pos - 1);
                            if (cadv) *cadv = (EVAS_FONT_WALK_OT_WIDTH / items);
                         }
                    }
                  else
                    {
                       if (cpen_x) *cpen_x = pen_x;
                       if (cadv) *cadv = 0;
                    }
                  ret_val = 1;
                  goto end;
               }
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START()
          {
             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_TRUE);

             if ((EVAS_FONT_WALK_DEFAULT_POS == (size_t) position))
               {
                  if (cy) *cy = -asc;
                  if (ch) *ch = asc + desc;
                  /* FIXME: A hack to make combining chars work nice, should change
                   * to take the base char's adv. */
                  if (visible)
                    {
                       if (EVAS_FONT_WALK_DEFAULT_X_ADV > 0)
                         {
                            if (cpen_x) *cpen_x = pen_x;
                            if (cadv) *cadv = EVAS_FONT_WALK_DEFAULT_X_ADV;
                         }
                       else
                         {
                            if (cpen_x) *cpen_x = pen_x +
                              EVAS_FONT_WALK_DEFAULT_X_OFF +
                              EVAS_FONT_WALK_DEFAULT_X_BEAR;
                            if (cadv) *cadv = EVAS_FONT_WALK_DEFAULT_WIDTH;
                         }
                    }
                  else
                    {
                       if (cpen_x) *cpen_x = pen_x;
                       if (cadv) *cadv = 0;
                    }
                  ret_val = 1;
                  goto end;
               }
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }
end:

   return ret_val;
}

/* char pos of text at xy pos
 * BiDi handling: Since we are looking for the char at the specific coords,
 * we have to get the visual string (to which the coords relate to), do
 * reverse kerning query because we are working on the visual string, and then
 * we need to get the logical position of the char we found from the visual string.
 */

EAPI int
evas_common_font_query_char_at_coords(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_Text_Props *intl_props, int x, int y, int *cx, int *cy, int *cw, int *ch)
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

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_VISUAL_START()
          {
             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_TRUE);
             if (!visible) continue;

             /* we need to see if the char at the visual position is the char,
              * we check that by checking if it's before the current pen
              * position and the next */
             if ((x >= pen_x) && (x <= (pen_x + EVAS_FONT_WALK_OT_X_ADV)) &&
                   (y >= -asc) && (y <= desc))
               {
                  int items = 1, item_pos = 1;

                  if (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR)
                    {
                       double part;
                       items = EVAS_FONT_WALK_OT_POS_NEXT -
                          EVAS_FONT_WALK_OT_POS;
                       /* If it's the last/first char in a ltr/rtl string */
                       if (items == 0)
                         {
                            items = EVAS_FONT_WALK_ORIG_LEN -
                               EVAS_FONT_WALK_OT_POS;
                         }
                       part = EVAS_FONT_WALK_OT_X_ADV / items;
                       item_pos = (int) ((x - pen_x) / part);
                    }
                  else
                    {
                       double part;
                       items = EVAS_FONT_WALK_OT_POS_PREV -
                          EVAS_FONT_WALK_OT_POS;
                       /* If it's the last/first char in a ltr/rtl string */
                       if (items == 0)
                         {
                            items = EVAS_FONT_WALK_ORIG_LEN -
                               EVAS_FONT_WALK_OT_POS;
                         }
                       part = EVAS_FONT_WALK_OT_X_ADV / items;
                       item_pos = items - ((int) ((x - pen_x) / part)) - 1;
                    }
                  if (cx) *cx = pen_x + EVAS_FONT_WALK_OT_X_OFF +
                    EVAS_FONT_WALK_OT_X_BEAR +
                       ((EVAS_FONT_WALK_OT_X_ADV / items) * (item_pos - 1));
                  if (cy) *cy = -asc;
                  if (cw) *cw = (EVAS_FONT_WALK_OT_X_ADV / items);
                  if (ch) *ch = asc + desc;
                  ret_val = EVAS_FONT_WALK_OT_POS + item_pos;
                  goto end;
               }
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START()
          {
             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_TRUE);
             if (!visible) continue;

             /* we need to see if the char at the visual position is the char,
              * we check that by checking if it's before the current pen position
              * and the next */
             if ((x >= pen_x) && (x <= (pen_x + EVAS_FONT_WALK_DEFAULT_X_ADV))
                   && (y >= -asc) && (y <= desc))
               {
                  if (cx) *cx = pen_x + EVAS_FONT_WALK_DEFAULT_X_OFF +
                    EVAS_FONT_WALK_DEFAULT_X_BEAR +
                       EVAS_FONT_WALK_DEFAULT_X_ADV;
                  if (cy) *cy = -asc;
                  if (cw) *cw = EVAS_FONT_WALK_DEFAULT_X_ADV;
                  if (ch) *ch = asc + desc;
                  ret_val = EVAS_FONT_WALK_DEFAULT_POS;
                  goto end;
               }
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }

end:

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
evas_common_font_query_last_up_to_pos(RGBA_Font *fn, const Eina_Unicode *in_text, const Evas_Text_Props *intl_props __UNUSED__, int x, int y)
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

#ifdef OT_SUPPORT
   if (evas_common_font_ot_is_enabled() && intl_props->ot_data)
     {
        EVAS_FONT_WALK_OT_TEXT_LOGICAL_START()
          {
             EVAS_FONT_WALK_OT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;

             if ((x >= pen_x) && (x <= (pen_x + EVAS_FONT_WALK_OT_X_ADV)) &&
                   (y >= -asc) && (y <= desc))
               {
                  ret = EVAS_FONT_WALK_OT_POS;
                  goto end;
               }
          }
        EVAS_FONT_WALK_OT_TEXT_END();
     }
   else
#endif
     {
        EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START()
          {
             EVAS_FONT_WALK_DEFAULT_TEXT_WORK(EINA_FALSE);
             if (!visible) continue;

             if ((x >= pen_x) &&
                   (x <= (pen_x + EVAS_FONT_WALK_DEFAULT_X_ADV)) &&
                   (y >= -asc) && (y <= desc))
               {
                  ret = char_index;
                  goto end;
               }
          }
        EVAS_FONT_WALK_DEFAULT_TEXT_END();
     }

end:

  evas_common_font_int_use_trim();
  return ret;
}

