#ifndef SIMPLE_H
#define SIMPLE_H

#include "Eo.h"
#include "eo_isa_interface.h"
#include "eo_isa_mixin.h"

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_A_GET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

/**
 * @def simple_a_set(a)
 * @brief Set value to a-property
 * @param[in] a integer value to set
 */
#define simple_a_set(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EO_TYPECHECK(int, a)

/**
 * @def simple_a_get(a)
 * @brief Get value of a-property
 * @param[out] integer pointer to a-value
 */
#define simple_a_get(a) SIMPLE_ID(SIMPLE_SUB_ID_A_GET), EO_TYPECHECK(int *, a)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
