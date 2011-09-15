/* EINA - EFL data type library
 * Copyright (C) 2011 Cedric Bail
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

#ifndef EINA_XATTR_H_
#define EINA_XATTR_H_

#include "eina_types.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @typedef Eina_Xattr_Flags
 * define extended attribute creation
 */
typedef enum {
  EINA_XATTR_INSERT, /**< This is the default behaviour, it will either create or replace the extended attribute */
  EINA_XATTR_REPLACE, /**< This will only succeed if the extended attribute previously existed */
  EINA_XATTR_CREATED /**< This will only succeed if the extended attribute wasn't previously set */
} Eina_Xattr_Flags;


/**
 * @brief Get an iterator that list all extended attribute of a file.
 *
 * @param file The filename to retrieve the extended attribute list from.
 * @return an iterator.
 *
 * The iterator will not allocate any data during the iteration step, so you need to copy them yourself
 * if you need.
 *
 * @since 1.1
 */
EAPI Eina_Iterator *eina_xattr_ls(const char *file);

/**
 * @brief Retrieve an extended attribute from a file.
 *
 * @param file The file to retrieve the extended attribute from.
 * @param atttribute The extended attribute name to retrieve.
 * @param size The size of the retrieved extended attribute.
 * @return the allocated data that hold the extended attribute value.
 *
 * It will return NULL and *size will be @c 0 if it fails.
 *
 * @since 1.1
 */
EAPI void *eina_xattr_get(const char *file, const char *attribute, ssize_t *size);

/**
 * @brief Set an extended attribute on a file.
 *
 * @param file The file to set the extended attribute to.
 * @param attribute The attribute to set.
 * @param data The data to set.
 * @param length The length of the data to set.
 * @param flags Define the set policy.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_set(const char *file, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags);

/**
 * @brief Set a string as a extended attribute properties.
 *
 * @param file The file to set the string to.
 * @param attribute The attribute to set.
 * @param data The NULL terminated string to set.
 * @param flags Define the set policy.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_string_set(const char *file, const char *attribute, const char *data, Eina_Xattr_Flags flags);

/**
 * @brief Get a string from an extended attribute properties.
 *
 * @param file The file to get the string from.
 * @param attribute The attribute to get.
 * @return a valid string on success, NULL otherwise.
 *
 * This call check that the string is properly NULL-terminated before returning it.
 *
 * @since 1.1
 */
EAPI char *eina_xattr_string_get(const char *file, const char *attribute);

/**
 * @brief Set a double as a extended attribute properties.
 *
 * @param file The file to set the double to.
 * @param attribute The attribute to set.
 * @param data The NULL terminated double to set.
 * @param flags Define the set policy.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_double_set(const char *file, const char *attribute, double value, Eina_Xattr_Flags flags);

/**
 * @brief Get a double from an extended attribute properties.
 *
 * @param file The file to get the string from.
 * @param attribute The attribute to get.
 * @param value Where to put the extracted value
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * This call check that the double is correctly set.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_double_get(const char *file, const char *attribute, double *value);

/**
 * @brief Set an int as a extended attribute properties.
 *
 * @param file The file to set the int to.
 * @param attribute The attribute to set.
 * @param data The NULL terminated int to set.
 * @param flags Define the set policy.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_int_set(const char *file, const char *attribute, int value, Eina_Xattr_Flags flags);

/**
 * @brief Get a int from an extended attribute properties.
 *
 * @param file The file to get the string from.
 * @param attribute The attribute to get.
 * @param value Where to put the extracted value
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * This call check that the int is correctly set.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_int_get(const char *file, const char *attribute, int *value);

/**
 * @}
 */

#endif
