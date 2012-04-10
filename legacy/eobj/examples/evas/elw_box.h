#ifndef ELW_BOX_H
#define ELW_BOX_H

#include "Eobj.h"

extern EAPI Eobj_Op ELW_BOX_BASE_ID;

enum {
     ELW_BOX_SUB_ID_PACK_END,
     ELW_BOX_SUB_ID_LAST
};

#define ELW_BOX_ID(sub_id) (ELW_BOX_BASE_ID + sub_id)

#define ELW_BOX_PACK_END(obj) ELW_BOX_ID(ELW_BOX_SUB_ID_PACK_END), EOBJ_TYPECHECK(Eobj *, obj)

#define ELW_BOX_CLASS elw_box_class_get()
const Eobj_Class *elw_box_class_get(void) EINA_CONST;

#endif
