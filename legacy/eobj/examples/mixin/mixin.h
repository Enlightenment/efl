#ifndef MIXIN_H
#define MIXIN_H

#include "Eobj.h"

extern EAPI Eobj_Op MIXIN_BASE_ID;

enum {
     MIXIN_SUB_ID_AB_SUM_GET,
     MIXIN_SUB_ID_LAST
};

#define MIXIN_ID(sub_id) (MIXIN_BASE_ID + sub_id)


/**
 * @def MIXIN_AB_SUM_GET(sum)
 * @brief Get sum of a,b integer elements
 * @param[out] sum integer pointer to sum - value
 */
#define MIXIN_AB_SUM_GET(sum) MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), EOBJ_TYPECHECK(int *, sum)

#define MIXIN_CLASS mixin_class_get()
const Eobj_Class *mixin_class_get(void) EINA_CONST;

#endif
