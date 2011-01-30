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
        size_t char_index; \
        (void) _pen_y; /* Sometimes it won't be used */

/**
 * @def EVAS_FONT_WALK_TEXT_VISUAL_START
 * @internal
 * This runs through the text in visual order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_X_OFF and the like.
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 * @see EVAS_FONT_WALK_TEXT_LOGICAL_START
 */
#define EVAS_FONT_WALK_TEXT_VISUAL_START() \
   do \
     { \
        int visible = 1; \
        for (char_index = text_props->start ; char_index < text_props->start + text_props->len ; char_index++) \
          {

/**
 * @def EVAS_FONT_WALK_TEXT_LOGICAL_START
 * @internal
 * This runs through the text in logical order while updating char_index,
 * which is the current index in the text.
 * Does not end with a ;
 * Take a look at EVAS_FONT_WALK_X_OFF and the like.
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 * @see EVAS_FONT_WALK_TEXT_VISUAL_START
 */
#ifdef BIDI_SUPPORT
#define EVAS_FONT_WALK_TEXT_LOGICAL_START() \
   do \
     { \
        int _char_index_d, _i; \
        int visible = 1; \
        _i = text_props->len; \
        if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) \
          { \
             char_index = text_props->len - 1; \
             _char_index_d = -1; \
          } \
        else \
          { \
             char_index = 0; \
             _char_index_d = 1; \
          } \
        char_index += text_props->start; \
        _i += text_props->start; \
        for ( ; _i > text_props->start ; char_index += _char_index_d, _i--) \
          {
#else
#define EVAS_FONT_WALK_TEXT_LOGICAL_START() EVAS_FONT_WALK_TEXT_VISUAL_START()
#endif

/*FIXME: doc */
#ifdef OT_SUPPORT
# define EVAS_FONT_WALK_X_OFF \
             ((text_props->info->ot) ? \
             (EVAS_FONT_ROUND_26_6_TO_INT( \
                EVAS_FONT_OT_X_OFF_GET( \
                   text_props->info->ot[char_index]))) : \
              (0))
# define EVAS_FONT_WALK_Y_OFF \
             ((text_props->info->ot) ? \
             (EVAS_FONT_ROUND_26_6_TO_INT( \
                EVAS_FONT_OT_Y_OFF_GET( \
                   text_props->info->ot[char_index]))) : \
              (0))
# define EVAS_FONT_WALK_POS \
             ((text_props->info->ot) ? \
             (EVAS_FONT_OT_POS_GET( \
                      text_props->info->ot[char_index])) : \
              (char_index))
# define EVAS_FONT_WALK_POS_NEXT \
             ((text_props->info->ot) ? \
              ((!EVAS_FONT_WALK_IS_LAST) ? \
               EVAS_FONT_OT_POS_GET( \
                                     text_props->info->ot[char_index + 1]) : \
               EVAS_FONT_WALK_POS \
              ) : \
              ((!EVAS_FONT_WALK_IS_LAST) ? \
               (char_index + 1) : EVAS_FONT_WALK_POS))
# define EVAS_FONT_WALK_POS_PREV \
             ((text_props->info->ot) ? \
             ((char_index > 0) ? \
             EVAS_FONT_OT_POS_GET( \
                      text_props->info->ot[char_index - 1]) : \
              EVAS_FONT_WALK_POS \
             ) : \
             ((char_index > 0) ? \
              (char_index - 1) : EVAS_FONT_WALK_POS))
#else
# define EVAS_FONT_WALK_X_OFF 0
# define EVAS_FONT_WALK_Y_OFF 0
# define EVAS_FONT_WALK_POS (char_index)
# define EVAS_FONT_WALK_POS_NEXT \
             ((!EVAS_FONT_WALK_IS_LAST) ? \
              (char_index + 1) : EVAS_FONT_WALK_POS)
# define EVAS_FONT_WALK_POS_PREV \
             ((char_index > 0) ? \
              (char_index - 1) : EVAS_FONT_WALK_POS)
#endif

#define EVAS_FONT_WALK_X_BEAR (text_props->info->glyph[char_index].x_bear)
#define EVAS_FONT_WALK_Y_BEAR (fg->glyph_out->top)
#define _EVAS_FONT_WALK_X_ADV \
                (text_props->info->glyph[char_index].advance)
#define EVAS_FONT_WALK_WIDTH (text_props->info->glyph[char_index].width)

#define EVAS_FONT_WALK_X_ADV \
             (EVAS_FONT_ROUND_26_6_TO_INT(_EVAS_FONT_WALK_X_ADV))
#define EVAS_FONT_WALK_PEN_X (EVAS_FONT_ROUND_26_6_TO_INT(_pen_x))
#define EVAS_FONT_WALK_PEN_Y (EVAS_FONT_ROUND_26_6_TO_INT(_pen_y))
#define EVAS_FONT_WALK_Y_ADV (0)
#define EVAS_FONT_WALK_IS_LAST \
             (char_index + 1 == text_props->start + text_props->len)
#define EVAS_FONT_WALK_IS_FIRST \
             (!char_index)
#define EVAS_FONT_WALK_LEN (text_props->len)

/**
 * @def EVAS_FONT_WALK_TEXT_WORK
 * @internal
 * This macro actually updates the values mentioned in EVAS_FONT_WALK_TEXT_VISUAL_START
 * according to the current positing in the walk.
 * @see EVAS_FONT_WALK_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_WORK() do {} while(0);

/**
 * @def EVAS_FONT_WALK_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_TEXT_VISUAL_START, needs to end with a ;
 * @see EVAS_FONT_WALK_TEXT_VISUAL_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 */
#define EVAS_FONT_WALK_TEXT_END() \
             if (visible) \
               { \
                  _pen_x += _EVAS_FONT_WALK_X_ADV; \
               } \
          } \
     } \
   while(0)


#endif
