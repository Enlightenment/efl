#ifndef _ECORE_FB_PRIVATE_H
#define _ECORE_FB_PRIVATE_H

/* hacks to stop people NEEDING #include <linux/h3600_ts.h> */
#ifndef TS_SET_CAL
#define TS_SET_CAL 0x4014660b
#endif
#ifndef TS_GET_CAL
#define TS_GET_CAL 0x8014660a
#endif
#ifndef TS_SET_BACKLIGHT
#define TS_SET_BACKLIGHT 0x40086614
#endif
#ifndef TS_GET_BACKLIGHT
#define TS_GET_BACKLIGHT 0x80086614
#endif
#ifndef LED_ON
#define LED_ON 0x40046605
#endif
#ifndef TS_SET_CONTRAST
#define TS_SET_CONTRAST 0x40046615
#endif
#ifndef TS_GET_CONTRAST
#define TS_GET_CONTRAST 0x80046615
#endif
#ifndef FLITE_ON
#define FLITE_ON 0x40046607
#endif

#endif
