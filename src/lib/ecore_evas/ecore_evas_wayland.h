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
   Ecore_Wl2_Window *(*window2_get)(const Ecore_Evas *ee);
   void (*pre_post_swap_callback_set)(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e));
   void (*aux_hint_add)(Ecore_Evas *ee, int id, const char *hint, const char *val);
   void (*aux_hint_change)(Ecore_Evas *ee, int id, const char *val);
   void (*aux_hint_del)(Ecore_Evas *ee, int id);
};

#endif
