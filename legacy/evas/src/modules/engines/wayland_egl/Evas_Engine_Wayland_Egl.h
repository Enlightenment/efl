#ifndef _EVAS_ENGINE_WAYLAND_EGL_H
#define _EVAS_ENGINE_WAYLAND_EGL_H

/*
 * Wayland supoprt is considered experimental as wayland itself is still
 * unstable and liable to change core protocol. If you use this api, it is
 * possible it will break in future, until this notice is removed.
 */

#include <wayland-client.h>

typedef struct _Evas_Engine_Info_Wayland_Egl Evas_Engine_Info_Wayland_Egl;

struct _Evas_Engine_Info_Wayland_Egl
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      struct wl_display *display;
      struct wl_surface *surface;
      int          depth;
      int          screen;
      int          rotation;
      unsigned int destination_alpha  : 1;
   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
   struct {
      int       (*best_depth_get)    (Evas_Engine_Info_Wayland_Egl *einfo);
   } func;

   struct {
      void      (*pre_swap)          (void *data, Evas *e);
      void      (*post_swap)         (void *data, Evas *e);

      void       *data; // data for callback calls
   } callback;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   unsigned char vsync : 1; // does nothing right now
   unsigned char indirect : 1; // use indirect rendering
};
#endif
