/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_TRASH_H__
#define EINA_TRASH_H__

/**
 * @defgroup Eina_Trash_Group Trash
 * @ingroup Eina_Containers_Group
 * @brief This group provides a generic container.
 *
 * @{
 */

/**
 * @typedef Eina_Trash
 * @brief The type for structure #_Eina_Trash.
 */
typedef struct _Eina_Trash Eina_Trash;

/**
 * @struct _Eina_Trash
 * @brief The structure type for a generic container of an unused allocated pointer.
 */
struct _Eina_Trash
{
   Eina_Trash *next; /**< Next item in the trash */
};

/**
 * @brief Initializes a trash before using it.
 * @details This function just set to zero the trash to correctly
 *          initialize it.
 *
 * @param[in] trash The trash
 *
 * @note You can just set *trash to @c NULL and you will have
 *       the same result.
 */
static inline void  eina_trash_init(Eina_Trash **trash) EINA_ARG_NONNULL(1);

/**
 * @brief Pushes an unused pointer in the trash instead of freeing it.
 * @details Instead of freeing a pointer and put pressure on malloc/free
 *          you can push it in a trash for a later use. This function just
 *          provide a fast way to push a now unused pointer into a trash.
 *
 * @param[in,out] trash A pointer to an Eina_Trash
 * @param[in] data An unused pointer big enough to put a (void*)
 *
 * @note Do not use the pointer after insertion or bad things will
 *       happens.
 *
 * @note This trash will not resize, nor do anything with the size of
 *       the region pointed by @p data, so it's your duty to manage the size.
 */
static inline void  eina_trash_push(Eina_Trash **trash, void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Pops an available pointer from the trash if possible.
 * @details Instead of calling malloc, and putting pressure on malloc/free
 *          you can recycle the content of the trash, if it's not empty.
 *
 * @param[in] trash A #Eina_Trash handle
 *
 * @note This trash will not resize, nor do anything with the size of
 *       the region pointed by pointer inside the trash, so it's your duty
 *       to manage the size of the returned pointer.
 */
static inline void *eina_trash_pop(Eina_Trash **trash) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @def EINA_TRASH_CLEAN
 * @brief Definition for a macro to remove all the pointers from the trash.
 * @details This macro allows the cleaning of @a trash in an easy way. It
 *          removes all the pointers from @a trash until it's empty.
 *
 * @param[in,out] trash The trash to clean
 * @param[out] data The pointer extracted from the trash
 *
 * @note This macro can be used for freeing the data in the trash, like in
 *       the following example:
 *
 * @code
 * Eina_Trash *trash = NULL;
 * char *data;
 *
 * // trash is filled with a pointer to some duped strings.
 *
 * EINA_TRASH_CLEAN(&trash, data)
 *   free(data);
 * @endcode
 *
 * @note This macro is useful when you implement some memory pool.
 */
#define EINA_TRASH_CLEAN(trash, data) while ((data = eina_trash_pop(trash)))

#include "eina_inline_trash.x"

/**
 * @}
 */

#endif /* EINA_TRASH_H_ */
