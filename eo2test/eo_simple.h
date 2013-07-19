#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eo.h"

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_INC,
     SIMPLE_SUB_ID_GET,
     SIMPLE_SUB_ID_SET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define inc() SIMPLE_ID(SIMPLE_SUB_ID_INC)

#define get(x) SIMPLE_ID(SIMPLE_SUB_ID_GET), EO_TYPECHECK(int *, x)

#define set(x) SIMPLE_ID(SIMPLE_SUB_ID_SET), EO_TYPECHECK(int, x)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
