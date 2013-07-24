#ifndef _EO2_SIMPLE_H
#define _EO2_SIMPLE_H

#include "Eo.h"

EAPI void inc2();

EAPI int get2();

EAPI void set2(int x);

EAPI const Eo_Class *eo2_simple_class_get(void);
#define EO2_SIMPLE_CLASS eo2_simple_class_get()

#endif
