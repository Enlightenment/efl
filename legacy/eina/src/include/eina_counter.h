/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_COUNTER_H_
#define EINA_COUNTER_H_

#include <stdio.h>

#include "eina_types.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Counter_Group Counter
 *
 * @{
 */

/**
 * @typedef Eina_Counter
 * Counter type.
 */
typedef struct _Eina_Counter Eina_Counter;

EAPI int eina_counter_init(void);
EAPI int eina_counter_shutdown(void);

EAPI Eina_Counter *eina_counter_add(const char *name);
EAPI void eina_counter_delete(Eina_Counter *counter);

EAPI void eina_counter_start(Eina_Counter *counter);
EAPI void eina_counter_stop(Eina_Counter *counter, int specimen);
EAPI void eina_counter_dump(Eina_Counter *counter, FILE *out);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_COUNTER_H_ */
