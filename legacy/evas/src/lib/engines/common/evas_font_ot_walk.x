#ifndef _EVAS_FONT_OT_WALK_X
#define _EVAS_FONT_OT_WALK_X
# include "evas_font_ot.h"
# include "language/evas_language_utils.h"

/* Macros for text walking */

/**
 * @def EVAS_FONT_WALK_OT_TEXT_VISUAL_START
 * @internal
 * This runs through the text in visual order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_OT_X_OFF and the like.
 * @see EVAS_FONT_WALK_OT_TEXT_INIT
 * @see EVAS_FONT_WALK_OT_TEXT_WORK
 * @see EVAS_FONT_WALK_OT_TEXT_END
 * @see EVAS_FONT_WALK_OT_TEXT_LOGICAL_START
 */
#define EVAS_FONT_WALK_OT_TEXT_VISUAL_START() \
   do \
     { \
        int visible; \
        prev_index = 0; \
        /* Load the glyph according to the first letter of the script, preety
         * bad, but will have to do */ \
          { \
             /* Skip common chars */ \
             const Eina_Unicode *tmp; \
             for (tmp = text ; \
                   *tmp && \
                   evas_common_language_char_script_get(*tmp) == \
                   EVAS_SCRIPT_COMMON ; \
                   tmp++) \
               ; \
             if (!*tmp && (tmp > text)) tmp--; \
             evas_common_font_glyph_search(fn, &fi, *tmp); \
          } \
        for (char_index = 0 ; char_index < intl_props->ot_data->len ; char_index++) \
          { \
             FT_UInt index; \
             RGBA_Font_Glyph *fg; \

/**
 * @def EVAS_FONT_WALK_OT_TEXT_LOGICAL_START
 * @internal
 * This runs through the text in logical order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_OT_X_OFF and the like.
 * @see EVAS_FONT_WALK_OT_TEXT_INIT
 * @see EVAS_FONT_WALK_OT_TEXT_WORK
 * @see EVAS_FONT_WALK_OT_TEXT_END
 * @see EVAS_FONT_WALK_OT_TEXT_VISUAL_START
 */
#define EVAS_FONT_WALK_OT_TEXT_LOGICAL_START() \
   do \
     { \
        int _char_index_d, _i; \
        int visible; \
        /* Load the font needed for this script */ \
          { \
             /* Skip common chars */ \
             const Eina_Unicode *tmp; \
             for (tmp = text ; \
                   *tmp && \
                   evas_common_language_char_script_get(*tmp) == \
                   EVAS_SCRIPT_COMMON ; \
                   tmp++) \
               ; \
             if (!*tmp && (tmp > text)) tmp--; \
             evas_common_font_glyph_search(fn, &fi, *tmp); \
          } \
        prev_index = 0; \
        _i = intl_props->ot_data->len; \
        if (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) \
          { \
             char_index = intl_props->ot_data->len - 1; \
             _char_index_d = -1; \
          } \
        else \
          { \
             char_index = 0; \
             _char_index_d = 1; \
          } \
        for ( ; _i > 0 ; char_index += _char_index_d, _i--) \
          { \
             FT_UInt index; \
             RGBA_Font_Glyph *fg; \

/*FIXME: doc */
#define EVAS_FONT_WALK_OT_X_OFF \
             (EVAS_FONT_OT_X_OFF_GET( \
                      intl_props->ot_data->items[char_index]) >> 6)
#define EVAS_FONT_WALK_OT_Y_OFF \
             (EVAS_FONT_OT_Y_OFF_GET( \
                      intl_props->ot_data->items[char_index]) >> 6)
#define EVAS_FONT_WALK_OT_X_BEAR (fg->glyph_out->left)
#define EVAS_FONT_WALK_OT_Y_BEAR (fg->glyph_out->top)
#define EVAS_FONT_WALK_OT_X_ADV \
             (EVAS_FONT_OT_X_ADV_GET( \
                      intl_props->ot_data->items[char_index]) >> 6)
#define EVAS_FONT_WALK_OT_WIDTH (fg->glyph_out->bitmap.width)
#define EVAS_FONT_WALK_OT_POS \
             (EVAS_FONT_OT_POS_GET( \
                      intl_props->ot_data->items[char_index]))
#define EVAS_FONT_WALK_OT_IS_LAST \
             (char_index + 1 == intl_props->ot_data->len)
#define EVAS_FONT_WALK_OT_IS_FIRST \
             (!char_index)
#define EVAS_FONT_WALK_OT_POS_NEXT \
             ((!EVAS_FONT_WALK_OT_IS_LAST) ? \
             EVAS_FONT_OT_POS_GET( \
                      intl_props->ot_data->items[char_index + 1]) : \
              EVAS_FONT_WALK_OT_POS \
             )
#define EVAS_FONT_WALK_OT_POS_PREV \
             ((char_index > 0) ? \
             EVAS_FONT_OT_POS_GET( \
                      intl_props->ot_data->items[char_index - 1]) : \
              EVAS_FONT_WALK_OT_POS \
             )
#define EVAS_FONT_WALK_OT_LEN (intl_props->ot_data->len)
/**
 * @def EVAS_FONT_WALK_OT_TEXT_WORK
 * @internal
 * This macro actually updates the values mentioned in EVAS_FONT_WALK_OT_TEXT_VISUAL_START
 * according to the current positing in the walk.
 * @see EVAS_FONT_WALK_OT_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_OT_TEXT_INIT
 * @see EVAS_FONT_WALK_OT_TEXT_END
 */
#define EVAS_FONT_WALK_OT_TEXT_WORK(is_visual) \
             index = EVAS_FONT_OT_INDEX_GET(intl_props->ot_data->items[char_index]); \
             LKL(fi->ft_mutex); \
             fg = evas_common_font_int_cache_glyph_get(fi, index); \
             if (!fg) \
               { \
                  LKU(fi->ft_mutex); \
                  continue; \
               } \
             if (EVAS_FONT_CHARACTER_IS_INVISIBLE(text[EVAS_FONT_WALK_OT_POS])) \
               { \
                  visible = 0; \
               } \
             else \
               { \
                  visible = 1; \
               } \
 \
             pface = fi->src->ft.face; \
             LKU(fi->ft_mutex);

/**
 * @def EVAS_FONT_WALK_OT_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_OT_TEXT_VISUAL_START, needs to end with a ;
 * @see EVAS_FONT_WALK_OT_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_OT_TEXT_INIT
 * @see EVAS_FONT_WALK_OT_TEXT_WORK
 */
#define EVAS_FONT_WALK_OT_TEXT_END() \
             if (visible) \
               { \
                  pen_x += EVAS_FONT_WALK_OT_X_ADV; \
               } \
             prev_index = index; \
          } \
     } \
   while(0)


#endif
