#ifndef _EVAS_ENGINE_FB_H
#define _EVAS_ENGINE_FB_H

typedef struct _Evas_Engine_Info_FB Evas_Engine_Info_FB;

struct _Evas_Engine_Info_FB
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {

      int virtual_terminal;
      int device_number;
      int refresh;
      int rotation;
   } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};
#endif


