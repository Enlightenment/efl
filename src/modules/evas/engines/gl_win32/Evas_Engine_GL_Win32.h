#ifndef _EVAS_ENGINE_GL_WIN32_H
#define _EVAS_ENGINE_GL_WIN32_H

#include "../gl_generic/Evas_Engine_GL_Shared.h"

typedef struct _Evas_Engine_Info_GL_Win32 Evas_Engine_Info_GL_Win32;

struct _Evas_Engine_Info_GL_Win32
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      void        *window;
      int          rotation;
      unsigned int destination_alpha  : 1;
   } info;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;

   unsigned char vsync : 1; // does nothing right now
   unsigned char indirect : 1; // use indirect rendering
   Evas_Engine_Info_Gl_Swap_Mode swap_mode : 4; // what swap mode to assume

   /* window surface should be made with these config */
   int           stencil_bits;
   int           msaa_bits;
};


#endif
