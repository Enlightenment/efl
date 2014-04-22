#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eo.h"
#include "eo_isa_interface.h"
#include "eo_isa_mixin.h"

/**
 * @def simple_a_set(a)
 * @brief Set value to a-property
 * @param[in] a integer value to set
 */
EAPI void simple_a_set(int a);

/**
 * @def simple_a_get(a)
 * @brief Get value of a-property
 * @param[out] integer pointer to a-value
 */
EAPI int simple_a_get(void);

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
