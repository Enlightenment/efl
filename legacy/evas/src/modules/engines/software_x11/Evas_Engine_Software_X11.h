#ifndef _EVAS_ENGINE_SOFTWARE_X11_H
#define _EVAS_ENGINE_SOFTWARE_X11_H

#include <X11/Xlib.h>

typedef enum
{
  EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB,
  EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB
} Evas_Engine_Info_Software_X11_Backend;

typedef struct _Evas_Engine_Info_Software_X11 Evas_Engine_Info_Software_X11;

/*
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
      Evas_Engine_Info_Software_X11_Backend backend;
      void                                 *connection;
      void                                 *screen;
      unsigned int                          drawable;
      unsigned int                          mask;
      void                                 *visual;
      unsigned int                          colormap;
      int                                   depth;
      int                                   rotation;

      unsigned int                          alloc_grayscale    : 1;
      unsigned int                          debug              : 1;
      unsigned int                          shape_dither       : 1;
      unsigned int                          destination_alpha  : 1;
      unsigned int                          track_mask_changes : 1;

      int                                   alloc_colors_max;
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


