#ifndef INTERFACE_H
#define INTERFACE_H

#include "Eo.h"

extern EAPI Eo_Op INTERFACE_BASE_ID;

enum {
     INTERFACE_SUB_ID_A_POWER_3_GET,
     INTERFACE_SUB_ID_LAST
};

#define INTERFACE_ID(sub_id) (INTERFACE_BASE_ID + sub_id)


/**
 * @def interface_a_power_3_get(ret)
 * @brief Get a^3
 * @param[out] ret integer pointer to ret - value
 */
#define interface_a_power_3_get(ret) INTERFACE_ID(INTERFACE_SUB_ID_A_POWER_3_GET), EO_TYPECHECK(int *, ret)

#define INTERFACE_CLASS interface_class_get()
const Eo_Class *interface_class_get(void);

#endif
