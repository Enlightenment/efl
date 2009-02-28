#ifndef _EVAS_ENGINE_SOFTWARE_X11_H
#define _EVAS_ENGINE_SOFTWARE_X11_H

#include <X11/Xlib.h>

typedef struct _Evas_Engine_Info_Software_X11 Evas_Engine_Info_Software_X11;

/*
 * backend: 0 == Xlib, 1 == XCB
 *
 *               Xlib       |      XCB
 * connection | Display *   |  xcb_connection_t *
 * screen     | NULL        |  xcb_screen_t *
 * drawable   | Drawable    |  xcb_drawable_t
 * mask       | Pixmap      |  xcb_pixmap_t
 * visual     | Visual *    |  xcb_visualtype_t *
 * colormap   | Colormap    |  xcb_colormap_t
 */

struct _Evas_Engine_Info_Software_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      int          backend;
      void        *connection;
      void        *screen;
      unsigned int drawable;
      unsigned int mask;
      void        *visual;
      unsigned int colormap;
      int          depth;
      int          rotation;

      int          alloc_grayscale    : 1;
      int          debug              : 1;
      int          shape_dither       : 1;
      int          destination_alpha  : 1;
      int          track_mask_changes : 1;

      int          alloc_colors_max;
   } info;
   /* engine specific function calls to query stuff about the destination */
   struct {
      void        *(*best_visual_get)   (int backend, void *connection, int screen);
      unsigned int (*best_colormap_get) (int backend, void *connection, int screen);
      int          (*best_depth_get)    (int backend, void *connection, int screen);
   } func;

   int mask_changed;
};

#endif


