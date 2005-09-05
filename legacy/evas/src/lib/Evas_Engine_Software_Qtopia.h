#ifndef _EVAS_ENGINE_SOFTWARE_QTOPIA_H
#define _EVAS_ENGINE_SOFTWARE_QTOPIA_H

#ifdef EVAS_COMMON_H
#ifdef EVAS_PRIVATE_H
typedef void QWidget;
#endif
#endif

#ifndef EVAS_COMMON_H
#ifndef EVAS_PRIVATE_H
#include <qwidget.h>
#endif
#endif

typedef struct _Evas_Engine_Info_Software_Qtopia Evas_Engine_Info_Software_Qtopia;

struct _Evas_Engine_Info_Software_Qtopia
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      QWidget *target;
      int      rotation;
   } info;
};
#endif


