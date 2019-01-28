/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_ERROR_H_
#define EINA_ERROR_H_

#include <stdarg.h>

#include "eina_types.h"


/**
 * @page tutorial_error_page Error Tutorial
 *
 * @section tutorial_error_registering_msg Registering messages
 *
 * The error module can provide a system that extends the errno system
 * of the C standard library. It consists in 2 parts:
 *
 * @li A way of registering new messages with
 * eina_error_msg_register() and eina_error_msg_get(),
 * @li A way of setting / getting the last error message with
 * eina_error_set() / eina_error_get().
 *
 * So one has to first register all the error messages that a program
 * or a library should manage. Then, when an error occurs, use
 * eina_error_set(), and when errors are managed, use
 * eina_error_get(). If eina_error_set() is used to set an error, do
 * not forget to remove previous set errors before calling eina_error_set().
 *
 * Here is an example for use:
 *
 * @include eina_error_01.c
 *
 * Of course, instead of printf(), eina_log_print() can be used to
 * have beautiful error messages.
 */

/**
 * @defgroup Eina_Error_Group Error
 * @ingroup Eina_Tools_Group
 *
 * @brief This group discusses the functions that provide error management for projects.
 *
 * The Eina error module provides a way to manage errors in a simple
 * but powerful way in libraries and modules. It is also used in Eina
 * itself.  An extension to libC's @c errno and strerror() facilities,
 * this is extensible and recommended for other libraries and
 * applications as well.
 *
 * While libC's @c errno is fixed, Eina_Error can be extended by
 * libraries and applications with domain-specific codes and
 * associated error messages. All @c errno.h codes are usable
 * seamlessly with Eina_Error. The constants defined in errno.h will
 * have messages as reported by strerror() in eina_error_msg_get()
 *
 * Success (no-error) condition is reported with Eina_Error 0
 * (#EINA_ERROR_NO_ERROR).
 *
 * A simple example of how to use this can be seen @ref tutorial_error_page
 * "here".
 *
 * @{
 */

/**
 * @typedef Eina_Error
 * @brief The integer type containing the error type.
 *
 * Errors are registered with eina_error_msg_register(),
 * eina_error_msg_static_register() or those inherited from the
 * system's @c errno.h such as @c ENOMEM.
 */
typedef int Eina_Error;

/**
 * @def EINA_ERROR_NO_ERROR
 * @brief No error reported.
 *
 * This is a convenience macro for people that are extra verbose, same
 * as "0".
 */
#define EINA_ERROR_NO_ERROR ((Eina_Error)0)

/**
 * @var EINA_ERROR_OUT_OF_MEMORY
 * @brief The error identifier corresponding to lack of memory.
 *
 * @deprecated since 1.19, same as @c ENOMEM from @c errno.h
 */
EAPI extern Eina_Error EINA_ERROR_OUT_OF_MEMORY EINA_DEPRECATED; /* use ENOMEM */

/**
 * @brief Registers a new error type.
 * @details This function stores the error message described by
 *          @p msg in a list. The returned value is a unique identifier greater than or equal
 *          to @c 1. The description can be retrieved later by passing
 *          the returned value to eina_error_msg_get().
 *
 * @param[in] msg The description of the error \n
 *                It is duplicated using eina_stringshare_add().
 * @return The unique number identifier for this error
 *
 * @note There is no need to register messages that exist in libC's @c
 *       errno.h, such as @c ENOMEM or @c EBADF.
 *
 * @see eina_error_msg_static_register()
 */
EAPI Eina_Error  eina_error_msg_register(const char *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Registers a new error type, statically allocated message.
 * @details This function stores the error message described by
 *          @p msg in a list. The returned value is a unique identifier greater than or equal
 *          to @c 1. The description can be retrieved later by passing
 *          the returned value to eina_error_msg_get().
 *
 * @param[in] msg The description of the error \n
 *            This string is not duplicated and thus
 *            the given pointer should live during the usage of eina_error.
 * @return The unique number identifier for this error
 *
 * @note There is no need to register messages that exist in libC's @c
 *       errno.h, such as @c ENOMEM or @c EBADF.
 *
 * @see eina_error_msg_register()
 */
EAPI Eina_Error  eina_error_msg_static_register(const char *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Changes the message of an already registered message.
 * @details This function modifies the message associated with @p error and changes
 *          it to @p msg. If the error is previously registered by @ref eina_error_msg_static_register
 *          then the string is not duplicated, otherwise the previous message
 *          is unref'ed and @p msg is copied.
 *
 * @param[in] error The Eina_Error to change the message of
 * @param[in] msg The description of the error \n
 *            This string is duplicated only if the error is registered with @ref eina_error_msg_register,
 *            otherwise it must remain intact for the duration.
 * @return #EINA_TRUE if successful, otherwise #EINA_FALSE on error
 *
 * @note It is not possible to modify messages that exist in libC's @c
 *       errno.h, such as @c ENOMEM or @c EBADF.
 *
 *
 * @see eina_error_msg_register()
 */
EAPI Eina_Bool   eina_error_msg_modify(Eina_Error  error,
                                       const char *msg) EINA_ARG_NONNULL(2);

/**
 * @brief Returns the last set error.
 * @details This function returns the last error set by eina_error_set(). The
 *          description of the message is returned by eina_error_msg_get().
 *
 * @return The last error or 0 (#EINA_ERROR_NO_ERROR).
 *
 * @note This function is thread safe @since 1.10, but slower to use.
 */
EAPI Eina_Error  eina_error_get(void);

/**
 * @brief Sets the last error.
 * @details This function sets the last error identifier. The last error can be
 *          retrieved by eina_error_get().
 *
 * @param[in] err The error identifier
 *
 * @note This is also used to clear previous errors, in which case @p err should
 *        be @c 0 (#EINA_ERROR_NO_ERROR).
 *
 * @note This function is thread safe @since 1.10, but slower to use.
 */
EAPI void        eina_error_set(Eina_Error err);

/**
 * @brief Returns the description of the given error number.
 * @details This function returns the description of an error that has been
 *          registered by eina_error_msg_register(). If an incorrect error is
 *          given, then @c NULL is returned.
 * @param[in] error The error number
 * @return The description of the error
 *
 */
EAPI const char *eina_error_msg_get(Eina_Error error) EINA_PURE;

/**
 * @brief Finds the #Eina_Error corresponding to a message string.
 * @details This function attempts to match @p msg with its corresponding #Eina_Error value.
 *          If no such value is found, @c 0 is returned.
 *
 * @param[in] msg The error message string to match (NOT @c NULL)
 * @return The #Eina_Error matching @p msg, otherwise @c 0 on failure
 *
 * @note this function only works for explicitly registered errors
 *       such as the messages given to eina_error_msg_register(),
 *       eina_error_msg_static_register() or modified with
 *       eina_error_msg_modify().
 */
EAPI Eina_Error  eina_error_find(const char *msg) EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @}
 */

#endif /* EINA_ERROR_H_ */
