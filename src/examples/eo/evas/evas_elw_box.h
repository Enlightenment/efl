#ifndef ELW_BOX_H
#define ELW_BOX_H

#include "Eo.h"

extern EAPI Eo_Op ELW_BOX_BASE_ID;

enum {
     ELW_BOX_SUB_ID_PACK_END,
     ELW_BOX_SUB_ID_LAST
};

#define ELW_BOX_ID(sub_id) (ELW_BOX_BASE_ID + sub_id)

/**
 * @def elw_box_pack_end(obj)
 * @brief Pack object to the end of the box
 * @param[in] obj object to pack into box
 */
#define elw_box_pack_end(obj) ELW_BOX_ID(ELW_BOX_SUB_ID_PACK_END), EO_TYPECHECK(Eo *, obj)

#define ELW_BOX_CLASS elw_box_class_get()
const Eo_Class *elw_box_class_get(void);

#endif
