#ifndef INHERIT_H
#define INHERIT_H

#include "Eobj.h"

extern EAPI Eobj_Op INHERIT_BASE_ID;

enum {
     INHERIT_SUB_ID_PROT_PRINT,
     INHERIT_SUB_ID_LAST
};

#define INHERIT_ID(sub_id) (INHERIT_BASE_ID + sub_id)

#define inherit_prot_print() INHERIT_ID(INHERIT_SUB_ID_PROT_PRINT)

#define INHERIT_CLASS inherit_class_get()
const Eobj_Class *inherit_class_get(void) EINA_CONST;

#endif
