#ifndef INHERIT2_H
#define INHERIT2_H

#include "Eobj.h"

extern EAPI Eobj_Op INHERIT2_BASE_ID;

enum {
     INHERIT2_SUB_ID_PRINT,
     INHERIT2_SUB_ID_PRINT2,
     INHERIT2_SUB_ID_LAST
};

#define INHERIT2_ID(sub_id) (INHERIT2_BASE_ID + sub_id)

#define INHERIT2_PRINT() INHERIT2_ID(INHERIT2_SUB_ID_PRINT)
#define INHERIT2_PRINT2() INHERIT2_ID(INHERIT2_SUB_ID_PRINT2)

#define INHERIT2_CLASS inherit2_class_get()
const Eobj_Class *inherit2_class_get(void) EINA_CONST;

#endif
