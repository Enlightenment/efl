#ifndef _EVAS_ENGINE_GL_X11_H
#define _EVAS_ENGINE_GL_X11_H

#include <X11/Xlib.h>

typedef struct _Evas_Engine_Info_GL_X11              Evas_Engine_Info_GL_X11;

struct _Evas_Engine_Info_GL_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      Display  *display;
      Drawable  drawable;
      Visual   *visual;
      Colormap  colormap;
      int       depth;
   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
   struct {
      Visual *  (*best_visual_get)   (Display *disp, int screen);
      Colormap  (*best_colormap_get) (Display *disp, int screen);
      int       (*best_depth_get)    (Display *disp, int screen);
   } func;
};
#endif


