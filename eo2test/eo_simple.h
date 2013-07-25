#ifndef EO_SIMPLE_H
#define EO_SIMPLE_H

#include "Eo.h"

extern EAPI Eo_Op EO_SIMPLE_BASE_ID;

enum {
     EO_SIMPLE_SUB_ID_INC,
     EO_SIMPLE_SUB_ID_GET,
     EO_SIMPLE_SUB_ID_SET,
     EO_SIMPLE_SUB_ID_LAST
};

#define EO_SIMPLE_ID(sub_id) (EO_SIMPLE_BASE_ID + sub_id)

#define inc() EO_SIMPLE_ID(EO_SIMPLE_SUB_ID_INC)

#define get(x) EO_SIMPLE_ID(EO_SIMPLE_SUB_ID_GET), EO_TYPECHECK(int *, x)

#define set(x) EO_SIMPLE_ID(EO_SIMPLE_SUB_ID_SET), EO_TYPECHECK(int, x)

#define EO_SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
