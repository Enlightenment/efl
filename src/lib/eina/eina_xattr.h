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
 *
 * @since 1.1
 */
typedef enum {
  EINA_XATTR_INSERT, /**< This is the default behaviour, it will either create or replace the extended attribute */
  EINA_XATTR_REPLACE, /**< This will only succeed if the extended attribute previously existed */
  EINA_XATTR_CREATED /**< This will only succeed if the extended attribute wasn't previously set */
} Eina_Xattr_Flags;

typedef struct _Eina_Xattr Eina_Xattr;
struct _Eina_Xattr
{
   const char *name; /**< The eXtended attribute name @since 1.2 */
   const char *value; /**< The eXtended attribute value @since 1.2 */

   size_t length; /**< The length of the eXtended attribute value @since 1.2 */
};

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
EAPI Eina_Iterator *eina_xattr_ls(const char *file) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get an iterator that list all extended attribute value related to a fd.
 *
 * @param file The filename to retrieve the extended attribute list from.
 * @return an iterator.
 *
 * The iterator will not allocate any data during the iteration step, so you need to copy them yourself
 * if you need. The iterator will provide an Eina_Xattr structure.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_xattr_value_ls(const char *file) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get an iterator that list all extended attribute related to a fd.
 *
 * @param fd The file descriptor to retrieve the extended attribute list from.
 * @return an iterator.
 *
 * The iterator will not allocate any data during the iteration step, so you need to copy them yourself
 * if you need.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_xattr_fd_ls(int fd) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get an iterator that list all extended attribute value related to a fd.
 *
 * @param fd The file descriptor to retrieve the extended attribute list from.
 * @return an iterator.
 *
 * The iterator will not allocate any data during the iteration step, so you need to copy them yourself
 * if you need. The iterator will provide an Eina_Xattr structure.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_xattr_value_fd_ls(int fd) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Copy the extended attribute from one file to another.
 * @param src source file to use as input.
 * @param dst destination file to use as output.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_xattr_fd_copy()
 * @since 1.8
 */
EAPI Eina_Bool eina_xattr_copy(const char *src, const char *dst) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Copy the extended attribute from one file descriptor to another.
 * @param src source file descriptor to use as input.
 * @param dst destination file descriptor to use as output.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_xattr_copy()
 * @since 1.8
 */
EAPI Eina_Bool eina_xattr_fd_copy(int src, int dst);

/**
 * @brief Retrieve an extended attribute from a file.
 *
 * @param file The file to retrieve the extended attribute from.
 * @param attribute The extended attribute name to retrieve.
 * @param size The size of the retrieved extended attribute.
 * @return the allocated data that hold the extended attribute value.
 *
 * It will return @c NULL and *size will be @c 0 if it fails.
 *
 * @since 1.1
 */
EAPI void *eina_xattr_get(const char *file, const char *attribute, ssize_t *size) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Retrieve an extended attribute from a file descriptor.
 *
 * @param fd The file descriptor to retrieve the extended attribute from.
 * @param attribute The extended attribute name to retrieve.
 * @param size The size of the retrieved extended attribute.
 * @return the allocated data that hold the extended attribute value.
 *
 * It will return @c NULL and *size will be @c 0 if it fails.
 *
 * @since 1.8
 */
EAPI void *eina_xattr_fd_get(int fd, const char *attribute, ssize_t *size) EINA_ARG_NONNULL(2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set an extended attribute on a file.
 *
 * @param file The file to set the extended attribute to.
 * @param attribute The attribute to set.
 * @param data The data to set.
 * @param length The length of the data to set.
 * @param flags Define the set policy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @since 1.1
 */
EAPI Eina_Bool eina_xattr_set(const char *file, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Set an extended attribute on a file descriptor.
 *
 * @param fd The file descriptor to set the extended attribute to.
 * @param attribute The attribute to set.
 * @param data The data to set.
 * @param length The length of the data to set.
 * @param flags Define the set policy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @since 1.8
 */
EAPI Eina_Bool eina_xattr_fd_set(int fd, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags) EINA_ARG_NONNULL(2, 3);


/**
 * @brief Delete (remove) an extended attribute from a file.
 *
 * @param file The file to del the extended attribute from.
 * @param attribute The attribute to del.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @since 1.8
 */
EAPI Eina_Bool eina_xattr_del(const char *file, const char *attribute) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Delete (remove) an extended attribute from a file descriptor.
 *
 * @param fd The file descriptor to del the extended attribute from.
 * @param attribute The attribute to del.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @since 1.8
 */
EAPI Eina_Bool eina_xattr_fd_del(int fd, const char *attribute) EINA_ARG_NONNULL(2);

/**
 * @brief Set a string as a extended attribute properties.
 *
 * @param file The file to set the string to.
 * @param attribute The attribute to set.
 * @param data The NULL-terminated string to set.
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
 * @return A valid string on success, @c NULL otherwise.
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
 * @param value The NULL-terminated double to set.
 * @param flags Define the set policy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @param value The NULL-terminated int to set.
 * @param flags Define the set policy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
