#ifndef _EVAS_FONT_DEFAULT_WALK_X
#define _EVAS_FONT_DEFAULT_WALK_X
/* Macros for text walking */

/**
 * @def EVAS_FONT_UPDATE_KERN()
 * @internal
 * This macro updates kern according to kerning.
 * This macro assumes the following variables exist:
 * intl_props, char_index, fi, kern, index, prev_index
 */
#ifdef BIDI_SUPPORT
#define EVAS_FONT_UPDATE_KERN(is_visual) \
   do \
      { \
         /* if it's rtl, the kerning matching should be reversed, */ \
         /* i.e prev index is now the index and the other way */ \
         /* around. There is a slight exception when there are */ \
         /* compositing chars involved.*/ \
         if (intl_props && (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) && \
               visible && !is_visual) \
           { \
              if (evas_common_font_query_kerning(fi, index, prev_index, &kern)) \
                pen_x += kern; \
           } \
         else \
           { \
              if (evas_common_font_query_kerning(fi, prev_index, index, &kern)) \
                pen_x += kern; \
           } \
      } \
   while (0)
#else
#define EVAS_FONT_UPDATE_KERN(is_visual) \
   do \
      { \
         (void) is_visual; \
         if (evas_common_font_query_kerning(fi, prev_index, index, &kern)) \
           pen_x += kern; \
      } \
   while (0)
#endif

/**
 * @def EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START
 * @internal
 * This runs through the text in visual order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_DEFAULT_X_OFF and the like.
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_INIT
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_WORK
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_END
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START
 */
#define EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START() \
   do \
     { \
        const Eina_Unicode *_base_text; \
        int _char_index_d, _i; \
        int visible; \
        prev_index = 0; \
        _base_text = text; \
        for ( ; *text ; text++); \
        _i = text - _base_text; \
        if (intl_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) \
          { \
             char_index = text - _base_text - 1; \
             text--; \
             _char_index_d = -1; \
          } \
        else \
          { \
             char_index = 0; \
             text = _base_text; \
             _char_index_d = 1; \
          } \
        for ( ; _i > 0 ; char_index += _char_index_d, text += _char_index_d, _i--) \
          { \
             FT_UInt index; \
             RGBA_Font_Glyph *fg; \
             int _gl, kern; \
             _gl = *text; \
             if (_gl == 0) break;

/**
 * @def EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START
 * @internal
 * This runs through the text in logical order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_DEFAULT_X_OFF and the like.
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_INIT
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_WORK
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_END
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START
 */
#define EVAS_FONT_WALK_DEFAULT_TEXT_LOGICAL_START() \
   do \
     { \
        int visible; \
        prev_index = 0; \
        for (char_index = 0 ; *text ; text++, char_index++) \
          { \
             FT_UInt index; \
             RGBA_Font_Glyph *fg; \
             int _gl, kern; \
             _gl = *text; \
             if (_gl == 0) break;

/*FIXME: doc */
#define EVAS_FONT_WALK_DEFAULT_X_OFF (0)
#define EVAS_FONT_WALK_DEFAULT_Y_OFF (0)
#define EVAS_FONT_WALK_DEFAULT_X_BEAR (fg->glyph_out->left)
#define EVAS_FONT_WALK_DEFAULT_Y_BEAR (fg->glyph_out->top)
#define EVAS_FONT_WALK_DEFAULT_X_ADV (fg->glyph->advance.x >> 16)
#define EVAS_FONT_WALK_DEFAULT_Y_ADV (0)
#define EVAS_FONT_WALK_DEFAULT_WIDTH (fg->glyph_out->bitmap.width)
#define EVAS_FONT_WALK_DEFAULT_POS (char_index)
#define EVAS_FONT_WALK_DEFAULT_IS_LAST \
             (!text[char_index])
#define EVAS_FONT_WALK_DEFAULT_IS_FIRST \
             (!char_index)
#define EVAS_FONT_WALK_DEFAULT_POS_NEXT \
             ((!EVAS_FONT_WALK_DEFAULT_IS_LAST) ? \
                      (char_index + 1) : \
              (char_index) \
             )
#define EVAS_FONT_WALK_DEFAULT_POS_PREV \
             ((!EVAS_FONT_WALK_DEFAULT_IS_FIRST) ? \
             (char_index - 1) : \
              EVAS_FONT_WALK_DEFAULT_POS \
             )
#define EVAS_FONT_WALK_DEFAULT_LEN (EVAS_FONT_WALK_ORIG_LEN)
/**
 * @def EVAS_FONT_WALK_DEFAULT_TEXT_WORK
 * @internal
 * This macro actually updates the values mentioned in EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START
 * according to the current positing in the walk.
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_INIT
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_END
 */
#define EVAS_FONT_WALK_DEFAULT_TEXT_WORK(is_visual) \
             index = evas_common_font_glyph_search(fn, &fi, _gl); \
             LKL(fi->ft_mutex); \
             fg = evas_common_font_int_cache_glyph_get(fi, index); \
             if (!fg) \
               { \
                  LKU(fi->ft_mutex); \
                  continue; \
               } \
             kern = 0; \
             if (EVAS_FONT_CHARACTER_IS_INVISIBLE(_gl)) \
               { \
                  visible = 0; \
               } \
             else \
               { \
                  visible = 1; \
               } \
             /* hmmm kerning means i can't sanely do my own cached metric */ \
             /* tables! grrr - this means font face sharing is kinda... not */ \
             /* an option if you want performance */ \
             if ((use_kerning) && (prev_index) && (index) && \
                   (pface == fi->src->ft.face)) \
               { \
                  EVAS_FONT_UPDATE_KERN(is_visual); \
               } \
 \
             pface = fi->src->ft.face; \
             LKU(fi->ft_mutex); \

/**
 * @def EVAS_FONT_WALK_DEFAULT_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START, needs to end with a ;
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_INIT
 * @see EVAS_FONT_WALK_DEFAULT_TEXT_WORK
 */
#define EVAS_FONT_WALK_DEFAULT_TEXT_END() \
             if (visible) \
               { \
                  pen_x += EVAS_FONT_WALK_DEFAULT_X_ADV; \
               } \
             prev_index = index; \
          } \
     } \
   while(0)


#endif
