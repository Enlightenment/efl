#ifndef _EDJE_H
#define _EDJE_H

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif
   
   void         edje_init                       (void);
   void         edje_frametime_set              (double t);
   double       edje_frametime_get              (void);
   
   Evas_Object *edje_object_add                 (Evas *evas);
   void         edje_object_file_set            (Evas_Object *o, const char *file, const char *part);
   void         edje_object_file_get            (Evas_Object *o, const char **file, const char **part);
   void         edje_object_signal_callback_add (Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data);
   void        *edje_object_signal_callback_del (Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source));
   void         edje_object_signal_emit         (Evas_Object *o, const char *emission, const char *source);
   void         edje_object_play_set            (Evas_Object *o, int play);
   int          edje_object_play_get            (Evas_Object *o);
   void         edje_object_animation_set       (Evas_Object *o, int on);
   int          edje_object_animation_get       (Evas_Object *o);
   int          edje_object_part_exists         (Evas_Object *o, const char *part);
   void         edje_object_part_geometry_get   (Evas_Object *o, const char *part, double *x, double *y, double *w, double *h);
   void         edje_object_part_text_set       (Evas_Object *o, const char *part, const char *text);
   const char  *edje_object_part_text_get       (Evas_Object *o, const char *part);
       
#ifdef __cplusplus
}
#endif

#endif
