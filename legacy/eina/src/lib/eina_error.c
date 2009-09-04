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


/**
 * @page tutorial_error_page Error Tutorial
 *
 * @section tutorial_error_introduction Introduction
 *
 * The Eina error module provides a way to manage errors in a simple
 * but powerful way in libraries and modules. It is also used in Eina
 * itself. Similar to libC's @c errno and strerror() facilities, this
 * is extensible and recommended for other libraries and applications.
 *
 * @section tutorial_error_basic_usage Basic Usage
 *
 * The first thing to do when using the error module is to initialize
 * it with eina_error_init() and, when the error module is not used
 * anymore, to shut down it with eina_error_shutdown(). So a basic
 * program would look like that:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_error.h>
 *
 * int main(void)
 * {
 *    if (!eina_error_init())
 *    {
 *        printf ("Error during the initialization of eina_error module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    eina_error_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * All program using any module of eina must be compiled with the
 * following command:
 *
 * @code
 * gcc -Wall -o my_exe my_source.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 *
 * @section tutorial_error_registering_msg Registering messages
 *
 * The error module can provide a system that mimic the errno system
 * of the C standard library. It consists in 2 parts:
 *
 * @li a way of registering new messages with
 * eina_error_msg_register() and eina_error_msg_get(),
 * @li a way of setting / getting last error message with
 * eina_error_set() / eina_error_get().
 *
 * So one has to fisrt register all the error messages that a program
 * or a lib should manage. Then, when an error can occur, use
 * eina_error_set(), and when errors are managed, use
 * eina_error_get(). If eina_error_set() is used to set an error, do
 * not forget to call before eina_error_set0), to remove previous set
 * errors.
 *
 * Here is an example of use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_error.h>
 *
 * Eina_Error MY_ERROR_NEGATIVE;
 * Eina_Error MY_ERROR_NULL;
 *
 * voi *data_new()
 * {
 *    eina_error_set(0);
 *
 *    eina_error_set(MY_ERROR_NULL);
 *    return NULL;
 * }
 *
 * int test(int n)
 * {
 *    eina_error_set(0);
 *
 *    if (n < 0)
 *    {
 *       eina_error_set(MY_ERROR_NEGATIVE);
 *       return 0;
 *    }
 *
 *    return 1;
 * }
 *
 * int main(void)
 * {
 *    void *data;
 *
 *    if (!eina_error_init())
 *    {
 *       printf ("Error during the initialization of eina_error module\n");
 *       return EXIT_FAILURE;
 *    }
 *
 *    MY_ERROR_NEGATIVE = eina_error_msg_register("Negative number");
 *    MY_ERROR_NULL = eina_error_msg_register("NULL pointer");

 *    data = data_new();
 *    if (!data)
 *    {
 *       Eina_Error err;
 *
 *       err = eina_error_get();
 *       if (err)
 *          printf("Error during memory allocation: %s\n",
 *                 eina_error_msg_get(err));
 *    }
 *
 *    if (!test(0))
 *    {
 *       Eina_Error err;
 *
 *       err = eina_error_get();
 *       if (err)
 *          printf("Error during test function: %s\n",
 *                 eina_error_msg_get(err));
 *    }
 *
 *    if (!test(-1))
 *    {
 *       Eina_Error err;
 *
 *       err = eina_error_get();
 *       if (err)
 *          printf("Error during test function: %s\n",
 *                 eina_error_msg_get(err));
 *    }
 *
 *    eina_error_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Of course, instead of printf(), eina_log_print() can be used to
 * have beautiful error messages.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_safety_checks.h"
#include "eina_error.h"
#include "eina_inlist.h"
#include "eina_log.h" /* remove me when eina_error_print is removed! */

/* TODO
 * + printing errors to stdout or stderr can be implemented
 * using a queue, useful for multiple threads printing
 * + add a wrapper for assert?
 * + add common error numbers, messages
 * + add a calltrace of errors, not only store the last error but a list of them
 * and also store the function that set it
 */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int _eina_error_init_count = 0;
static Eina_Inlist *_error_list = NULL;
static int _error_list_count = 0;
static Eina_Error _err;

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Error_Group Error
 *
 * @brief These functions provide error management for projects.
 *
 * The error system must be initialized with eina_error_init() and
 * shut down with eina_error_shutdown(). Error codes are registered
 * with eina_error_msg_register() and converted from identifier to
 * original message string with eina_error_msg_get().
 *
 * Logging functions are not in eina_error anymore, see
 * eina_log_print() instead.
 *
 * @{
 */


/**
 * @cond LOCAL
 */

EAPI Eina_Error EINA_ERROR_OUT_OF_MEMORY = 0;

/**
 * @endcond
 */

/**
 * @brief Initialize the error module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the error module of Eina. It is called by
 * eina_init() and by all modules initialization functions. It returns
 * @c 0 on failure, otherwise it returns the number of times it is
 * called.
 *
 * The default error level value is set by default to
 * #EINA_ERROR_LEVEL_DBG if Eina is compiled with debug mode, or to
 * #EINA_ERROR_LEVEL_ERR otherwise. That value can be overwritten by
 * setting the environment variable EINA_ERROR_LEVEL. This function
 * checks the value of that environment variable in the first
 * call. Its value must be a number between 0 and 3, to match the
 * error levels #EINA_ERROR_LEVEL_ERR, #EINA_ERROR_LEVEL_WARN,
 * #EINA_ERROR_LEVEL_INFO and #EINA_ERROR_LEVEL_DBG. That value can
 * also be set later with eina_error_log_level_set().
 *
 * This function registers the error #EINA_ERROR_OUT_OF_MEMORY.
 *
 * Once the error module is not used anymore, then
 * eina_error_shutdown() must be called to shut down the error
 * module.
 *
 * @see eina_init()
 */
EAPI int eina_error_init(void)
{
   _eina_error_init_count++;
   if (_eina_error_init_count != 1)
     return _eina_error_init_count;

   /* TODO register the eina's basic errors */
   EINA_ERROR_OUT_OF_MEMORY = eina_error_msg_register("Out of memory");

   if (!eina_safety_checks_init())
     {
	fprintf(stderr, "Could not initialize eina safety checks.\n");
	_eina_error_init_count = 0;
	return 0;
     }
   return 1;
}

/**
 * @brief Shut down the error module.
 *
 * @return 0 when the error module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the error module set up by
 * eina_error_init(). It is called by eina_shutdown() and by all
 * modules shutdown functions. It returns 0 when it is called the
 * same number of times than eina_error_init(). In that case it clears
 * the error list.
 *
 * @see eina_shutdown()
 */
EAPI int eina_error_shutdown(void)
{
	Eina_Inlist *tmp;

	_eina_error_init_count--;
	if (!_eina_error_init_count)
	{
		/* remove the error strings */
		while (_error_list)
		{
			tmp = _error_list;

			_error_list = _error_list->next;
			free(tmp);
		}
		_error_list_count = 0;
		eina_safety_checks_shutdown();
	}
	return _eina_error_init_count;
}

/**
 * @brief Register a new error type.
 *
 * @param msg The description of the error.
 * @return The unique number identifier for this error.
 *
 * This function stores in a list the error message described by
 * @p msg. The returned value is a unique identifier greater or equal
 * than 1. The description can be retrieve later by passing to
 * eina_error_msg_get() the returned value.
 */
EAPI Eina_Error eina_error_msg_register(const char *msg)
{
	Eina_Inlist *tmp;
	size_t length;

	EINA_SAFETY_ON_NULL_RETURN_VAL(msg, 0);

	length = strlen(msg) + 1;

	tmp = malloc(sizeof (Eina_Inlist) + length);
	if (!tmp) return 0;

	memcpy((char*)(tmp + 1), msg, length);

	_error_list = eina_inlist_append(_error_list, tmp);

	return ++_error_list_count;
}

/**
 * @brief Return the description of the given an error number.
 *
 * @param error The error number.
 * @return The description of the error.
 *
 * This function returns the description of an error that has been
 * registered with eina_error_msg_register(). If an incorrect error is
 * given, then @c NULL is returned.
 */
EAPI const char * eina_error_msg_get(Eina_Error error)
{
	Eina_Inlist *tmp;
	int i;

	for (i = 0, tmp = _error_list; i < error - 1; ++i, tmp = tmp->next)
		;
	return (char*) (tmp + 1);
}

/**
 * @brief Return the last set error.
 *
 * @return The last error.
 *
 * This function returns the last error set by eina_error_set(). The
 * description of the message is returned by eina_error_msg_get().
 */
EAPI Eina_Error eina_error_get(void)
{
	return _err;
}

/**
 * @brief Set the last error.
 *
 * @param err The error identifier.
 *
 * This function sets the last error identifier. The last error can be
 * retrieved with eina_error_get().
 */
EAPI void eina_error_set(Eina_Error err)
{
	_err = err;
}

/**
 * @deprecated use eina_log_print() instead.
 */
EAPI void eina_error_print(int level, const char *file,
		const char *fnc, int line, const char *fmt, ...)
{
	va_list args;

	EINA_SAFETY_ON_NULL_RETURN(file);
	EINA_SAFETY_ON_NULL_RETURN(fnc);
	EINA_SAFETY_ON_NULL_RETURN(fmt);

	EINA_LOG_WARN("this function is deprecated!");

	va_start(args, fmt);
	eina_log_vprint(EINA_LOG_DOMAIN_GLOBAL, level, file, fnc, line, fmt, args);
	va_end(args);
}

/**
 * @deprecated use eina_log_vprint() instead.
 */
EAPI void eina_error_vprint(int level, const char *file,
		const char *fnc, int line, const char *fmt, va_list args)
{
	EINA_SAFETY_ON_NULL_RETURN(file);
	EINA_SAFETY_ON_NULL_RETURN(fnc);
	EINA_SAFETY_ON_NULL_RETURN(fmt);

	EINA_LOG_WARN("this function is deprecated!");

	eina_log_vprint(EINA_LOG_DOMAIN_GLOBAL, level, file, fnc, line, fmt, args);
}

/**
 * @deprecated use eina_log_level_set() instead.
 */
EAPI void eina_error_log_level_set(int level)
{
   EINA_LOG_WARN("this function is deprecated!");
   eina_log_level_set(level);
}

/**
 * @}
 */
