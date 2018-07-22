/* EINA - EFL data type library
 * Copyright (C) 2015-2016 Carsten Haitzler, Cedric Bail
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

#ifndef EINA_SAFEPOINTER_H__
#define EINA_SAFEPOINTER_H__

/**
 * @addtogroup Eina_Safepointer_Group Safe Pointer
 *
 * @brief These functions provide a wrapper that protect access to pointers
 *
 * Eina_Safepointer is an pointer to index converter that allows an increased
 * level of safety by forbidding direct access to the pointer. The protection
 * works by using a set of indirection tables that are mmapped and mprotected
 * against write access. Thus the pointer they store and that map to a specific
 * index is always correct. Also once a pointer is unregistered the index
 * won't be served back for 2^8 on 32 bits system and 2^28 on 64 bits system
 * for that specific slot. Finally we guarantee that the lower 2 bits of the
 * returned index are actually never used and completely ignored by our API.
 * So you can safely store whatever information you want in it, we will ignore
 * it and treat as if it wasn't there.
 *
 * @note The use of Eina_Safepointer is thread safe.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Safepointer_Group Safe Pointer
 *
 * @{
 */

/**
 * @typedef Eina_Safepointer
 * Type of the protected index.
 */
typedef struct _Eina_Safepointer Eina_Safepointer;

/**
 * @brief Register a pointer and get an Eina_Safepointer that maps to it.
 *
 * @param target The pointer to register.
 * @return A valid pointer that is an index to the mapped pointer.
 *
 * @note It will return @c NULL on error or if @p target is @c NULL.
 *
 * @note The lower 2 bits of the returned pointer will always be 0.
 *
 * @note The returned pointer can be used like a pointer, but cannot
 * be touched except with Eina_Safepointer functions.
 *
 * @since 1.18
 */
EAPI const Eina_Safepointer *eina_safepointer_register(const void *target);

/**
 * @brief Unregister an Eina_Safepointer and the pointer that maps to it.
 *
 * @param safe The index to unregister from the mapping.
 *
 * @note This function will ignore the lower 2 bits of the given pointer.
 *
 * @since 1.18
 */
EAPI void  eina_safepointer_unregister(const Eina_Safepointer *safe);

/**
 * @brief Get the associated pointer from an Eina_Safepointer mapping.
 *
 * @param safe The Eina_Safepointer index to lookup at.
 * @return The pointer registered with that index or @c NULL in any other case.
 *
 * @note It is always safe to ask for a pointer for any value of the mapping.
 * If the pointer is invalid or @c NULL, we will return @c NULL and not crash.
 */
static inline void *eina_safepointer_get(const Eina_Safepointer *safe);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */


# include "eina_inline_safepointer.x"

#endif
