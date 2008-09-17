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
 * itself. It provides facilities for displaying different kind of
 * messages (warning, informations, errors, or debug), which can be
 * tuned by the user, or for registering new errors messages.
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
 * Now that the error module is initialized, error messages can
 * be displayed. Helper macros are already defined for such purpose:
 *
 * @li EINA_ERROR_PERR(),
 * @li EINA_ERROR_PINFO(),
 * @li EINA_ERROR_PWARN(),
 * @li EINA_ERROR_PDBG().
 *
 * Here is an example:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_error.h>
 *
 * void test(int i)
 * {
 *    EINA_ERROR_PDBG("Entering test\n");
 *
 *    if (i < 0)
 *    {
 *        EINA_ERROR_PERR("argument is negative\n");
 *        return;
 *    }
 *
 *    EINA_ERROR_PINFO("argument non negative\n");
 *
 *    EINA_ERROR_PDBG("Exiting test\n");
 * }
 *
 * int main(void)
 * {
 *    if (!eina_error_init())
 *    {
 *        printf ("Error during the initialization of eina_error module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    test(-1);
 *    test(0);
 *
 *    eina_error_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * If you compiled Eina without debug mode, after executing that
 * program, you will see only 1 message (the argument being
 * negative). Why ? These macros are just wrappers around
 * eina_error_print(). This function only dysplays messages if the
 * current error level is lesser or equal than the one used by
 * eina_error_print(). By default, the current error level is
 * #EINA_ERROR_LEVEL_ERR (in non debug mode), and the macro uses
 * the error values defines by #Eina_Error_Level. So as
 * #EINA_ERROR_LEVEL_ERR is the smallest value, only
 * EINA_ERROR_PERR() will display the message.
 *
 * To modify this behavior, you have two solutions:
 *
 * @li Using the environment variable EINA_ERROR_LEVEL. In that case,
 * just set the environment variable to a integer number between 0
 * (#EINA_ERROR_LEVEL_ERR) and 3 (#EINA_ERROR_LEVEL_DBG)
 * before executing the program. That environment variable will be
 * read during the first call of eina_error_init().
 * @li Using the function eina_error_log_level_set().
 *
 * So try using the environment variable like this:
 *
 * @code
 * EINA_ERROR_LEVEL=2 ./my_app
 * @endcode
 *
 * To do the same with eina_error_log_level_set(), just add
 *
 * @code
 * eina_error_log_level_set(EINA_ERROR_LEVEL_INFO);
 * @endcode
 *
 * before the calls of the tests in the above example.
 *
 * @section tutorial_error_advanced_display Advanced usage of print callbacks
 *
 * The error module allows the user to change the way
 * eina_error_print() displays the messages. It suffices to pass to
 * eina_error_print_cb_set() the function used to display the
 * message. That  function must be of type #Eina_Error_Print_Cb. As a
 * custom data can be passed to that callback, powerful display
 * messages can be displayed.
 *
 * It is suggested to not use __FILE__, __FUNCTION__ or __LINE__ when
 * writing that callback, but when defining macros (like
 * EINA_ERROR_PERR() and other macros).
 *
 * Here is an example of custom callback, whose behavior can be
 * changed at runtime:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_error.h>
 *
 * #define ERROR(fmt, ...)                                    \
 *    eina_error_print(EINA_ERROR_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
 *
 * typedef struct _Data Data;
 *
 * struct _Data
 * {
 *    int to_stderr;
 * };
 *
 * void print_cb(Eina_Error_Level level,
 *               const char *file,
 *               const char *fnc,
 *               int line,
 *               const char *fmt,
 *               void *data,
 *               va_list args)
 * {
 *    Data *d;
 *    FILE *output;
 *    char *str;
 *
 *    d = (Data *)data;
 *    if (d->to_stderr)
 *    {
 *       output = stderr;
 *       str = "stderr";
 *    }
 *    else
 *    {
 *       output = stdout;
 *       str = "stdout";
 *    }
 *
 *    fprintf(output, "%s:%s (%d) %s: ", file, fnc, line, str);
 *    vfprintf(output, fmt, args);
 * }
 *
 * void test(Data *data, int i)
 * {
 *    if (i < 0)
 *       data->to_stderr = 0;
 *    else
 *       data->to_stderr = 1;
 *
 *    ERROR("error message...\n");
 * }
 *
 * int main(void)
 * {
 *    Data *data;
 *
 *    if (!eina_error_init())
 *    {
 *       printf ("Error during the initialization of eina_error module\n");
 *       return EXIT_FAILURE;
 *    }
 *
 *    data = (Data *)malloc(sizeof(Data));
 *    if (!data)
 *    {
 *       printf ("Error during memory allocation\n");
 *       eina_error_shutdown();
 *       return EXIT_FAILURE;
 *    }
 *
 *    eina_error_print_cb_set(print_cb, data);
 *
 *    test(data, -1);
 *    test(data, 0);
 *
 *    eina_error_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
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
 * Of course, instead of printf(), eina_error_print() can be used to
 * have beautiful error messages.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "eina_error.h"
#include "eina_list.h"
#include "eina_private.h"

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

static int _init_count = 0;
static Eina_List *_error_list;
static Eina_Error _err;
static Eina_Error_Print_Cb _print_cb = eina_error_print_cb_stdout;
static void *_print_cb_data = NULL;

#define RED     "\033[31;1m"
#define GREEN   "\033[32;1m"
#define YELLOW  "\033[33;1m"
#define WHITE   "\033[37;1m"
#define NOTHING "\033[0m"

#ifdef DEBUG
static Eina_Error_Level _error_level = EINA_ERROR_LEVEL_DBG;
#else
static Eina_Error_Level _error_level = EINA_ERROR_LEVEL_ERR;
#endif

static char *_colors[EINA_ERROR_LEVELS] = {
	[EINA_ERROR_LEVEL_ERR] = RED,
	[EINA_ERROR_LEVEL_WARN] = YELLOW,
	[EINA_ERROR_LEVEL_INFO] = NOTHING,
	[EINA_ERROR_LEVEL_DBG] = GREEN,
};

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
 * @addtogroup Eina_Tools_Group Tools Modules
 *
 * @{
 */

/**
 * @addtogroup Eina_Error_Group Error Functions
 *
 * @brief These functions provide error management for projects.
 *
 * The error system must be initialized with eina_error_init() and
 * shut down with eina_error_shutdown(). The most generic way to print
 * errors is to use eina_error_print() but the helper macros
 * EINA_ERROR_PERR(), EINA_ERROR_PINFO(), EINA_ERROR_PWARN() and
 * EINA_ERROR_PDBG() should be used instead.
 *
 * Here is a straightforward example:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_error.h>
 *
 * void test_warn(void)
 * {
 *    EINA_ERROR_PWARN("Here is a warning message\n");
 * }
 *
 * int main(void)
 * {
 *    if (!eina_error_init())
 *    {
 *        printf ("Error during the initialization of eina_error module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    test_warn();
 *
 *    eina_error_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following commant:
 *
 * @code
 * gcc -Wall -o test_eina_error test_eina.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 * If Eina is compiled without debug mode, then executing the
 * resulting program displays nothing because the default error level
 * is #EINA_ERROR_LEVEL_ERR and we want to display a warning
 * message, which level is strictly greater than the error level (see
 * eina_error_print() for more informations). Now execute the program
 * with:
 *
 * @code
 * EINA_ERROR_LEVEL=2 ./test_eina_error
 * @endcode
 *
 * You should see a message displayed in the terminal.
 *
 * For more information, you can look at the @ref tutorial_error_page.
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
 * @brief Initialize the error system.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the error system or Eina. It is called by
 * eina_init() and by all subsystems initialization functions. It
 * returns 0 on failure, otherwise it returns the number of times it
 * is called.
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
 * If you call explicitely this function and once the error subsystem
 * is not used anymore, then eina_error_shutdown() must be called to
 * shut down the error system.
 */
EAPI int eina_error_init(void)
{
	if (!_init_count)
	{
		char *level;
		/* TODO register the eina's basic errors */
		/* TODO load the environment variable for getting the log level */
		if ((level = getenv("EINA_ERROR_LEVEL")))
		{
			_error_level = atoi(level);
		}
		EINA_ERROR_OUT_OF_MEMORY = eina_error_msg_register("Out of memory");
	}
	/* get all the modules */
	return ++_init_count;
}

/**
 * @brief Shut down the error system.
 *
 * @return 0 when the error system is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shut down the error system set up by
 * eina_error_init(). It is called by eina_shutdown() and by all
 * subsystems shutdown functions. It returns 0 when it is called the
 * same number of times than eina_error_init() and it clears the error
 * list.
 */
EAPI int eina_error_shutdown(void)
{
	_init_count--;
	if (!_init_count)
	{
		/* remove the error strings */
		while (_error_list)
		{
			free(eina_list_data(_error_list));
			_error_list = eina_list_free(_error_list);
		}
	}
	return _init_count;
}

/**
 * @brief Register a new error type.
 *
 * @param msg The description of the error.
 * @return The unique number identifier for this error.
 *
 * This function stores in a list the error message described by
 * @p msg. The returned value is a unique identifier greater or equal
 * than 1. The description can be retrive later by passing to
 * eina_error_msg_get() the returned value.
 */
EAPI Eina_Error eina_error_msg_register(const char *msg)
{
	_error_list = eina_list_append(_error_list, strdup(msg));

	return eina_list_count(_error_list);
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
	return eina_list_nth(_error_list, error - 1);
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
 * @brief Print the error to a file.
 *
 * @param level The error level.
 * @param file The name of the file where the error occurred.
 * @param fnc The name of the function where the error occurred.
 * @param line The number of the line where the error occurred.
 * @param fmt The format to use.
 *
 * This function sends to a stream (like stdout or stderr) a formatted
 * string that describes the error. The error level is set by
 * @p level, the name of the file, of the function and the number of the
 * line where the error occurred are respectively set by @p file,
 * @p fnc and @p line. A description of the error message is given by
 * @p fmt, which is a formatted string, followed by optional arguments
 * that can be converted (like with printf). If @p level is strictly
 * larger than the current error level, that function returns
 * immediatly, otherwise it prints all the errors up to the current
 * error level. The current error level can be changed with
 * eina_error_log_level_set(). See also eina_error_init() for more
 * informations.
 *
 * By default, that formatted message is send to stdout and is
 * formatted by eina_error_print_cb_stdout(). The destination of the
 * formatted message is send and the way it is formatted can be
 * changed by setting a print callback with
 * eina_error_print_cb_set(). Some print callbacks are already
 * defined: eina_error_print_cb_stdout() that send the message to
 * stdout and eina_error_print_cb_file() that sends it to a file, but
 * custom print callbacks can be used. They must be of type
 * #Eina_Error_Print_Cb.
 */
EAPI void eina_error_print(Eina_Error_Level level, const char *file,
		const char *fnc, int line, const char *fmt, ...)
{
	va_list args;

	if (level > _error_level)
		return;

	va_start(args, fmt);
	_print_cb(level, file, fnc, line, fmt, _print_cb_data, args);
	va_end(args);
}

/**
 * @brief Print callback that sends the error message to stdout.
 *
 * @param level The error level.
 * @param file The name of the file where the error occurred.
 * @param fnc The name of the function where the error occurred.
 * @param line The number of the line where the error occurred.
 * @param fmt The format to use.
 * @param data Unused.
 * @param args The arguments that will be converted.
 *
 * This function is used to send a formatted error message to standard
 * output and is used as a print callback, with
 * eina_error_print(). This is the default print callback.
 */
EAPI void eina_error_print_cb_stdout(Eina_Error_Level level, const char *file,
		const char *fnc, int line, const char *fmt, __UNUSED__ void *data,
		va_list args)
{
	printf("%s", _colors[level]);
	printf("[%s:%d] %s() ", file, line, fnc);
	printf("%s", _colors[EINA_ERROR_LEVEL_INFO]);
	vprintf(fmt, args);
}

/**
 * @brief Print callback that sends the error message to a specified stream.
 *
 * @param level Unused.
 * @param file The name of the file where the error occurred.
 * @param fnc The name of the function where the error occurred.
 * @param line The number of the line where the error occurred.
 * @param fmt The format to use.
 * @param data The file stream.
 * @param args The arguments that will be converted.
 *
 * This function is used to send a formatted error message to the
 * stream specified by @p data. That stream must be of type FILE *. Look
 * at eina_error_print_cb_stdout() for the description of the other
 * parameters. Use eina_error_print_cb_set() to set it as default
 * print callback in eina_error_print().
 */
EAPI void eina_error_print_cb_file(__UNUSED__ Eina_Error_Level level, const char *file,
		const char *fnc, int line, const char *fmt, void *data,
		va_list args)
{
	FILE *f = data;

	fprintf(f, "[%s:%d] %s() ", file, line, fnc);
	vfprintf(f, fmt, args);
}

/**
 * @brief Set the default print callback.
 *
 * @param cb The print callback.
 * @param data The data to pass to the callback
 *
 * This function sets the default print callback @p cb used by
 * eina_error_print(). A data can be passed to that callback with
 * @p data.
 */
EAPI void eina_error_print_cb_set(Eina_Error_Print_Cb cb, void *data)
{
	_print_cb = cb;
	_print_cb_data = data;
}

/**
 * @brief Set the default error log level.
 *
 * @param level The error level.
 *
 * This function sets the error log level @p level. It is used in
 * eina_error_print().
 */
EAPI void eina_error_log_level_set(Eina_Error_Level level)
{
	_error_level = level;
}

/**
 * @}
 */

/**
 * @}
 */
