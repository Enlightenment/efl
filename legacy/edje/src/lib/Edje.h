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

   void         edje_part_geometry_get(Evas_Object *o, char *part, int *x, int *y, int *w, int *h);
       
#ifdef __cplusplus
}
#endif

#endif
