#ifndef _EO2_SIMPLE_H
#define _EO2_SIMPLE_H

#include "Eo.h"

EAPI void eo2_inc();

EAPI int eo2_get();

EAPI void eo2_set(int x);

EAPI void eo2_set_evt(int x);

EAPI int eo2_virtual(int in);

EAPI void eo2_class_hello(int a);

EAPI void eo2_simple_constructor(int x);

extern const Eo_Event_Description _EO2_EV_X_CHANGED;
#define EO2_EV_X_CHANGED (&(_EO2_EV_X_CHANGED))

EAPI const Eo_Class *eo2_simple_class_get(void);
#define EO2_SIMPLE_CLASS eo2_simple_class_get()

#endif
