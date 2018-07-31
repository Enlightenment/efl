/*
 * Copyright (C) 2013, 2014 Mike Blumenkrantz
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


#ifndef EINA_INLINE_VALUE_UTIL_H_
#define EINA_INLINE_VALUE_UTIL_H_

#include "eina_value.h"

/**
 * @addtogroup Eina_Value
 *
 * @{
 */

/**
 * @brief Creates a basic #Eina_Value struct desc with refcounting.
 *
 * @return The #Eina_Value_Struct_Desc on success, @c NULL on failure
 *
 * @since 1.12
 */
EAPI Eina_Value_Struct_Desc *eina_value_util_struct_desc_new(void);

/**
 * @brief Creates a new #Eina_Value containing the passed parameter.
 *
 * @param[in] timestr The value to use
 * @return The #Eina_Value
 *
 * @since 1.12
 */
EAPI Eina_Value *eina_value_util_time_string_new(const char *timestr);

#include "eina_inline_value_util.x"

/**
 * @}
 */

#endif
