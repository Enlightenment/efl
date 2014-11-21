/* EINA - EFL data type library
 * Copyright (C) 2011 Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @brief Enumeration of extended attribute creation.
 *
 * @since 1.1
 */
typedef enum {
  EINA_XATTR_INSERT, /**< This is the default behaviour, it either creates or replaces the extended attribute */
  EINA_XATTR_REPLACE, /**< This only succeeds if the extended attribute previously existed */
  EINA_XATTR_CREATED /**< This only succeeds if the extended attribute isn't previously set */
} Eina_Xattr_Flags;

typedef struct _Eina_Xattr Eina_Xattr;
struct _Eina_Xattr
{
   const char *name; /**< The extended attribute name @since 1.2 */
   const char *value; /**< The extended attribute value @since 1.2 */

   size_t length; /**< The length of the extended attribute value @since 1.2 */
};

/**
 * @brief Gets an iterator that lists all the extended attributes of a file.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks The iterator does not allocate any data during the iteration step, so you need to copy them yourself
 *          if you need.
 *
 * @param[in] file The file name to retrieve the extended attribute list from
 * @return An iterator
 *
 */
EAPI Eina_Iterator *eina_xattr_ls(const char *file);

/**
 * @brief Gets an iterator that lists all the extended attribute values related to a file.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The iterator does not allocate any data during the iteration step, so you need to copy them yourself
 *          if you need them. The iterator provides an Eina_Xattr structure.
 *
 * @param[in] file The file name to retrieve the extended attribute list from
 * @return An iterator
 *
 */
EAPI Eina_Iterator *eina_xattr_value_ls(const char *file);

/**
 * @brief Gets an iterator that lists all extended attribute related to a file.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The iterator does not allocate any data during the iteration step, so you need to copy them yourself
 *          if you need them.
 *
 * @param[in] fd The file descriptor to retrieve the extended attribute list from
 * @return An iterator
 *
 */
EAPI Eina_Iterator *eina_xattr_fd_ls(int fd);

/**
 * @brief Gets an iterator that lists all extended attribute value related to a file.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The iterator does not allocate any data during the iteration step, so you need to copy them yourself
 *          if you need them. The iterator provides an Eina_Xattr structure.
 *
 * @param[in] fd The file descriptor to retrieve the extended attribute list from
 * @return An iterator
 *
 */
EAPI Eina_Iterator *eina_xattr_value_fd_ls(int fd);

/**
 * @brief Gets an extended attribute from a file.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks It returns @c NULL and *size is @c 0 if it fails.
 *
 * @param[in] file The file to retrieve the extended attribute from
 * @param[in] attribute The extended attribute name to retrieve
 * @param[out] size The size of the retrieved extended attribute
 * @return The allocated data that hold the extended attribute value
 *
 */
EAPI void *eina_xattr_get(const char *file, const char *attribute, ssize_t *size);

/**
 * @brief Sets an extended attribute to a file.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] file The file to set the extended attribute to
 * @param[in] attribute The attribute to set
 * @param[in] data The data to set
 * @param[in] length The length of the data to set
 * @param[in] flags The flag that defines the set policy
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_set(const char *file, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags);

/**
 * @brief Sets a string as an extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] file The file to set the string to
 * @param[in] attribute The attribute to set
 * @param[in] data The NULL-terminated string to set
 * @param[in] flags The flag that defines the set policy
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_string_set(const char *file, const char *attribute, const char *data, Eina_Xattr_Flags flags);

/**
 * @brief Gets a string from an extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This call makes sure that the string is properly NULL-terminated before returning it.
 *
 * @param[in] file The file to get the string from
 * @param[in] attribute The attribute to get
 * @return A valid string on success, otherwise @c NULL
 *
 */
EAPI char *eina_xattr_string_get(const char *file, const char *attribute);

/**
 * @brief Sets a double as an extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] file The file to set the double to
 * @param[in] attribute The attribute to set
 * @param[in] value The NULL-terminated double to set
 * @param[in] flags The flag that defines the set policy
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_double_set(const char *file, const char *attribute, double value, Eina_Xattr_Flags flags);

/**
 * @brief Gets a double from an extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This call makes sure that the double is correctly set.
 *
 * @param[in] file The file to get the string from
 * @param[in] attribute The attribute to get
 * @param[out] value The extracted value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_double_get(const char *file, const char *attribute, double *value);

/**
 * @brief Sets an integer as a extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @param[in] file The file to set the integer to
 * @param[in] attribute The attribute to set
 * @param[in] value The NULL-terminated integer to set
 * @param[in] flags The flag that defines the set policy
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_int_set(const char *file, const char *attribute, int value, Eina_Xattr_Flags flags);

/**
 * @brief Gets a integer from an extended attribute property.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks This call makes sure that the integer is correctly set.
 *
 * @param[in] file The file to get the string from
 * @param[in] attribute The attribute to get
 * @param[out] value The extracted value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_xattr_int_get(const char *file, const char *attribute, int *value);

/**
 * @}
 */

#endif
