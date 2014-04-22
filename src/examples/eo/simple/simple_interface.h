#ifndef INTERFACE_H
#define INTERFACE_H

#include "Eo.h"

/**
 * @def interface_a_power_3_get(ret)
 * @brief Get a^3
 * @return integer value
 */
EAPI int interface_a_power_3_get(void);

#define INTERFACE_CLASS interface_class_get()
const Eo_Class *interface_class_get(void);

#endif
