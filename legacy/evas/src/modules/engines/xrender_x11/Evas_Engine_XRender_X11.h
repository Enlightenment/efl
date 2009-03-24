#ifndef _EVAS_ENGINE_XRENDER_X11_H
#define _EVAS_ENGINE_XRENDER_X11_H

#include <X11/Xlib.h>

typedef enum
{
  EVAS_ENGINE_INFO_XRENDER_BACKEND_XLIB,
  EVAS_ENGINE_INFO_XRENDER_BACKEND_XCB
} Evas_Engine_Info_XRender_Backend;

typedef struct _Evas_Engine_Info_XRender_X11 Evas_Engine_Info_XRender_X11;

/*
 *               Xlib       |      XCB
 * connection | Display *   |  xcb_connection_t *
 * screen     | NULL        |  xcb_screen_t *
 * drawable   | Drawable    |  xcb_drawable_t
 * mask       | Pixmap      |  xcb_pixmap_t
 * visual     | Visual *    |  xcb_visualtype_t *
 * colormap   | Colormap    |  xcb_colormap_t
 */

struct _Evas_Engine_Info_XRender_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      Evas_Engine_Info_XRender_Backend backend;
      void                            *connection;
      void                            *screen;
      unsigned int                     drawable;
      unsigned int                     mask;
      void                            *visual;
      unsigned char                    destination_alpha : 1;
   } info;
};
#endif
