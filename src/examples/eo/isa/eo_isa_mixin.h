#ifndef MIXIN_H
#define MIXIN_H

#include "Eo.h"

/**
 * @def mixin_a_square_get(ret)
 * @brief Get the square of a.
 * @param[out] ret the square of a
 */
EAPI int mixin_a_square_get(void);

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
