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

#endif /* !_EVAS_FONT_PRIVATE_H */
