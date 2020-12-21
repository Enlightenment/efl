#ifndef _EVAS_FONT_PRIVATE_H
#define _EVAS_FONT_PRIVATE_H

#include "evas_font.h"

/* macros needed to log message through eina_log */
EVAS_API extern int _evas_font_log_dom_global;
#ifdef  _EVAS_FONT_DEFAULT_LOG_DOM
# undef _EVAS_FONT_DEFAULT_LOG_DOM
#endif
#define _EVAS_FONT_DEFAULT_LOG_DOM _evas_font_log_dom_global

#ifdef EVAS_FONT_DEFAULT_LOG_COLOR
# undef EVAS_FONT_DEFAULT_LOG_COLOR
#endif
#define EVAS_FONT_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_EVAS_FONT_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_EVAS_FONT_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_EVAS_FONT_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_EVAS_FONT_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_EVAS_FONT_DEFAULT_LOG_DOM, __VA_ARGS__)


extern LK(lock_font_draw); // for freetype2 API calls
extern LK(lock_bidi); // for fribidi API calls
extern LK(lock_ot); // for harfbuzz calls

#  define FTLOCK()   LKL(lock_font_draw)
#  define FTUNLOCK() LKU(lock_font_draw)

#  define BIDILOCK()   LKL(lock_bidi)
#  define BIDIUNLOCK() LKU(lock_bidi)

/* Macros for text walking */
#  define OTLOCK()   LKL(lock_ot)
#  define OTUNLOCK() LKU(lock_ot)

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

# if 1
// do proper round (up or down like 1.4 -> 1 and 1.6 -> 2 etc
#  define FONT_METRIC_CONV(val, dv, scale) \
   (((long long)((val) * (scale)) + (long long)((dv) * (dv) / 2LL)) \
     / (long long)((dv) * (dv)))
#  define FONT_METRIC_ROUNDUP(val) \
   (((val) + 31) >> 6)
# else
// truncate/round down
#  define FONT_METRIC_CONV(val, dv, scale) \
   (((val) * (scale)) / ((dv) * (dv)))
#  define FONT_METRIC_ROUNDUP(val) \
   ((val) >> 6)
# endif

# include "evas_font_default_walk.x"
#endif /* !_EVAS_FONT_PRIVATE_H */
