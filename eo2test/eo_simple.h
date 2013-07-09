#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eo.h"

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_SET,
     SIMPLE_SUB_ID_A_GET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define simple_set(a, b) SIMPLE_ID(SIMPLE_SUB_ID_SET), EO_TYPECHECK(int, a), EO_TYPECHECK(int, b)

#define simple_a_get(a) SIMPLE_ID(SIMPLE_SUB_ID_A_GET), EO_TYPECHECK(int *, a)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
