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
   Evas_Object *edje_add(Evas *evas);
   void         edje_file_set(Evas_Object *o, const char *file, const char *part);
       
#ifdef __cplusplus
}
#endif

#endif
