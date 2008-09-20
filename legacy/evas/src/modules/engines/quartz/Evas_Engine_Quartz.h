#ifndef _EVAS_ENGINE_QUARTZ_H
#define _EVAS_ENGINE_QUARTZ_H

#include <ApplicationServices/ApplicationServices.h>

typedef struct _Evas_Engine_Info_Quartz Evas_Engine_Info_Quartz;

struct _Evas_Engine_Info_Quartz
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      CGContextRef context;
   } info;
};

#endif


