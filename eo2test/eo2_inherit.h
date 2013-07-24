#ifndef _EO2_INHERIT_H
#define _EO2_INHERIT_H

#include "Eo.h"

EAPI int get3();

EAPI const Eo_Class *eo2_inherit_class_get(void);
#define EO2_INHERIT_CLASS eo2_inherit_class_get()

#endif
