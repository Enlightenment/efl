#ifndef INHERIT_H
#define INHERIT_H

#include "Eobj.h"

#define INHERIT_CLASS inherit_class_get()
const Eobj_Class *inherit_class_get(void) EINA_CONST;

#endif
