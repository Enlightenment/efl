#ifndef MIXIN_H
#define MIXIN_H

#include "Eo.h"

extern EAPI Eo_Op MIXIN_BASE_ID;

enum {
     MIXIN_SUB_ID_A_SQUARE_GET,
     MIXIN_SUB_ID_LAST
};

#define MIXIN_ID(sub_id) (MIXIN_BASE_ID + sub_id)


/**
 * @def mixin_a_square_get(ret)
 * @brief Get the square of a.
 * @param[out] ret the square of a
 */
#define mixin_a_square_get(ret) MIXIN_ID(MIXIN_SUB_ID_A_SQUARE_GET), EO_TYPECHECK(int *, ret)

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
