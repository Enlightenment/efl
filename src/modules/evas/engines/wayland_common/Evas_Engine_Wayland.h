#ifndef _EVAS_ENGINE_WAYLAND_COMMON_H
# define _EVAS_ENGINE_WAYLAND_COMMON_H

#include "Ecore_Wl2.h"

typedef struct _Evas_Engine_Info_Wayland Evas_Engine_Info_Wayland;

struct _Evas_Engine_Info_Wayland
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out
    * at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct
     {
        Ecore_Wl2_Window *wl2_win;
        int depth, rotation;
        Eina_Bool destination_alpha : 1;
        Eina_Bool hidden : 1;
     } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   Eina_Bool indirect : 1;

   Eina_Bool www_avail : 1;
   Eina_Rectangle window;
   int x_rel, y_rel;
   int x_cursor, y_cursor;
   int resizing;
   uint32_t timestamp;
   Eina_Bool dragging : 1;
   Eina_Bool drag_start : 1;
   Eina_Bool drag_stop : 1;
   Eina_Bool drag_ack : 1;
};

#endif
