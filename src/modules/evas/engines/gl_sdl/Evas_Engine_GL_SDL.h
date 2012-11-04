#ifndef _EVAS_ENGINE_GL_SDL_H
#define _EVAS_ENGINE_GL_SDL_H

#include <SDL/SDL.h>

typedef struct _Evas_Engine_Info_GL_SDL              Evas_Engine_Info_GL_SDL;

struct _Evas_Engine_Info_GL_SDL
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
     int rotation;
     int fullscreen : 1;
     int noframe : 1;
   } flags;
   
};
#endif


