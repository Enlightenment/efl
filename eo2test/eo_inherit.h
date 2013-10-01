#ifndef EO_INHERIT_H
#define EO_INHERIT_H

#include "Eo.h"

extern EAPI Eo_Op EO_INHERIT_BASE_ID;

enum {
     EO_INHERIT_SUB_ID_GET,
     EO_INHERIT_SUB_ID_LAST
};

#define EO_INHERIT_ID(sub_id) (EO_INHERIT_BASE_ID + sub_id)

#define eo_inherit_get(x) EO_INHERIT_ID(EO_INHERIT_SUB_ID_GET), EO_TYPECHECK(int *, x)

#define EO_INHERIT_CLASS inherit_class_get()
const Eo_Class *inherit_class_get(void);

#endif
