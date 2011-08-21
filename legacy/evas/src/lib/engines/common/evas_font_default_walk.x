#ifndef _EVAS_FONT_DEFAULT_WALK_X
#define _EVAS_FONT_DEFAULT_WALK_X
/* Macros for text walking */

/**
 * @def EVAS_FONT_WALK_TEXT_INIT
 * @internal
 * This macro defines the variables that will later be used with the following
 * macros, and by font handling functions.
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 */
# define EVAS_FONT_WALK_TEXT_INIT() \
        int _pen_x = 0, _pen_y = 0; \
        Evas_Coord _start_pen = (text_props->info && \
              (text_props->start > 0)) ? \
           text_props->info->glyph[text_props->start - 1].pen_after : 0 ; \
        size_t char_index; \
        (void) _pen_y; /* Sometimes it won't be used */

/* Visual walk helper macros */
#ifdef OT_SUPPORT
#define _EVAS_FONT_WALK_TEXT_START() \
        Evas_Font_OT_Info *_ot_itr = (text_props->info) ? \
           text_props->info->ot + text_props->start : NULL; \
        for (char_index = 0 ; char_index < text_props->len ; char_index++, _glyph_itr++, _ot_itr++) \
          {
#else
#define _EVAS_FONT_WALK_TEXT_START() \
        for (char_index = 0 ; char_index < text_props->len ; char_index++, _glyph_itr++) \
          {
#endif

/**
 * @def EVAS_FONT_WALK_TEXT_START
 * @internal
 * This runs through the text in visual order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_X_OFF and the like.
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_START() \
   do \
     { \
        Evas_Font_Glyph_Info *_glyph_itr = (text_props->info) ? \
           text_props->info->glyph + text_props->start : NULL; \
        _EVAS_FONT_WALK_TEXT_START()

/*FIXME: doc */
#ifdef OT_SUPPORT
# define EVAS_FONT_WALK_X_OFF \
             ((_ot_itr) ? \
              (EVAS_FONT_ROUND_26_6_TO_INT(EVAS_FONT_OT_X_OFF_GET(*_ot_itr)) : \
              0)
# define EVAS_FONT_WALK_Y_OFF \
             ((_ot_itr) ? \
              (EVAS_FONT_ROUND_26_6_TO_INT(EVAS_FONT_OT_Y_OFF_GET(*_ot_itr)) : \
              0)
# define EVAS_FONT_WALK_POS \
             ((_ot_itr) ? \
              (EVAS_FONT_OT_POS_GET(*_ot_itr) - text_props->text_offset) : \
              0)
# define EVAS_FONT_WALK_POS_NEXT \
              ((!EVAS_FONT_WALK_IS_LAST) ? \
               ((_ot_itr) ? EVAS_FONT_OT_POS_GET(*(_ot_itr + 1)) : 0) - text_props->text_offset : \
               EVAS_FONT_WALK_POS \
              )
# define EVAS_FONT_WALK_POS_PREV \
             ((char_index > 0) ? \
              ((_ot_itr) ? EVAS_FONT_OT_POS_GET(*(_ot_itr - 1)) : 0) - text_props->text_offset : \
              EVAS_FONT_WALK_POS \
             )
#else
# define EVAS_FONT_WALK_X_OFF 0
# define EVAS_FONT_WALK_Y_OFF 0
# define EVAS_FONT_WALK_POS \
              ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) ? \
               (text_props->len - char_index - 1) : \
               (char_index))
# define EVAS_FONT_WALK_POS_NEXT \
             ((!EVAS_FONT_WALK_IS_LAST) ? \
              ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) ? \
               text_props->len - char_index - 2 \
               : (char_index + 1)) : \
              EVAS_FONT_WALK_POS)
# define EVAS_FONT_WALK_POS_PREV \
             ((char_index > 0) ? \
              ((text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) ? \
               text_props->len - char_index \
               : (char_index - 1)) : \
              EVAS_FONT_WALK_POS)
#endif


#define EVAS_FONT_WALK_IS_VISIBLE (_glyph_itr->index != 0)
#define EVAS_FONT_WALK_X_BEAR (_glyph_itr->x_bear)
#define EVAS_FONT_WALK_Y_BEAR (fg->glyph_out->top)
#define EVAS_FONT_WALK_X_ADV ((_glyph_itr > text_props->info->glyph) ? \
      _glyph_itr->pen_after - (_glyph_itr - 1)->pen_after : \
      _glyph_itr->pen_after)
#define EVAS_FONT_WALK_WIDTH (_glyph_itr->width)

#define EVAS_FONT_WALK_INDEX (_glyph_itr->index)
#define EVAS_FONT_WALK_PEN_X (_pen_x)
#define EVAS_FONT_WALK_PEN_X_AFTER (_glyph_itr->pen_after - _start_pen)
#define EVAS_FONT_WALK_PEN_Y (EVAS_FONT_ROUND_26_6_TO_INT(_pen_y))
#define EVAS_FONT_WALK_Y_ADV (0)
#define EVAS_FONT_WALK_IS_LAST \
             (char_index + 1 == text_props->len)
#define EVAS_FONT_WALK_IS_FIRST \
             (char_index == 0)
#define EVAS_FONT_WALK_LEN (text_props->len)

/**
 * @def EVAS_FONT_WALK_TEXT_WORK
 * @internal
 * This macro actually updates the values mentioned in EVAS_FONT_WALK_TEXT_START
 * according to the current positing in the walk.
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_WORK() do {} while(0)

/**
 * @def EVAS_FONT_WALK_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_TEXT_START, needs to end with a ;
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 */
#define EVAS_FONT_WALK_TEXT_END() \
             if (EVAS_FONT_WALK_IS_VISIBLE) \
               { \
                  _pen_x = _glyph_itr->pen_after - _start_pen; \
               } \
          } \
     } \
   while(0)

#endif
