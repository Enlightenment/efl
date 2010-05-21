#ifndef _EVAS_FONT_PRIVATE_H
#define _EVAS_FONT_PRIVATE_H

LK(lock_font_draw);	// for freetype2 API calls
LK(lock_fribidi);		// for fribidi API calls

#ifdef EVAS_FRAME_QUEUING
#define FTLOCK() LKL(lock_font_draw)
#define FTUNLOCK() LKU(lock_font_draw)

#define FBDLOCK() LKL(lock_fribidi)
#define FBDUNLOCK() LKU(lock_fribidi)
#else
#define FTLOCK(x) 
#define FTUNLOCK(x) 

#define FBDLOCK() 
#define FBDUNLOCK() 
#endif

#endif /* !_EVAS_FONT_PRIVATE_H */

#ifndef _EVAS_FONT_PRIVATE_H
#define _EVAS_FONT_PRIVATE_H

LK(lock_font_draw);	// for freetype2 API calls
LK(lock_fribidi);		// for fribidi API calls

#ifdef EVAS_FRAME_QUEUING
#define FTLOCK() LKL(lock_font_draw)
#define FTUNLOCK() LKU(lock_font_draw)

#define FBDLOCK() LKL(lock_fribidi)
#define FBDUNLOCK() LKU(lock_fribidi)
#else
#define FTLOCK(x) 
#define FTUNLOCK(x) 

#define FBDLOCK() 
#define FBDUNLOCK() 
#endif

#endif /* !_EVAS_FONT_PRIVATE_H */

#ifndef _EVAS_FONT_PRIVATE_H
#define _EVAS_FONT_PRIVATE_H

LK(lock_font_draw);	// for freetype2 API calls
LK(lock_fribidi);		// for fribidi API calls

#ifdef EVAS_FRAME_QUEUING
#define FTLOCK() LKL(lock_font_draw)
#define FTUNLOCK() LKU(lock_font_draw)

#define FBDLOCK() LKL(lock_fribidi)
#define FBDUNLOCK() LKU(lock_fribidi)
#else
#define FTLOCK(x) 
#define FTUNLOCK(x) 

#define FBDLOCK() 
#define FBDUNLOCK() 
#endif

#endif /* !_EVAS_FONT_PRIVATE_H */

#ifndef _EVAS_FONT_PRIVATE_H
#define _EVAS_FONT_PRIVATE_H

LK(lock_font_draw);

#ifdef EVAS_FRAME_QUEUING
#define FTLOCK() LKL(lock_font_draw)
#define FTUNLOCK() LKU(lock_font_draw)
#else
#define FTLOCK(x) 
#define FTUNLOCK(x) 
#endif

#endif /* !_EVAS_FONT_PRIVATE_H */

