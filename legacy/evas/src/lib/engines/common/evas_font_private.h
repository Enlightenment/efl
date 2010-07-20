#ifndef _EVAS_FONT_PRIVATE_H
# define _EVAS_FONT_PRIVATE_H

#ifdef BUILD_PTHREAD
extern LK(lock_font_draw); // for freetype2 API calls
extern LK(lock_fribidi); // for fribidi API calls
#endif

# if defined(EVAS_FRAME_QUEUING) || defined(BUILD_PIPE_RENDER)
#  define FTLOCK() LKL(lock_font_draw)
#  define FTUNLOCK() LKU(lock_font_draw)

#  define FBDLOCK() LKL(lock_fribidi)
#  define FBDUNLOCK() LKU(lock_fribidi)
# else
#  define FTLOCK(x) 
#  define FTUNLOCK(x) 

#  define FBDLOCK() 
#  define FBDUNLOCK() 
# endif

#endif /* !_EVAS_FONT_PRIVATE_H */
