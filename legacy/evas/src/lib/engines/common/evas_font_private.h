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
        int pen_x = 0, pen_y = 0; \
        size_t char_index; \
        FT_UInt prev_index; \
        FT_Face pface = NULL; \
        int _len = eina_unicode_strlen(text); \
        (void) _len; /* We don't have to use it */ \
        (void) pen_y; /* Sometimes it won't be used */

<<<<<<< HEAD
/**
 * @def EVAS_FONT_WALK_TEXT_START
 * @internal
 * This runs through the variable text while updating char_index,
 * which is the current index in the text. This macro exposes (inside
 * the loop) the following vars:
 * adv - advancement
 * gl - the current unicode code point
 * bear_x, bear_y, width - info about the bitmap
 * pen_x, pen_y - (also available outside of the loop, but updated here)
 * fg - the font glyph.
 * index, prev_index - font indexes.
 * Does not end with a ;
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_START() \
   do \
     { \
        int adv; \
        int visible; \
        prev_index = 0; \
        for (char_index = 0 ; *text ; text++, char_index++) \
          { \
             FT_UInt index; \
             RGBA_Font_Glyph *fg; \
             int gl, kern; \
             int bear_x, bear_y, width; \
             gl = *text; \
             if (gl == 0) break;

/**
 * @def EVAS_FONT_WALK_TEXT_WORK
 * @internal
 * This macro actually updates the values mentioned in EVAS_FONT_WALK_TEXT_START
 * according to the current positing in the walk.
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_WORK(is_visual) \
             index = evas_common_font_glyph_search(fn, &fi, gl); \
             LKL(fi->ft_mutex); \
             fg = evas_common_font_int_cache_glyph_get(fi, index); \
             if (!fg) \
               { \
                  LKU(fi->ft_mutex); \
                  continue; \
               } \
             kern = 0; \
             if (EVAS_FONT_CHARACTER_IS_INVISIBLE(gl)) \
               { \
                  adv = width = bear_x = bear_y = 0; \
                  visible = 0; \
               } \
             else \
               { \
                  bear_x = fg->glyph_out->left; \
                  bear_y = fg->glyph_out->top; \
                  adv = fg->glyph->advance.x >> 16; \
                  width = fg->glyph_out->bitmap.width; \
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
 * @def EVAS_FONT_WALK_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_TEXT_START, needs to end with a ;
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 */
#define EVAS_FONT_WALK_TEXT_END() \
             if (visible) \
               { \
                  pen_x += adv; \
               } \
             prev_index = index; \
          } \
     } \
   while(0)

=======
>>>>>>> Evas: Multiple changes that all relate to the Harfbuzz integration:
#endif /* !_EVAS_FONT_PRIVATE_H */
