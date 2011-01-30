#ifndef _EVAS_FONT_PRIVATE_H
# define _EVAS_FONT_PRIVATE_H

#ifdef BUILD_PTHREAD
extern LK(lock_font_draw); // for freetype2 API calls
extern LK(lock_bidi); // for fribidi API calls
#endif

# if defined(EVAS_FRAME_QUEUING) || defined(BUILD_PIPE_RENDER)
#  define FTLOCK() LKL(lock_font_draw)
#  define FTUNLOCK() LKU(lock_font_draw)

#  define BIDILOCK() LKL(lock_bidi)
#  define BIDIUNLOCK() LKU(lock_bidi)
# else
#  define FTLOCK(x)
#  define FTUNLOCK(x)

#  define BIDILOCK()
#  define BIDIUNLOCK()
# endif

void evas_common_font_source_unload(RGBA_Font_Source *fs);
void evas_common_font_source_reload(RGBA_Font_Source *fs);

void evas_common_font_int_promote(RGBA_Font_Int *fi);
void evas_common_font_int_use_increase(int size);
void evas_common_font_int_use_trim(void);
void evas_common_font_int_unload(RGBA_Font_Int *fi);
void evas_common_font_int_reload(RGBA_Font_Int *fi);
/* Macros for text walking */
/**
 * @def EVAS_FONT_UPDATE_KERN()
 * @internal
 * This macro updates pen_x and kern according to kerning.
 * This macro assumes the following variables exist:
 * intl_props, char_index, adv, fi, kern, pen_x
 */
#ifdef BIDI_SUPPORT
#define EVAS_FONT_UPDATE_KERN() \
   do \
      { \
         /* if it's rtl, the kerning matching should be reversed, */ \
         /* i.e prev index is now the index and the other way */ \
         /* around. There is a slight exception when there are */ \
         /* compositing chars involved.*/ \
         if (intl_props && \
               evas_bidi_is_rtl_char(intl_props, char_index) && \
               adv > 0) \
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
#define EVAS_FONT_UPDATE_KERN() \
   do \
      { \
         if (evas_common_font_query_kerning(fi, prev_index, index, &kern)) \
           pen_x += kern; \
      } \
   while (0)
#endif

/**
 * @def EVAS_FONT_WALK_TEXT_INIT
 * @internal
 * This macro defines the variables that will later be used with the following
 * macros, and by font handling functions.
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_WORK
 * @see EVAS_FONT_WALK_TEXT_END
 */
#define EVAS_FONT_WALK_TEXT_INIT() \
        int pen_x = 0, pen_y = 0; \
        int char_index; \
        int last_adv; \
        FT_UInt prev_index; \
        FT_Face pface = NULL; \
        (void) pen_y; /* Sometimes it won't be used */

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
        prev_index = 0; \
        last_adv = 0; \
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
#define EVAS_FONT_WALK_TEXT_WORK() \
             index = evas_common_font_glyph_search(fn, &fi, gl); \
             LKL(fi->ft_mutex); \
             fg = evas_common_font_int_cache_glyph_get(fi, index); \
             if (!fg) \
               { \
                  LKU(fi->ft_mutex); \
                  continue; \
               } \
             kern = 0; \
             bear_x = fg->glyph_out->left; \
             bear_y = fg->glyph_out->top; \
             adv = fg->glyph->advance.x >> 16; \
             width = fg->glyph_out->bitmap.width; \
             /* hmmm kerning means i can't sanely do my own cached metric */ \
             /* tables! grrr - this means font face sharing is kinda... not */ \
             /* an option if you want performance */ \
             if ((use_kerning) && (prev_index) && (index) && \
                   (pface == fi->src->ft.face)) \
               { \
                  EVAS_FONT_UPDATE_KERN(); \
               } \
 \
             pface = fi->src->ft.face; \
             LKU(fi->ft_mutex); \
             /* If the current one is not a compositing char, do the */ \
             /* previous advance and set the current advance as the next */ \
             /* advance to do */ \
             if (adv > 0) \
               { \
                  pen_x += last_adv; \
                  last_adv = adv; \
               } \

/**
 * @def EVAS_FONT_WALK_TEXT_END
 * @internal
 * Closes EVAS_FONT_WALK_TEXT_START, needs to end with a ;
 * @see EVAS_FONT_WALK_TEXT_START
 * @see EVAS_FONT_WALK_TEXT_INIT
 * @see EVAS_FONT_WALK_TEXT_WORK
 */
#define EVAS_FONT_WALK_TEXT_END() \
             prev_index = index; \
          } \
     } \
   while(0)

#endif /* !_EVAS_FONT_PRIVATE_H */
