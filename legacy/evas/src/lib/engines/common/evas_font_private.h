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

/* Macros for text walking */
#  define OTLOCK()
#  define OTUNLOCK()
# endif

void evas_common_font_source_unload(RGBA_Font_Source *fs);
void evas_common_font_source_reload(RGBA_Font_Source *fs);

void evas_common_font_int_promote(RGBA_Font_Int *fi);
void evas_common_font_int_use_increase(int size);
void evas_common_font_int_use_trim(void);
void evas_common_font_int_unload(RGBA_Font_Int *fi);
void evas_common_font_int_reload(RGBA_Font_Int *fi);

/* 6th bit is on is the same as frac part >= 0.5 */
# define EVAS_FONT_ROUND_26_6_TO_INT(x) \
   (((x + 0x20) & -0x40) >> 6)

# define EVAS_FONT_CHARACTER_IS_INVISIBLE(x) ( \
      ((0x200C <= (x)) && ((x) <= 0x200D)) || /* ZWNJ..ZWH */ \
      ((0x200E <= (x)) && ((x) <= 0x200F)) || /* BIDI stuff */ \
      ((0x202A <= (x)) && ((x) <= 0x202E)) /* BIDI stuff */ \
      )

# include "evas_font_default_walk.x"
#endif /* !_EVAS_FONT_PRIVATE_H */
