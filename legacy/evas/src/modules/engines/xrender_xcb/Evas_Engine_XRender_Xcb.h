#ifndef _EVAS_ENGINE_XRENDER_X11_H
#define _EVAS_ENGINE_XRENDER_X11_H

#include <X11/XCB/xcb.h>

typedef struct _Evas_Engine_Info_XRender_Xcb              Evas_Engine_Info_XRender_Xcb;

struct _Evas_Engine_Info_XRender_Xcb
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      XCBConnection *conn;
      XCBDRAWABLE    drawable;
      XCBPIXMAP      mask;
      XCBVISUALID    visual;
      unsigned char  destination_alpha : 1;
   } info;
};
#endif


