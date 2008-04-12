#ifndef         _EVAS_ENGINE_SDL_16_H
# define        _EVAS_ENGINE_SDL_16_H

#include <SDL/SDL.h>

typedef struct _Evas_Engine_Info_SDL_16 Evas_Engine_Info_SDL_16;
struct _Evas_Engine_Info_SDL_16
{
  /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
  /* at you and make nasty noises */
  Evas_Engine_Info magic;

  struct {
    int                         rotation;
    int                         fullscreen : 1;
    int                         hwsurface : 1;
    int                         noframe : 1;
    int                         alpha : 1;
  } info;
};

#endif
