#ifndef MIXIN_H
#define MIXIN_H

#include "eobj.h"

extern EAPI Eobj_Op MIXIN_BASE_ID;

enum {
     MIXIN_SUB_ID_ADD_AND_SET,
     MIXIN_SUB_ID_LAST
};

#define MIXIN_ID(sub_id) (MIXIN_BASE_ID + sub_id)

#define MIXIN_ADD_AND_PRINT(x) MIXIN_ID(MIXIN_SUB_ID_ADD_AND_SET), EOBJ_TYPECHECK(int, x)

#define MIXIN_CLASS mixin_class_get()
const Eobj_Class *mixin_class_get(void) EINA_CONST;

#endif
