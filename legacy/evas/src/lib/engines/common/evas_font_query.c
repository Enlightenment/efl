#include "evas_common.h"
#include "language/evas_bidi_utils.h" /*defines BIDI_SUPPORT if possible */
#include "evas_font_private.h" /* for Frame-Queuing support */
#include "evas_font_ot.h"


/* FIXME: Check coverage according to the font and not by actually loading */
/**
 * @internal
 * Find the end of a run according to font coverage, and return the base script
 * font and the current wanted font.
 *
 * @param[in] fn the font to use.
 * @param script_fi The base font instance to be used with the script. If NULL, then it's calculated and returned in this variable, if not NULL, it's used and not modified.
 * @param[out] cur_fi The font instance found for the current run.
 * @param[in] script the base script
 * @param[in] text the text to work on.
 * @param[in] run_let the current run len, i.e "search limit".
 * @return length of the run found.
 */
EAPI int
evas_common_font_query_run_font_end_get(RGBA_Font *fn, RGBA_Font_Int **script_fi, RGBA_Font_Int **cur_fi, Evas_Script_Type script, const Eina_Unicode *text, int run_len)
{
   RGBA_Font_Int *fi = NULL;
   const Eina_Unicode *run_end = text + run_len;
   const Eina_Unicode *itr;

   /* If there's no current script_fi, find it first */
   if (!*script_fi)
     {
        const Eina_Unicode *base_char = NULL;
        /* Skip common chars */
        for (base_char = text ;
             (base_char < run_end) &&
             (evas_common_language_char_script_get(*base_char) != script) ;
             base_char++)
           ;
        if (base_char == run_end) base_char = text;

        /* Find the first renderable char */
        while (base_char < run_end)
          {
             /* 0x1F is the last ASCII contral char, just a hack in
              * the meanwhile. */
             if ((*base_char > 0x1F) &&
                   evas_common_font_glyph_search(fn, &fi, *base_char))
                break;
             base_char++;
          }


        /* If everything else fails, at least try to find a font for the
         * replacement char */
        if (base_char == run_end)
           evas_common_font_glyph_search(fn, &fi, REPLACEMENT_CHAR);

        if (!fi)
           fi = fn->fonts->data;

        *script_fi = fi;
     }
   else
     {
        fi = *script_fi;
     }

   /* Find the longest run of the same font starting from the start position
    * and update cur_fi accordingly. */
   itr = text;
   while (itr < run_end)
     {
        RGBA_Font_Int *tmp_fi;
        /* Itr will end up being the first of the next run  */
        for ( ; itr < run_end ; itr++)
          {
             /* 0x1F is the last ASCII contral char, just a hack in
              * the meanwhile. */
             if (*itr <= 0x1F)
                continue;
             /* Break if either it's not in the font, or if it is in the
              * script's font. */
             if (fi == *script_fi)
               {
                  if (!evas_common_get_char_index(fi, *itr))
                     break;
               }
             else
               {
                  if (evas_common_get_char_index(*script_fi, *itr))
                     break;
               }
          }

        /* Abort if we reached the end */
        if (itr == run_end)
           break;

        /* If the script font doesn't fit even one char, find a new font. */
        if (itr == text)
          {
             /* If we can find a font, use it. Otherwise, find the first
              * char the run of chars that can't be rendered until the first
              * one that can. */
             if (evas_common_font_glyph_search(fn, &tmp_fi, *itr))
               {
                  fi = tmp_fi;
               }
             else
               {
                  itr++;
                  /* Go through all the chars that can't be rendered with any
                   * font */
                  for ( ; itr < run_end ; itr++)
                    {
                       tmp_fi = fi;
                       if (evas_common_get_char_index(fi, *itr) ||
                             evas_common_font_glyph_search(fn, &tmp_fi, *itr))
                         {
                            fi = tmp_fi;
                            break;
                         }
                    }

                  /* If we found a renderable character and the found font
                   * can render the replacement char, continue, otherwise
                   * find a font most suitable for the replacement char and
                   * break */
                  if ((itr == run_end) ||
                        !evas_common_get_char_index(fi, REPLACEMENT_CHAR))
                    {
                       evas_common_font_glyph_search(fn, &fi, REPLACEMENT_CHAR);
                       break;
                    }
               }
             itr++;
          }
        else
          {
             /* If this char is not renderable by any font, but the replacement
              * char can be rendered using the currentfont, continue this
              * run. */
             if (!evas_common_font_glyph_search(fn, &tmp_fi, *itr) &&
                   evas_common_get_char_index(fi, REPLACEMENT_CHAR))
               {
                  itr++;
               }
             else
               {
                  /* Done, we did as much as possible */
                  break;
               }
          }
     }

   if (fi)
      *cur_fi = fi;
   else
      *cur_fi = *script_fi;

   return itr - text;
}

/**
 * @internal
 * Calculate the kerning between "left" and "right.
 *
 * @param fi the font instance to use
 * @param left the left glyph index
 * @param right the right glyph index
 * @param[out] kerning the kerning calculated.
 * @return FALSE on error, TRUE on success.
 */
EAPI int
evas_common_font_query_kerning(RGBA_Font_Int *fi, FT_UInt left, FT_UInt right,
			       int *kerning)
{
   int *result;
   FT_Vector delta;
   int key[2];
   int error = 1;

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
		      FT_KERNING_DEFAULT, &delta) == 0)
     {
	int *push;

        FTUNLOCK();
	*kerning = delta.x;

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

/**
 * @internal
 * Calculate the inset of the text. Inset is the difference between the pen
 * position of the first char in the string, and the first pixel drawn.
 * (can be negative).
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @return the calculated inset.
 */
EAPI int
evas_common_font_query_inset(RGBA_Font *fn __UNUSED__, const Evas_Text_Props *text_props)
{
   if (!text_props->len) return 0;
   return text_props->info->glyph[text_props->start].x_bear;
}

/**
 * @internal
 * Calculate the right inset of the text. This is the difference between the
 * pen position of the glyph after the last glyph in the text, and the last
 * pixel drawn in the text (essentially "advance - width" of the last char).
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @return the calculated inset.
 *
 * @see evas_common_font_query_inset()
 */
EAPI int
evas_common_font_query_right_inset(RGBA_Font *fn __UNUSED__, const Evas_Text_Props *text_props)
{
   const Evas_Font_Glyph_Info *gli;
   if (!text_props->len) return 0;
   gli = text_props->info->glyph + text_props->start + text_props->len - 1;

   /* If the last char is a whitespace, we use the advance as the size,
    * so the right_inset is 0. */
   if (gli->width == 0)
      return 0;

   return ((gli > text_props->info->glyph) ?
      gli->pen_after - (gli - 1)->pen_after : gli->pen_after) -
      (gli->width + gli->x_bear
#ifdef OT_SUPPORT
       + EVAS_FONT_ROUND_26_6_TO_INT(EVAS_FONT_OT_X_OFF_GET(
              text_props->info->ot[text_props->start + text_props->len - 1]))
#endif
      );
}

/**
 * @internal
 * Calculate the size of the string (width and height).
 * The width is the disntance between the first pen position and the last pixel
 * drawn.
 * The height is the max ascent+descent of the font.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param[out] w the calculated width
 * @param[out] h the calculated height
 */
EAPI void
evas_common_font_query_size(RGBA_Font *fn, const Evas_Text_Props *text_props, int *w, int *h)
{
   Evas_Coord ret_w = 0;

   if (text_props->len > 0)
     {
        const Evas_Font_Glyph_Info *glyph = text_props->info->glyph +
           text_props->start;
        const Evas_Font_Glyph_Info *last_glyph = glyph;

        if (text_props->len > 1)
          {
             last_glyph += text_props->len - 1;
             ret_w = last_glyph[-1].pen_after;
             if (text_props->start > 0)
                ret_w -= glyph[-1].pen_after;
          }
#ifdef OT_SUPPORT
        ret_w += EVAS_FONT_ROUND_26_6_TO_INT(EVAS_FONT_OT_X_OFF_GET(
              text_props->info->ot[text_props->start + text_props->len - 1]));
#endif
        ret_w += last_glyph->width + last_glyph->x_bear;
     }

   if (w) *w = ret_w;
   if (h) *h = evas_common_font_max_ascent_get(fn) + evas_common_font_max_descent_get(fn);
}

/**
 * @internal
 * Calculate the advance of the string. Advance is the distance between the
 * first pen position and the pen position after the string.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param[out] h_adv the calculated horizontal advance.
 * @param[out] v_adv the calculated vertical advance.
 */
EAPI void
evas_common_font_query_advance(RGBA_Font *fn, const Evas_Text_Props *text_props, int *h_adv, int *v_adv)
{
   Evas_Coord ret_adv = 0;
   if (text_props->len > 0)
     {
        const Evas_Font_Glyph_Info *glyph = text_props->info->glyph +
           text_props->start;
        ret_adv = glyph[text_props->len - 1].pen_after;
        if (text_props->start > 0)
           ret_adv -= glyph[-1].pen_after;
     }

   if (h_adv) *h_adv = ret_adv;
   if (v_adv) *v_adv = evas_common_font_get_line_advance(fn);
}

/**
 * @internal
 * Query the coordinates of the char at position pos. If the position is at the
 * end of the string (i.e where the finishing null would be) it returns the
 * coordinates of the position right after the last char. This is either on
 * the left or on the right of the string, depending on BiDi direction. Returned
 * width in this case is 0. It returns the x of the leftmost pixel drawn.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param pos the position of the char in the string object (not actual position in the string object, but the position of the source character).
 * @param[out] cx the calculated x - CAN BE NULL
 * @param[out] cy the calculated y - CAN BE NULL
 * @param[out] cw the calculated width - CAN BE NULL
 * @param[out] ch the calculated height - CAN BE NULL
 * @return TRUE on success, FALSE otherwise.
 *
 * @see evas_common_font_query_pen_coords()
 */
EAPI int
evas_common_font_query_char_coords(RGBA_Font *fn, const Evas_Text_Props *text_props, int pos, int *cx, int *cy, int *cw, int *ch)
{
   int asc, desc;
   size_t position = 0;
   int ret_val = 0;
   EVAS_FONT_WALK_TEXT_INIT();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   position = pos;
   /* If it's the null, choose location according to the direction. */
   if (position == text_props->text_len)
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
          {
             if (cx) *cx = 0;
             if (ch) *ch = asc + desc;
          }
        else
#endif
          {
             evas_common_font_query_advance(fn, text_props, cx, ch);
          }
        if (cy) *cy = 0;
        if (cw) *cw = 0;
        ret_val = 1;
        goto end;
     }

   Evas_Coord cluster_start = 0, last_end = 0;
   int prev_cluster = -1;
   int found = 0, items = 1, item_pos = 1;
   int last_is_visible = 0;
   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK();

        if (prev_cluster != (int) EVAS_FONT_WALK_POS)
          {
             if (found)
               {
                  break;
               }
             else
               {
                  cluster_start = EVAS_FONT_WALK_PEN_X +
                     EVAS_FONT_WALK_X_OFF +
                     EVAS_FONT_WALK_X_BEAR;
               }
          }
        last_is_visible = EVAS_FONT_WALK_IS_VISIBLE;
        last_end = EVAS_FONT_WALK_PEN_X + EVAS_FONT_WALK_X_OFF +
           EVAS_FONT_WALK_X_BEAR + EVAS_FONT_WALK_WIDTH;
        /* we need to see if the char at the visual position is the char wanted */
        if ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR) &&
              (EVAS_FONT_WALK_POS <= (size_t) position) &&
              ((((size_t) position) < EVAS_FONT_WALK_POS_NEXT) ||
               (EVAS_FONT_WALK_IS_LAST)))
          {
             found = 1;
#ifdef OT_SUPPORT
             items = evas_common_font_ot_cluster_size_get(text_props,
                                                          char_index);
#endif
             item_pos = position - EVAS_FONT_WALK_POS + 1;
          }
        else if ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) &&
              ((EVAS_FONT_WALK_POS_PREV > (size_t) position) ||
               (EVAS_FONT_WALK_IS_FIRST)) &&
              (((size_t) position) >= EVAS_FONT_WALK_POS))
          {
             found = 1;
#ifdef OT_SUPPORT
             items = evas_common_font_ot_cluster_size_get(text_props,
                                                          char_index);
#endif
             item_pos = items - (position - EVAS_FONT_WALK_POS);
          }

        prev_cluster = EVAS_FONT_WALK_POS;
     }
   EVAS_FONT_WALK_TEXT_END();
   if (found)
     {
        Evas_Coord cluster_w;
        cluster_w = last_end - cluster_start;
        if (cy) *cy = -asc;
        if (ch) *ch = asc + desc;
        if (last_is_visible)
          {
             if (cx) *cx = cluster_start +
               (cluster_w / items) *
                  (item_pos - 1);
             if (cw) *cw = (cluster_w / items);
          }
        else
          {
             if (cx) *cx = cluster_start;
             if (cw) *cw = 0;
          }
        ret_val = 1;
        goto end;
     }
end:

   return ret_val;
}

/**
 * @internal
 * Query the coordinates of the char at position pos. If the position is at the
 * end of the string (i.e where the finishing null would be) it returns the
 * coordinates of the position right after the last char. This is either on
 * the left or on the right of the string, depending on BiDi direction. Returned
 * advance in this case is 0.
 *
 * This is the same as evas_common_font_query_char_coords() except that the
 * advance of the character is returned instead of the width and the pen
 * position is returned instead of the actual pixel position.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param pos the position of the char in the string object (not actual position in the string object, but the position of the source character).
 * @param[out] cpenx the calculated x - CAN BE NULL
 * @param[out] cy the calculated y - CAN BE NULL
 * @param[out] cadv the calculated advance - CAN BE NULL
 * @param[out] ch the calculated height - CAN BE NULL
 * @return TRUE on success, FALSE otherwise.
 *
 * @see evas_common_font_query_char_coords()
 */
EAPI int
evas_common_font_query_pen_coords(RGBA_Font *fn, const Evas_Text_Props *text_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch)
{
   int asc, desc;
   size_t position;
   int ret_val = 0;
   EVAS_FONT_WALK_TEXT_INIT();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

   position = pos;
   /* If it's the null, choose location according to the direction. */
   if (position == text_props->text_len)
     {
        /* if it's rtl then the location is the left of the string,
         * otherwise, the right. */
#ifdef BIDI_SUPPORT
        if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
          {
             if (cpen_x) *cpen_x = 0;
             if (ch) *ch = asc + desc;
          }
        else
#endif
          {
             evas_common_font_query_advance(fn, text_props, cpen_x, ch);
          }
        if (cy) *cy = 0;
        if (cadv) *cadv = 0;
        ret_val = 1;
        goto end;
     }
   Evas_Coord cluster_start = 0;
   int prev_cluster = -1;
   int found = 0, items = 1, item_pos = 1;
   int last_is_visible = 0;
   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK();

        if (prev_cluster != (int) EVAS_FONT_WALK_POS)
          {
             if (found)
               {
                  break;
               }
             else
               {
                  cluster_start = EVAS_FONT_WALK_PEN_X;
               }
          }
        last_is_visible = EVAS_FONT_WALK_IS_VISIBLE;

        if ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR) &&
              (EVAS_FONT_WALK_POS <= (size_t) position) &&
              ((((size_t) position) < EVAS_FONT_WALK_POS_NEXT) ||
               (EVAS_FONT_WALK_IS_LAST)))
          {
             found = 1;
#ifdef OT_SUPPORT
             items = evas_common_font_ot_cluster_size_get(text_props,
                                                          char_index);
#endif
             item_pos = position - EVAS_FONT_WALK_POS + 1;
          }
        else if ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) &&
              ((EVAS_FONT_WALK_POS_PREV > (size_t) position) ||
               (EVAS_FONT_WALK_IS_FIRST)) &&
              (((size_t) position) >= EVAS_FONT_WALK_POS))
          {
             found = 1;
#ifdef OT_SUPPORT
             items = evas_common_font_ot_cluster_size_get(text_props,
                                                          char_index);
#endif
             item_pos = items - (position - EVAS_FONT_WALK_POS);
          }

        prev_cluster = EVAS_FONT_WALK_POS;
     }
   EVAS_FONT_WALK_TEXT_END();

   if (found)
     {
        Evas_Coord cluster_adv;
        cluster_adv = EVAS_FONT_WALK_PEN_X - cluster_start;
        if (cy) *cy = -asc;
        if (ch) *ch = asc + desc;
        if (last_is_visible)
          {
             if (cpen_x) *cpen_x = cluster_start +
               (cluster_adv / items) *
                  (item_pos - 1);
             if (cadv) *cadv = (cluster_adv / items);
          }
        else
          {
             if (cpen_x) *cpen_x = EVAS_FONT_WALK_PEN_X;
             if (cadv) *cadv = 0;
          }
        ret_val = 1;
        goto end;
     }
end:

   return ret_val;
}

/**
 * @internal
 * Find the character at a specific x, y coordinates and return it's position
 * in the text (not in the text object, but in the source text). Also calculate
 * the char's geometry.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param x the x to look at.
 * @param y the y to look at.
 * @param[out] cx the calculated x - CAN BE NULL
 * @param[out] cy the calculated y - CAN BE NULL
 * @param[out] cw the calculated width - CAN BE NULL
 * @param[out] ch the calculated height - CAN BE NULL
 * @return the position found, -1 on failure.
 */
EAPI int
evas_common_font_query_char_at_coords(RGBA_Font *fn, const Evas_Text_Props *text_props, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   int asc, desc;
   int ret_val = -1;
   EVAS_FONT_WALK_TEXT_INIT();

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);
   Evas_Coord cluster_start = 0;
   int prev_cluster = -1;
   int found = 0, items = 1;
   EVAS_FONT_WALK_TEXT_START()
     {
        EVAS_FONT_WALK_TEXT_WORK();
        if (prev_cluster != (int) EVAS_FONT_WALK_POS)
          {
             if (found)
               {
                  break;
               }
             else
               {
                  cluster_start = EVAS_FONT_WALK_PEN_X;
               }
          }

        if (!EVAS_FONT_WALK_IS_VISIBLE) continue;

        /* we need to see if the char at the visual position is the char,
         * we check that by checking if it's before the current pen
         * position and the next */
        if ((x >= EVAS_FONT_WALK_PEN_X) &&
            (x <= (EVAS_FONT_WALK_PEN_X_AFTER)) && (y >= -asc) && (y <= desc))
          {
#ifdef OT_SUPPORT
             items = evas_common_font_ot_cluster_size_get(text_props,
                                                          char_index);
#endif
             found = 1;
          }

        prev_cluster = EVAS_FONT_WALK_POS;
     }
   EVAS_FONT_WALK_TEXT_END();
   if (found)
     {
        int item_pos;
        Evas_Coord cluster_adv;
        cluster_adv = EVAS_FONT_WALK_PEN_X - cluster_start;

        if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_LTR)
          {
             double part;
             part = cluster_adv / items;
             item_pos = (int) ((x - cluster_start) / part);
          }
        else
          {
             double part;
             part = cluster_adv / items;
             item_pos = items - ((int) ((x - cluster_start) / part)) - 1;
          }
        if (cx) *cx = EVAS_FONT_WALK_PEN_X +
          ((cluster_adv / items) * (item_pos - 1));
        if (cy) *cy = -asc;
        if (cw) *cw = (cluster_adv / items);
        if (ch) *ch = asc + desc;
        ret_val = prev_cluster + item_pos;
        goto end;
     }
end:

   return ret_val;
}

/**
 * @internal
 * Find one after the last character that fits until the boundaries set by x
 * and y. I.e find the first char that doesn't fit.
 * This LOGICALLY walks the string. This is needed for wrapping for example
 * where we want the first part to be the first logical part.
 *
 * @param fn the font set to use.
 * @param text_props the string object.
 * @param x the x boundary.
 * @param y the y boundary.
 * @return the position found, -1 on failure.
 */
EAPI int
evas_common_font_query_last_up_to_pos(RGBA_Font *fn, const Evas_Text_Props *text_props, int x, int y)
{
   int asc, desc;
   int ret=-1;

   asc = evas_common_font_max_ascent_get(fn);
   desc = evas_common_font_max_descent_get(fn);

#ifdef BIDI_SUPPORT
   if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        Evas_Font_Glyph_Info *gli = NULL;
        Evas_Coord full_adv = 0, pen_x = 0, start_pen = 0;
        int i;

        if ((text_props->info) && (text_props->len > 0))
          {
             gli = text_props->info->glyph + text_props->start;
             full_adv = gli[text_props->len - 1].pen_after;
             if (text_props->start > 0)
               {
                  start_pen = gli[-1].pen_after;
                  full_adv -= start_pen;
               }

             gli += text_props->len - 1;

             for (i = text_props->len - 1 ; i >= 0 ; i--, gli--)
               {
                  pen_x = full_adv - (gli->pen_after - start_pen);
                  /* If invisible, skip */
                  if (gli->index == 0) continue;
                  if ((x >= pen_x) &&
                        (((i == 0) && (x <= full_adv)) ||
                         (x <= (full_adv - (gli[-1].pen_after - start_pen)))) &&
                        (y >= -asc) && (y <= desc))
                    {
#ifdef OT_SUPPORT
                       ret = EVAS_FONT_OT_POS_GET(
                             text_props->info->ot[text_props->start + i]) -
                          text_props->text_offset;
#else
                       ret = text_props->text_len - i - 1;
#endif
                       goto end;
                    }
               }
          }
     }
   else
#endif
     {
        EVAS_FONT_WALK_TEXT_INIT();
        /* When text is not rtl, visual direction = logical direction */
        EVAS_FONT_WALK_TEXT_START()
          {
             EVAS_FONT_WALK_TEXT_WORK();
             if (!EVAS_FONT_WALK_IS_VISIBLE) continue;

             if ((x >= EVAS_FONT_WALK_PEN_X) &&
                 (x <= (EVAS_FONT_WALK_PEN_X_AFTER)) &&
                 (y >= -asc) && (y <= desc))
               {
                  ret = EVAS_FONT_WALK_POS;
                  goto end;
               }
          }
        EVAS_FONT_WALK_TEXT_END();
     }

end:

  return ret;
}

