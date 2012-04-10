#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eobj.h"

extern EAPI Eobj_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_A_GET,
     SIMPLE_SUB_ID_B_SET,
     SIMPLE_SUB_ID_B_GET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define SIMPLE_A_SET(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EOBJ_TYPECHECK(int, a)
#define SIMPLE_A_GET(a) SIMPLE_ID(SIMPLE_SUB_ID_A_GET), EOBJ_TYPECHECK(int *, a)
#define SIMPLE_B_SET(b) SIMPLE_ID(SIMPLE_SUB_ID_B_SET), EOBJ_TYPECHECK(int, b)
#define SIMPLE_B_GET(b) SIMPLE_ID(SIMPLE_SUB_ID_B_GET), EOBJ_TYPECHECK(int *, b)

#define SIMPLE_CLASS simple_class_get()
const Eobj_Class *simple_class_get(void) EINA_CONST;

#endif
