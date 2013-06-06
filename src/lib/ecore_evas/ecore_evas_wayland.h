#ifndef ECORE_EVAS_WAYLAND_H_
# define ECORE_EVAS_WAYLAND_H_

typedef struct _Ecore_Evas_Interface_Wayland Ecore_Evas_Interface_Wayland;

struct _Ecore_Evas_Interface_Wayland 
{
   Ecore_Evas_Interface base;

   void (*resize)(Ecore_Evas *ee, int location);
   void (*move)(Ecore_Evas *ee, int x, int y);
   void (*pointer_set)(Ecore_Evas *ee, int hot_x, int hot_y);
   void (*type_set)(Ecore_Evas *ee, int type);
   Ecore_Wl_Window* (*window_get)(const Ecore_Evas *ee);
};

#endif
