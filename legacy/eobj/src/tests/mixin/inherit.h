#ifndef INHERIT_H
#define INHERIT_H

#include "Eo.h"
#include "simple.h"
#include "mixin4.h"

#define INHERIT_CLASS inherit_class_get()
const Eo_Class *inherit_class_get(void);

#endif
