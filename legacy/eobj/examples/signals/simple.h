#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eobj.h"

extern EAPI Eobj_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define SIMPLE_A_SET(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EOBJ_TYPECHECK(int, a)

extern const Eobj_Event_Description _SIG_A_CHANGED;
#define SIG_A_CHANGED (&(_SIG_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eobj_Class *simple_class_get(void) EINA_CONST;

#endif
