#ifndef _EVAS_ENGINE_SOFTWARE_8_X11_H
#define _EVAS_ENGINE_SOFTWARE_8_X11_H

#include <xcb/xcb.h>

typedef struct _Evas_Engine_Info_Software_8_X11              Evas_Engine_Info_Software_8_X11;

struct _Evas_Engine_Info_Software_8_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      xcb_connection_t *connection;
      xcb_drawable_t drawable;
      xcb_gcontext_t gc;
      xcb_screen_t *screen;

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
   } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
