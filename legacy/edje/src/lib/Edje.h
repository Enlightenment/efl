#ifndef _EDJE_H
#define _EDJE_H

#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/* API here                                                                */
/***************************************************************************/
   
   void         edje_init(void);
   void         edje_frametime_set(double t);
   double       edje_frametime_get(void);
   Evas_Object *edje_add(Evas *evas);
   void         edje_file_set(Evas_Object *o, const char *file, const char *part);
   void         edje_signal_callback_add(Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data);
   void        *edje_signal_callback_del(Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source));
   void         edje_signal_emit(Evas_Object *o, const char *emission, const char *source);
   void         edje_play_set(Evas_Object *obj, int play);
   int          edje_play_get(Evas_Object *obj);
   void         edje_animation_set(Evas_Object *obj, int on);
   int          edje_animation_get(Evas_Object *obj);
   int          edje_part_exists(Evas_Object *obj, const char *part);
   void         edje_part_geometry_get(Evas_Object *o, const char *part, double *x, double *y, double *w, double *h);
   void         edje_part_text_set(Evas_Object *o, const char *part, const char *text);
   const char  *edje_part_text_get(Evas_Object *obj, const char *part);
       
#ifdef __cplusplus
}
#endif

#endif
