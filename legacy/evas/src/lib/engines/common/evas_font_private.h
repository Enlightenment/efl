#ifndef _EVAS_FONT_PRIVATE_H
# define _EVAS_FONT_PRIVATE_H
#include "evas_font_ot.h"

#ifdef BUILD_PTHREAD
extern LK(lock_font_draw); // for freetype2 API calls
extern LK(lock_bidi); // for fribidi API calls
extern LK(lock_ot); // for harfbuzz calls
#endif

# if defined(EVAS_FRAME_QUEUING) || defined(BUILD_PIPE_RENDER)
#  define FTLOCK() LKL(lock_font_draw)
#  define FTUNLOCK() LKU(lock_font_draw)

#  define BIDILOCK() LKL(lock_bidi)
#  define BIDIUNLOCK() LKU(lock_bidi)

#  define OTLOCK() LKL(lock_ot)
#  define OTUNLOCK() LKU(lock_ot)
# else
#  define FTLOCK(x)
#  define FTUNLOCK(x)

#  define BIDILOCK()
#  define BIDIUNLOCK()

void evas_common_font_source_unload(RGBA_Font_Source *fs);
void evas_common_font_source_reload(RGBA_Font_Source *fs);

void evas_common_font_int_promote(RGBA_Font_Int *fi);
void evas_common_font_int_use_increase(int size);
void evas_common_font_int_use_trim(void);
void evas_common_font_int_unload(RGBA_Font_Int *fi);
void evas_common_font_int_reload(RGBA_Font_Int *fi);
/* Macros for text walking */
#  define OTLOCK()
#  define OTUNLOCK()
# endif

/* 6th bit is on is the same as frac part >= 0.5 */
# define EVAS_FONT_ROUND_26_6_TO_INT(x) \
   ((0x20 & x) ? ((x >> 6) + 1) : (x >> 6))

# define EVAS_FONT_CHARACTER_IS_INVISIBLE(x) ( \
      ((0x200C <= (x)) && ((x) <= 0x200D)) || /* ZWNJ..ZWH */ \
      ((0x200E <= (x)) && ((x) <= 0x200F)) || /* BIDI stuff */ \
      ((0x202A <= (x)) && ((x) <= 0x202E)) /* BIDI stuff */ \
      )

# define EVAS_FONT_WALK_ORIG_LEN (_len)

# ifdef OT_SUPPORT
#  include "evas_font_ot_walk.x"
# endif

# include "evas_font_default_walk.x"

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
        FT_UInt prev_index; \
        FT_Face pface = NULL; \
        int _len = eina_unicode_strlen(text); \
        (void) _len; /* We don't have to use it */ \
        (void) _pen_y; /* Sometimes it won't be used */

# define EVAS_FONT_WALK_PEN_X (EVAS_FONT_ROUND_26_6_TO_INT(_pen_x))

#endif /* !_EVAS_FONT_PRIVATE_H */
