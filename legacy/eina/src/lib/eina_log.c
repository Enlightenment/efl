/* EINA - EFL data type library
 * Copyright (C) 2007-2009 Jorge Luis Zapata Muga, Cedric Bail, Andre Dieb
 * Martins
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
 * @page tutorial_log_page log Tutorial
 *
 * @section tutorial_log_introduction Introduction
 *
 * The Eina Log module provides logging facilities for libraries and
 * applications. It provides colored logging, basic logging levels (error,
 * warning, debug, info, critical) and loggers - called logging domains -
 * which will be covered on next sections.
 *
 * @section tutorial_log_basic_usage Basic Usage
 *
 * Log messages can be displayed using the following macros:
 *
 * @li EINA_LOG_ERR(),
 * @li EINA_LOG_INFO(),
 * @li EINA_LOG_WARN(),
 * @li EINA_LOG_DBG().
 *
 * Here is an example:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <Eina.h>
 *
 * void test(int i)
 * {
 *    EINA_LOG_DBG("Entering test");
 *
 *    if (i < 0)
 *    {
 *        EINA_LOG_ERR("Argument is negative");
 *        return;
 *    }
 *
 *    EINA_LOG_INFO("argument non negative");
 *
 *    EINA_LOG_DBG("Exiting test");
 * }
 *
 * int main(void)
 * {
 *    if (!eina_init())
 *    {
 *        printf("log during the initialization of Eina_Log module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    test(-1);
 *    test(0);
 *
 *    eina_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * If you compiled Eina without debug mode, execution will yield only one log
 * message, which is "argument is negative".
 *
 * Here we introduce the concept of logging domains (or loggers), which might
 * already be familiar to readers. It is basically a way to separate a set of
 * log messages into a context (e.g. a module) and provide a way of controlling
 * this set as a whole.
 *
 * For example, suppose you have 3 different modules on your application and you
 * want to get logging only from one of them (e.g. create some sort of filter).
 * For achieving that, all you need to do is create a logging domain for each
 * module so that all logging inside a module can be considered as a whole.
 *
 * Logging domains are specified by a name, color applied to the name and the
 * level. The first two (name and color) are set through code, that is, inside
 * your application/module/library.
 *
 * The level is used for controlling which messages should appear. It
 * specifies the lowest level that should be displayed (e.g. a message
 * with level 11 being logged on a domain with level set to 10 would be
 * displayed, while a message with level 9 wouldn't).
 *
 * The domain level is set during runtime (in contrast with the name and
 * color) through the environment variable EINA_LOG_LEVELS. This variable
 * expects a list in the form domain_name1:level1,domain_name2:level2,... . For
 * example:
 *
 * @code
 *
 * EINA_LOG_LEVELS=mymodule1:5,mymodule2:2,mymodule3:0 ./myapp
 *
 * @encode
 *
 * This line would set mymodule1 level to 5, mymodule2 level to 2 and mymodule3
 * level to 0.
 *
 *
 * There's also a global logger to which EINA_LOG_(ERR, DBG, INFO, CRIT, WARN)
 * macros do log on. It is a logger that is created internally by Eina Log with
 * an empty name and can be used for general logging (where logging domains do
 * not apply).
 *
 * Since this global logger doesn't have a name, you can't set its level through
 * EINA_LOG_LEVELS variable. Here we introduce a second environment variable
 * that is a bit more special: EINA_LOG_LEVEL.
 *
 * This variable specifies the level of the global logging domain and the level
 * of domains that haven't been set through EINA_LOG_LEVELS. Here's an example:
 *
 * @code
 *
 * EINA_LOG_LEVEL=3 EINA_LOG_LEVELS=module1:10,module3:2 ./myapp
 *
 * @endcode
 *
 * Supposing you have modules named "module1", "module2" and "module3", this
 * line would result in module1 with level 10, module2 with level 3 and module3
 * with level 2. Note that module2's level wasn't specified, so it's level is
 * set to the global level. This way we can easily apply filters to multiple
 * domains with only one parameter (EINA_LOG_LEVEL=num).
 *
 * The global level (EINA_LOG_LEVEL) can also be set through code, using
 * eina_log_level_set() function.
 *
 *
 * While developing your libraries or applications, you may notice that 
 * EINA_LOG_DOM_(ERR, DBG, INFO, CRIT, WARN) macros also print out 
 * messages from eina itself. Here we introduce another environment variable
 * that is a bit more special: EINA_LOG_LEVELS_GLOB.
 *
 * This variable allows you to disable the logging of any/all code in eina itself.
 * This is useful when developing your libraries or applications so that you can 
 * see your own domain's messages easier without having to sift through a lot of 
 * internal eina debug messages. Here's an example:
 *
 * @code
 *
 * EINA_LOG_LEVEL=3 EINA_LOG_LEVELS_GLOB=eina_*:0 ./myapp
 *
 * @endcode
 *
 * This will disable eina_log output from all internal eina code thus allowing 
 * you to see your own domain messages easier.
 * 
 * @section tutorial_log_advanced_display Advanced usage of print callbacks
 *
 * The log module allows the user to change the way
 * eina_log_print() displays the messages. It suffices to pass to
 * eina_log_print_cb_set() the function used to display the
 * message. That  function must be of type #Eina_Log_Print_Cb. As a
 * custom data can be passed to that callback, powerful display
 * messages can be displayed.
 *
 * It is suggested to not use __FILE__, __FUNCTION__ or __LINE__ when
 * writing that callback, but when defining macros (like
 * EINA_LOG_ERR() and other macros).
 *
 * Here is an example of custom callback, whose behavior can be
 * changed at runtime:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_log.h>
 *
 * #define log(fmt, ...)                                    \
 *    eina_log_print(EINA_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
 *
 * typedef struct _Data Data;
 *
 * struct _Data
 * {
 *    int to_stderr;
 * };
 *
 * void print_cb(const Eina_Log_Domain *domain,
 *               Eina_Log_Level level,
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
 *    fprintf(output, "%s:%s:%s (%d) %s: ",
 *            domain->domain_str, file, fnc, line, str);
 *    vfprintf(output, fmt, args);
 *    putc('\n', output);
 * }
 *
 * void test(Data *data, int i)
 * {
 *    if (i < 0)
 *       data->to_stderr = 0;
 *    else
 *       data->to_stderr = 1;
 *
 *    log("log message...");
 * }
 *
 * int main(void)
 * {
 *    Data data;
 *
 *    if (!eina_init())
 *    {
 *       printf("log during the initialization of Eina_Log module\n");
 *       return EXIT_FAILURE;
 *    }
 *
 *    eina_log_print_cb_set(print_cb, &data);
 *
 *    test(&data, -1);
 *    test(&data, 0);
 *
 *    eina_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * @addtogroup Eina_Log_Group Log
 *
 * @{
 *
 * The default log level value is set by default to
 * #EINA_LOG_LEVEL_DBG if Eina is compiled with debug mode, or to
 * #EINA_LOG_LEVEL_ERR otherwise. That value can be overwritten by
 * setting the environment variable EINA_LOG_LEVEL. This function
 * checks the value of that environment variable in the first
 * call. Its value must be a number between 0 and 4, to match the log
 * levels #EINA_LOG_LEVEL_CRITICAL, #EINA_LOG_LEVEL_ERR,
 * #EINA_LOG_LEVEL_WARN, #EINA_LOG_LEVEL_INFO and
 * #EINA_LOG_LEVEL_DBG. That value can also be set later with
 * eina_log_log_level_set(). When logging domains are created, they
 * will get either this value or specific value given with
 * EINA_LOG_LEVELS that takes the format
 * 'domain_name:level,another_name:other_level'.
 *
 * Format and verbosity of messages depend on the logging method, see
 * eina_log_print_cb_set(). The default logging method is
 * eina_log_print_cb_stderr(), which will output fancy colored
 * messages to standard error stream. See its documentation on how to
 * disable coloring, function or file/line print.
 *
 * This module will optionally abort program execution if message
 * level is below or equal to @c EINA_LOG_LEVEL_CRITICAL and
 * @c EINA_LOG_ABORT=1.
 *
 * @}
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_inlist.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_log.h"

#include <assert.h>

/* TODO
 * + printing logs to stdout or stderr can be implemented
 * using a queue, useful for multiple threads printing
 * + add a wrapper for assert?
 */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_LOG_ENV_ABORT "EINA_LOG_ABORT"
#define EINA_LOG_ENV_ABORT_LEVEL "EINA_LOG_ABORT_LEVEL"
#define EINA_LOG_ENV_LEVEL "EINA_LOG_LEVEL"
#define EINA_LOG_ENV_LEVELS "EINA_LOG_LEVELS"
#define EINA_LOG_ENV_LEVELS_GLOB "EINA_LOG_LEVELS_GLOB"
#define EINA_LOG_ENV_COLOR_DISABLE "EINA_LOG_COLOR_DISABLE"
#define EINA_LOG_ENV_FILE_DISABLE "EINA_LOG_FILE_DISABLE"
#define EINA_LOG_ENV_FUNCTION_DISABLE "EINA_LOG_FUNCTION_DISABLE"


// Structure for storing domain level settings passed from the command line
// that will be matched with application-defined domains.
typedef struct _Eina_Log_Domain_Level_Pending Eina_Log_Domain_Level_Pending;
struct _Eina_Log_Domain_Level_Pending
{
   EINA_INLIST;
   unsigned int level;
   char name[];
};

/*
 * List of levels for domains set by the user before the domains are registered,
 * updates the domain levels on the first log and clears itself.
 */
static Eina_Inlist *_pending_list = NULL;
static Eina_Inlist *_glob_list = NULL;

// Disable color flag (can be changed through the env var
// EINA_LOG_ENV_COLOR_DISABLE).
static Eina_Bool _disable_color = EINA_FALSE;
static Eina_Bool _disable_file = EINA_FALSE;
static Eina_Bool _disable_function = EINA_FALSE;
static Eina_Bool _abort_on_critical = EINA_FALSE;
static int _abort_level_on_critical = EINA_LOG_LEVEL_CRITICAL;

#ifdef EFL_HAVE_PTHREAD
#include <pthread.h>
static Eina_Bool _threads_enabled = EINA_FALSE;
static pthread_t _main_thread;

#define IS_MAIN(t)  pthread_equal(t, _main_thread)
#define IS_OTHER(t) EINA_UNLIKELY(!IS_MAIN(t))
#define CHECK_MAIN(...)							\
  do {									\
     if (!IS_MAIN(pthread_self())) {					\
	fprintf(stderr,							\
		"ERR: not main thread! current=%lu, main=%lu\n",	\
		pthread_self(), _main_thread);				\
	return __VA_ARGS__;						\
     }									\
  } while (0)

#ifdef EFL_HAVE_PTHREAD_SPINLOCK
static pthread_spinlock_t _log_lock;
#define LOG_LOCK()								\
  if(_threads_enabled) \
  do {									\
     if (0)								\
       fprintf(stderr, "+++LOG LOG_LOCKED!   [%s, %lu]\n",			\
	       __FUNCTION__, pthread_self());				\
     if (EINA_UNLIKELY(_threads_enabled))				\
       pthread_spin_lock(&_log_lock);					\
  } while (0)
#define LOG_UNLOCK()							\
  if(_threads_enabled) \
  do {									\
     if (EINA_UNLIKELY(_threads_enabled))				\
       pthread_spin_unlock(&_log_lock);					\
     if (0)								\
       fprintf(stderr,							\
	       "---LOG LOG_UNLOCKED! [%s, %lu]\n",				\
	       __FUNCTION__, pthread_self());				\
  } while (0)
#define INIT() pthread_spin_init(&_log_lock, PTHREAD_PROCESS_PRIVATE);
#define SHUTDOWN() pthread_spin_destroy(&_log_lock);
#else
static pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOG_LOCK() if(_threads_enabled) pthread_mutex_lock(&_log_mutex);
#define LOG_UNLOCK() if(_threads_enabled) pthread_mutex_unlock(&_log_mutex);
#define INIT() do {} while (0)
#define SHUTDOWN() do {} while (0)
#endif
#else
#define LOG_LOCK() do {} while (0)
#define LOG_UNLOCK() do {} while (0)
#define IS_MAIN(t)  (1)
#define IS_OTHER(t) (0)
#define CHECK_MAIN(...) do {} while (0)
#define INIT() do {} while (0)
#define SHUTDOWN() do {} while (0)
#endif


// List of domains registered
static Eina_Log_Domain *_log_domains = NULL;
static int _log_domains_count = 0;
static int _log_domains_allocated = 0;

// Default function for printing on domains
static Eina_Log_Print_Cb _print_cb = eina_log_print_cb_stderr;
static void *_print_cb_data = NULL;

#ifdef DEBUG
static Eina_Log_Level _log_level = EINA_LOG_LEVEL_DBG;
#elif DEBUG_CRITICAL
static Eina_Log_Level _log_level = EINA_LOG_LEVEL_CRITICAL;
#else
static Eina_Log_Level _log_level = EINA_LOG_LEVEL_ERR;
#endif

// Default colors and levels
static const char *_colors[] = { // + 1 for higher than debug
  EINA_COLOR_LIGHTRED, // EINA_LOG_LEVEL_CRITICAL
  EINA_COLOR_RED, // EINA_LOG_LEVEL_ERR
  EINA_COLOR_YELLOW, // EINA_LOG_LEVEL_WARN
  EINA_COLOR_GREEN, // EINA_LOG_LEVEL_INFO
  EINA_COLOR_LIGHTBLUE, // EINA_LOG_LEVEL_DBG
  EINA_COLOR_BLUE, // Higher than DEBUG
};

static const char *_names[] = {
  "CRI",
  "ERR",
  "WRN",
  "INF",
  "DBG",
};

static inline void
eina_log_print_level_name_get(int level, const char **p_name)
{
   static char buf[4];
   if (EINA_UNLIKELY(level < 0))
     {
	snprintf(buf, sizeof(buf), "%03d", level);
	*p_name = buf;
     }
   else if (EINA_UNLIKELY(level > EINA_LOG_LEVELS))
     {
	snprintf(buf, sizeof(buf), "%03d", level);
	*p_name = buf;
     }
   else
	*p_name = _names[level];
}

static inline void
eina_log_print_level_name_color_get(int level, const char **p_name, const char **p_color)
{
   static char buf[4];
   if (EINA_UNLIKELY(level < 0))
     {
	snprintf(buf, sizeof(buf), "%03d", level);
	*p_name = buf;
	*p_color = _colors[0];
     }
   else if (EINA_UNLIKELY(level > EINA_LOG_LEVELS))
     {
	snprintf(buf, sizeof(buf), "%03d", level);
	*p_name = buf;
	*p_color = _colors[EINA_LOG_LEVELS];
     }
   else
     {
	*p_name = _names[level];
	*p_color = _colors[level];
     }
}

#define DECLARE_LEVEL_NAME(level) const char *name; eina_log_print_level_name_get(level, &name)
#define DECLARE_LEVEL_NAME_COLOR(level) const char *name, *color; eina_log_print_level_name_color_get(level, &name, &color)

/** No threads, No color */
static void
eina_log_print_prefix_NOthreads_NOcolor_file_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s:%s %s:%d %s() ", name, d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_NOthreads_NOcolor_NOfile_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file __UNUSED__, const char *fnc, int line __UNUSED__)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s:%s %s() ", name, d->domain_str, fnc);
}

static void
eina_log_print_prefix_NOthreads_NOcolor_file_NOfunc(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc __UNUSED__, int line)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s:%s %s:%d ", name, d->domain_str, file, line);
}

/* No threads, color */
static void
eina_log_print_prefix_NOthreads_color_file_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s %s:%d "
	   EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
	   color, name, d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_NOthreads_color_NOfile_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file __UNUSED__, const char *fnc, int line __UNUSED__)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s "
	   EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
	   color, name, d->domain_str, fnc);
}

static void
eina_log_print_prefix_NOthreads_color_file_NOfunc(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc __UNUSED__, int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s %s:%d ",
	   color, name, d->domain_str, file, line);
}

/** threads, No color */
#ifdef EFL_HAVE_PTHREAD
static void
eina_log_print_prefix_threads_NOcolor_file_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line)
{
   DECLARE_LEVEL_NAME(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s:%s[T:%lu] %s:%d %s() ",
		name, d->domain_str, cur, file, line, fnc);
	return;
     }
   fprintf(fp, "%s:%s %s:%d %s() ", name, d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_threads_NOcolor_NOfile_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file __UNUSED__, const char *fnc, int line __UNUSED__)
{
   DECLARE_LEVEL_NAME(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s:%s[T:%lu] %s() ",
		name, d->domain_str, cur, fnc);
	return;
     }
   fprintf(fp, "%s:%s %s() ", name, d->domain_str, fnc);
}

static void
eina_log_print_prefix_threads_NOcolor_file_NOfunc(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc __UNUSED__, int line)
{
   DECLARE_LEVEL_NAME(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s:%s[T:%lu] %s:%d ",
		name, d->domain_str, cur, file, line);
	return;
     }
   fprintf(fp, "%s:%s %s:%d ", name, d->domain_str, file, line);
}

/* threads, color */
static void
eina_log_print_prefix_threads_color_file_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s[T:"
		EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] %s:%d "
		EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
		color, name, d->domain_str, cur, file, line, fnc);
	return;
     }
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s %s:%d "
	   EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
	   color, name, d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_threads_color_NOfile_func(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file __UNUSED__, const char *fnc, int line __UNUSED__)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s[T:"
		EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] "
		EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
		color, name, d->domain_str, cur, fnc);
	return;
     }
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s "
	   EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
	   color, name, d->domain_str, fnc);
}

static void
eina_log_print_prefix_threads_color_file_NOfunc(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc __UNUSED__, int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
   pthread_t cur = pthread_self();
   if (IS_OTHER(cur))
     {
	fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s[T:"
		EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] %s:%d ",
		color, name, d->domain_str, cur, file, line);
	return;
     }
   fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s %s:%d ",
	   color, name, d->domain_str, file, line);
}
#endif

static void (*_eina_log_print_prefix)(FILE *fp, const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line) = eina_log_print_prefix_NOthreads_color_file_func;

static inline void
eina_log_print_prefix_update(void)
{
   if (_disable_file && _disable_function)
     {
	fprintf(stderr, "ERROR: cannot have " EINA_LOG_ENV_FILE_DISABLE " and "
		EINA_LOG_ENV_FUNCTION_DISABLE " set at the same time, will "
		"just disable function.\n");
	_disable_file = 0;
     }

#define S(NOthread, NOcolor, NOfile, NOfunc) \
   _eina_log_print_prefix = eina_log_print_prefix_##NOthread##threads_##NOcolor##color_##NOfile##file_##NOfunc##func

#ifdef EFL_HAVE_PTHREAD
   if (_threads_enabled)
     {
	if (_disable_color)
	  {
	     if (_disable_file)
	       S(,NO,NO,);
	     else if (_disable_function)
	       S(,NO,,NO);
	     else
	       S(,NO,,);
	  }
	else
	  {
	     if (_disable_file)
	       S(,,NO,);
	     else if (_disable_function)
	       S(,,,NO);
	     else
	       S(,,,);
	  }
	return;
     }
#endif

   if (_disable_color)
     {
	if (_disable_file)
	  S(NO,NO,NO,);
	else if (_disable_function)
	  S(NO,NO,,NO);
	else
	  S(NO,NO,,);
     }
   else
     {
	if (_disable_file)
	  S(NO,,NO,);
	else if (_disable_function)
	  S(NO,,,NO);
	else
	  S(NO,,,);
     }
#undef S
}

/*
 * Creates a colored domain name string.
 */
static const char *
eina_log_domain_str_get(const char *name, const char *color)
{
   const char *d;

   if (color)
     {
	size_t name_len;
	size_t color_len;

	name_len = strlen(name);
	color_len = strlen(color);
	d = malloc(sizeof(char) * (color_len + name_len + strlen(EINA_COLOR_RESET) + 1));
	if (!d) return NULL;
	memcpy((char *)d, color, color_len);
	memcpy((char *)(d + color_len), name, name_len);
	memcpy((char *)(d + color_len + name_len), EINA_COLOR_RESET, strlen(EINA_COLOR_RESET));
	((char *)d)[color_len + name_len + strlen(EINA_COLOR_RESET)] = '\0';
     }
   else
	d = strdup(name);

   return d;
}

/*
 * Setups a new logging domain to the name and color specified. Note that this
 * constructor acts upon an pre-allocated object.
 */
static Eina_Log_Domain *
eina_log_domain_new(Eina_Log_Domain *d, const char *name, const char *color)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   d->level = EINA_LOG_LEVEL_UNKNOWN;
   d->deleted = EINA_FALSE;

   if (name)
     {
	if ((color) && (!_disable_color))
	   d->domain_str = eina_log_domain_str_get(name, color);
	else
	   d->domain_str = eina_log_domain_str_get(name, NULL);

	d->name = strdup(name);
     }
   else
     {
	d->domain_str = NULL;
	d->name = NULL;
     }

   return d;
}

/*
 * Frees internal strings of a log domain, keeping the log domain itself as a
 * slot for next domain registers.
 */
static void
eina_log_domain_free(Eina_Log_Domain *d)
{
   EINA_SAFETY_ON_NULL_RETURN(d);

   if (d->domain_str)
      free((char *)d->domain_str);
   if (d->name)
      free((char *)d->name);
}

/*
 * Parses domain levels passed through the env var.
 */
static void
eina_log_domain_parse_pendings(void)
{
   const char *start;

   if (!(start = getenv(EINA_LOG_ENV_LEVELS))) return;

   // name1:level1,name2:level2,name3:level3,...
   while (1)
     {
	Eina_Log_Domain_Level_Pending *p;
	char *end = NULL;
	char *tmp = NULL;
	long int level;

	end = strchr(start, ':');
	if (!end) break;

	// Parse level, keep going if failed
	level = strtol((char *)(end + 1), &tmp, 10);
	if (tmp == (end + 1)) goto parse_end;

	// Parse name
	p = malloc(sizeof(Eina_Log_Domain_Level_Pending) + end - start + 1);
	if (!p) break;
	memcpy((char *)p->name, start, end - start);
	((char *)p->name)[end - start] = '\0';
	p->level = level;

	_pending_list = eina_inlist_append(_pending_list, EINA_INLIST_GET(p));

	parse_end:
	start = strchr(tmp, ',');
	if (start) start++;
	else break;
     }
}

static void
eina_log_domain_parse_pending_globs(void)
{
   const char *start;

   if (!(start = getenv(EINA_LOG_ENV_LEVELS_GLOB))) return;

   // name1:level1,name2:level2,name3:level3,...
   while (1)
     {
	Eina_Log_Domain_Level_Pending *p;
	char *end = NULL;
	char *tmp = NULL;
	long int level;

	end = strchr(start, ':');
	if (!end) break;

	// Parse level, keep going if failed
	level = strtol((char *)(end + 1), &tmp, 10);
	if (tmp == (end + 1)) goto parse_end;

	// Parse name
	p = malloc(sizeof(Eina_Log_Domain_Level_Pending) + end - start + 1);
	if (!p) break;
	memcpy((char *)p->name, start, end - start);
	((char *)p->name)[end - start] = '\0';
	p->level = level;

	_glob_list = eina_inlist_append(_glob_list, EINA_INLIST_GET(p));

	parse_end:
	start = strchr(tmp, ',');
	if (start) start++;
	else break;
     }
}

static inline int
eina_log_domain_register_unlocked(const char *name, const char *color)
{
   Eina_Log_Domain_Level_Pending *pending = NULL;
   int i;

   for (i = 0; i < _log_domains_count; i++)
     {
	if (_log_domains[i].deleted)
	  {
	     // Found a flagged slot, free domain_str and replace slot
	     eina_log_domain_new(&_log_domains[i], name, color);
	     goto finish_register;
	  }
     }

   if (_log_domains_count >= _log_domains_allocated)
     {
	Eina_Log_Domain *tmp;
	size_t size;

	if (!_log_domains)
	  // special case for init, eina itself will allocate a dozen of domains
	  size = 24;
	else
	  // grow 8 buckets to minimize reallocs
	  size = _log_domains_allocated + 8;

	tmp = realloc(_log_domains, sizeof(Eina_Log_Domain) * size);

	if (tmp)
	  {
	     // Success!
	     _log_domains = tmp;
	     _log_domains_allocated = size;
	  }
	else
	   return -1;
     }

   // Use an allocated slot
   eina_log_domain_new(&_log_domains[i], name, color);
   _log_domains_count++;

finish_register:
   EINA_INLIST_FOREACH(_glob_list, pending) 
     {
        if (!fnmatch(pending->name, name, 0)) 
          {
             _log_domains[i].level = pending->level;
             break;
          }
     }

   EINA_INLIST_FOREACH(_pending_list, pending)
     {
	if (!strcmp(pending->name, name))
	  {
	     _log_domains[i].level = pending->level;
	     _pending_list = eina_inlist_remove(_pending_list, EINA_INLIST_GET(pending));
	     free(pending);
	     break;
	  }
     }

   // Check if level is still UNKNOWN, set it to global
   if (_log_domains[i].level == EINA_LOG_LEVEL_UNKNOWN)
      _log_domains[i].level = _log_level;

   return i;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @internal
 * @brief Initialize the log module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the log module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 *
 * @warning Not-MT: just call this function from main thread! The
 *          place where this function was called the first time is
 *          considered the main thread.
 */
Eina_Bool
eina_log_init(void)
{
   const char *level, *tmp;

   assert((sizeof(_names)/sizeof(_names[0])) == EINA_LOG_LEVELS);
   assert((sizeof(_colors)/sizeof(_colors[0])) == EINA_LOG_LEVELS + 1);

   // Check if color is disabled
   if ((tmp = getenv(EINA_LOG_ENV_COLOR_DISABLE)) && (atoi(tmp) == 1))
     _disable_color = EINA_TRUE;

   if ((tmp = getenv(EINA_LOG_ENV_FILE_DISABLE)) && (atoi(tmp) == 1))
     _disable_file = EINA_TRUE;

   if ((tmp = getenv(EINA_LOG_ENV_FUNCTION_DISABLE)) && (atoi(tmp) == 1))
     _disable_function = EINA_TRUE;

   if ((tmp = getenv(EINA_LOG_ENV_ABORT)) && (atoi(tmp) == 1))
     _abort_on_critical = EINA_TRUE;

   if ((tmp = getenv(EINA_LOG_ENV_ABORT_LEVEL)))
     _abort_level_on_critical = atoi(tmp);

   eina_log_print_prefix_update();

   // Global log level
   if ((level = getenv(EINA_LOG_ENV_LEVEL)))
      _log_level = atoi(level);

   // Register UNKNOWN domain, the default logger
   EINA_LOG_DOMAIN_GLOBAL = eina_log_domain_register("", NULL);

   if (EINA_LOG_DOMAIN_GLOBAL < 0)
     {
	fprintf(stderr, "Failed to create global logging domain.\n");
	return EINA_FALSE;
     }

   // Parse pending domains passed through EINA_LOG_LEVELS_GLOB
   eina_log_domain_parse_pending_globs();

   // Parse pending domains passed through EINA_LOG_LEVELS
   eina_log_domain_parse_pendings();

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the log module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the log module set up by
 * eina_log_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 *
 * @warning Not-MT: just call this function from main thread! The
 *          place where eina_log_init() (eina_init()) was called the
 *          first time is considered the main thread.
 */
Eina_Bool
eina_log_shutdown(void)
{
   Eina_Inlist *tmp;

   while (_log_domains_count--)
     {
	if (_log_domains[_log_domains_count].deleted)
	   continue;
	eina_log_domain_free(&_log_domains[_log_domains_count]);
     }

   free(_log_domains);

   _log_domains = NULL;
   _log_domains_count = 0;
   _log_domains_allocated = 0;

   while (_glob_list) 
     {
        tmp = _glob_list;
        _glob_list = _glob_list->next;
        free(tmp);
     }

   while (_pending_list)
     {
	tmp = _pending_list;
	_pending_list = _pending_list->next;
	free(tmp);
     }

   return EINA_TRUE;
}

#ifdef EFL_HAVE_PTHREAD

/**
 * @internal
 * @brief Activate the log mutex.
 *
 * This function activate the mutex in the eina log module. It is called by
 * eina_thread_init().
 *
 * @see eina_thread_init()
 */
void 
eina_log_threads_init(void)
{
   _main_thread = pthread_self();
   _threads_enabled = EINA_TRUE;
   INIT();
}

/**
 * @internal
 * @brief Shut down the log mutex.
 *
 * This function shuts down the mutex in the log module. 
 * It is called by eina_thread_shutdown().
 *
 * @see eina_thread_shutdown()
 */
void
eina_log_threads_shutdown(void)
{
    CHECK_MAIN();
    SHUTDOWN();
    _threads_enabled = EINA_FALSE;
}

#endif

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Log_Group Log
 *
 * @brief These functions provide log management for projects.
 *
 * To use the log system Eina must be initialized with eina_init() and
 * later shut down with eina_shutdown(). The most generic way to print
 * logs is to use eina_log_print() but the helper macros
 * EINA_LOG_ERR(), EINA_LOG_INFO(), EINA_LOG_WARN() and EINA_LOG_DBG()
 * should be used instead.
 *
 * Here is a straightforward example:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_log.h>
 *
 * void test_warn(void)
 * {
 *    EINA_LOG_WARN("Here is a warning message");
 * }
 *
 * int main(void)
 * {
 *    if (!eina_init())
 *    {
 *        printf("log during the initialization of Eina_Log module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    test_warn();
 *
 *    eina_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following command:
 *
 * @code
 * gcc -Wall -o test_Eina_Log test_eina.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 * If Eina is compiled without debug mode, then executing the
 * resulting program displays nothing because the default log level
 * is #EINA_LOG_LEVEL_ERR and we want to display a warning
 * message, which level is strictly greater than the log level (see
 * eina_log_print() for more informations). Now execute the program
 * with:
 *
 * @code
 * EINA_LOG_LEVEL=2 ./test_eina_log
 * @endcode
 *
 * You should see a message displayed in the terminal.
 *
 * For more information, you can look at the @ref tutorial_log_page.
 *
 * @{
 */


/**
 * @cond LOCAL
 */

EAPI int EINA_LOG_DOMAIN_GLOBAL = 0;

/**
 * @endcond
 */


/**
 * Enable logging module to handle threads.
 *
 * There is no disable option on purpose, if it is enabled, there is
 * no way back until you call the last eina_shutdown().
 *
 * There is no function to retrieve if threads are enabled as one is
 * not supposed to know this from outside.
 *
 * After this call is executed at least once, if Eina was compiled
 * with threads support then logging will lock around debug messages
 * and threads that are not the main thread will have its identifier
 * printed.
 *
 * The main thread is considered the thread where the first
 * eina_init() was called.
 */
EAPI void
eina_log_threads_enable(void)
{
#ifdef EFL_HAVE_PTHREAD
   _threads_enabled = 1;
   eina_log_print_prefix_update();
#endif
}

/**
 * Sets logging method to use.
 *
 * By default, eina_log_print_cb_stderr() is used.
 *
 * @note MT: safe to call from any thread.
 *
 * @note MT: given function @a cb will be called protected by mutex.
 *       This means you're safe from other calls but you should never
 *       call eina_log_print(), directly or indirectly.
 */
EAPI void
eina_log_print_cb_set(Eina_Log_Print_Cb cb, void *data)
{
   LOG_LOCK();
   _print_cb = cb;
   _print_cb_data = data;
   eina_log_print_prefix_update();
   LOG_UNLOCK();
}

/**
 * @brief Set the default log log level.
 *
 * @param level The log level.
 *
 * This function sets the log log level @p level. It is used in
 * eina_log_print().
 */
EAPI void
eina_log_level_set(Eina_Log_Level level)
{
   _log_level = level;
}

/**
 * @param name Domain name
 * @param color Color of the domain name
 *
 * @return Domain index that will be used as the DOMAIN parameter on log
 *         macros. A negative return value means an log ocurred.
 *
 * @note MT: safe to call from any thread.
 */
EAPI int
eina_log_domain_register(const char *name, const char *color)
{
   int r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, -1);

   LOG_LOCK();
   r = eina_log_domain_register_unlocked(name, color);
   LOG_UNLOCK();
   return r;
}


static inline void
eina_log_domain_unregister_unlocked(int domain)
{
   Eina_Log_Domain *d;

   if (domain >= _log_domains_count) return;

   d = &_log_domains[domain];
   eina_log_domain_free(d);
   d->deleted = 1;
}

/**
 * Forget about a logging domain registered by eina_log_domain_register()
 *
 * @param domain domain identifier as reported by eina_log_domain_register(),
 *        must be >= 0.
 *
 * @note MT: safe to call from any thread.
 */
EAPI void
eina_log_domain_unregister(int domain)
{
   EINA_SAFETY_ON_FALSE_RETURN(domain >= 0);
   LOG_LOCK();
   eina_log_domain_unregister_unlocked(domain);
   LOG_UNLOCK();
}

/**
 * Default logging method, this will output to standard error stream.
 *
 * This method will colorize output based on domain provided color and
 * message logging level. To disable color, set environment variable
 * EINA_LOG_COLOR_DISABLE=1. Similarly, to disable file and line
 * information, set EINA_LOG_FILE_DISABLE=1 or
 * EINA_LOG_FUNCTION_DISABLE=1 to avoid function name in output. It is
 * not acceptable to have both EINA_LOG_FILE_DISABLE and
 * EINA_LOG_FUNCTION_DISABLE at the same time, in this case just
 * EINA_LOG_FUNCTION_DISABLE will be considered and file information
 * will be printed anyways.
 *
 * @note MT: if threads are enabled, this function is called within locks.
 * @note MT: Threads different from main thread will have thread id
 *       appended to domain name.
 */
EAPI void
eina_log_print_cb_stderr(const Eina_Log_Domain *d, Eina_Log_Level level,
		const char *file, const char *fnc, int line, const char *fmt,
		__UNUSED__ void *data, va_list args)
{
   _eina_log_print_prefix(stderr, d, level, file, fnc, line);
   vfprintf(stderr, fmt, args);
   putc('\n', stderr);
}

/**
 * Alternative logging method, this will output to standard output stream.
 *
 * This method will colorize output based on domain provided color and
 * message logging level. To disable color, set environment variable
 * EINA_LOG_COLOR_DISABLE=1. Similarly, to disable file and line
 * information, set EINA_LOG_FILE_DISABLE=1 or
 * EINA_LOG_FUNCTION_DISABLE=1 to avoid function name in output. It is
 * not acceptable to have both EINA_LOG_FILE_DISABLE and
 * EINA_LOG_FUNCTION_DISABLE at the same time, in this case just
 * EINA_LOG_FUNCTION_DISABLE will be considered and file information
 * will be printed anyways.
 *
 * @note MT: if threads are enabled, this function is called within locks.
 * @note MT: Threads different from main thread will have thread id
 *       appended to domain name.
 */
EAPI void
eina_log_print_cb_stdout(const Eina_Log_Domain *d, Eina_Log_Level level,
		const char *file, const char *fnc, int line, const char *fmt,
		__UNUSED__ void *data, va_list args)
{
   _eina_log_print_prefix(stdout, d, level, file, fnc, line);
   vprintf(fmt, args);
   putchar('\n');
}

/**
 * Alternative logging method, this will output to given file stream.
 *
 * This method will never output color.
 *
 * @note MT: if threads are enabled, this function is called within locks.
 * @note MT: Threads different from main thread will have thread id
 *       appended to domain name.
 */
EAPI void
eina_log_print_cb_file(const Eina_Log_Domain *d, __UNUSED__ Eina_Log_Level level,
		const char *file, const char *fnc, int line, const char *fmt,
		void *data, va_list args)
{
   FILE *f = data;
#ifdef EFL_HAVE_PTHREAD
   if (_threads_enabled)
     {
	pthread_t cur = pthread_self();
	if (IS_OTHER(cur))
	  {
	     fprintf(f, "%s[T:%lu] %s:%d %s() ", d->name, cur, file, line, fnc);
	     goto end;
	  }
     }
#endif
   fprintf(f, "%s %s:%d %s() ", d->name, file, line, fnc);
 end:
   vfprintf(f, fmt, args);
   putc('\n', f);
}

static inline void
eina_log_print_unlocked(int domain, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, va_list args)
{
   Eina_Log_Domain *d;

#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(domain >= _log_domains_count) ||
       EINA_UNLIKELY(domain < 0))
     {
	if (file && fnc && fmt)
	  fprintf(stderr, "CRI: %s:%d %s() eina_log_print() unknown domain %d, original message format '%s'\n", file, line, fnc, domain, fmt);
	else
	  fprintf(stderr, "CRI: eina_log_print() unknown domain %d, original message format '%s'\n", domain, fmt ? fmt : "");
	if (_abort_on_critical) abort();
	return;
     }
#endif
   d = _log_domains + domain;
#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(d->deleted))
     {
	fprintf(stderr, "ERR: eina_log_print() domain %d is deleted\n", domain);
	return;
     }
#endif

   if (level > d->level) return;

   _print_cb(d, level, file, fnc, line, fmt, _print_cb_data, args);

   if (EINA_UNLIKELY(_abort_on_critical) &&
       EINA_UNLIKELY(level <= _abort_level_on_critical))
     abort();
}

/**
 * Print out log message using given domain and level.
 *
 * @note Usually you'll not use this function directly but the helper
 *       macros EINA_LOG(), EINA_LOG_DOM_CRIT(), EINA_LOG_CRIT() and
 *       so on. See eina_log.h
 *
 * @param domain logging domain to use or @c EINA_LOG_DOMAIN_GLOBAL if
 *        you registered none. It is recommended that modules and
 *        applications have their own logging domain.
 * @param level message level, those with level greater than user
 *        specified value (eina_log_level_set() or environment
 *        variables EINA_LOG_LEVEL, EINA_LOG_LEVELS) will be ignored.
 * @param file filename that originated the call, must @b not be @c NULL.
 * @param fnc function that originated the call, must @b not be @c NULL.
 * @param line originating line in @a file.
 * @param fmt printf-like format to use. Should not provide trailing
 *        '\n' as it is automatically included.
 *
 * @note MT: this function may be called from different threads if
 *       eina_log_threads_enable() was called before.
 */
EAPI void
eina_log_print(int domain, Eina_Log_Level level, const char *file,
		const char *fnc, int line, const char *fmt, ...)
{
   va_list args;

#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(file == NULL))
     {
	fputs("ERR: eina_log_print() file == NULL\n", stderr);
	return;
     }
   if (EINA_UNLIKELY(fnc == NULL))
     {
	fputs("ERR: eina_log_print() fnc == NULL\n", stderr);
	return;
     }
   if (EINA_UNLIKELY(fmt == NULL))
     {
	fputs("ERR: eina_log_print() fmt == NULL\n", stderr);
	return;
     }
#endif
   va_start(args, fmt);
   LOG_LOCK();
   eina_log_print_unlocked(domain, level, file, fnc, line, fmt, args);
   LOG_UNLOCK();
   va_end(args);
}

/**
 * Print out log message using given domain and level.
 *
 * @note Usually you'll not use this function directly but the helper
 *       macros EINA_LOG(), EINA_LOG_DOM_CRIT(), EINA_LOG_CRIT() and
 *       so on. See eina_log.h
 *
 * @param domain logging domain to use or @c EINA_LOG_DOMAIN_GLOBAL if
 *        you registered none. It is recommended that modules and
 *        applications have their own logging domain.
 * @param level message level, those with level greater than user
 *        specified value (eina_log_level_set() or environment
 *        variables EINA_LOG_LEVEL, EINA_LOG_LEVELS) will be ignored.
 * @param file filename that originated the call, must @b not be @c NULL.
 * @param fnc function that originated the call, must @b not be @c NULL.
 * @param line originating line in @a file.
 * @param fmt printf-like format to use. Should not provide trailing
 *        '\n' as it is automatically included.
 *
 * @note MT: this function may be called from different threads if
 *       eina_log_threads_enable() was called before.
 *
 * @see eina_log_print()
 */
EAPI void
eina_log_vprint(int domain, Eina_Log_Level level, const char *file,
		const char *fnc, int line, const char *fmt, va_list args)
{
#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(file == NULL))
     {
	fputs("ERR: eina_log_print() file == NULL\n", stderr);
	return;
     }
   if (EINA_UNLIKELY(fnc == NULL))
     {
	fputs("ERR: eina_log_print() fnc == NULL\n", stderr);
	return;
     }
   if (EINA_UNLIKELY(fmt == NULL))
     {
	fputs("ERR: eina_log_print() fmt == NULL\n", stderr);
	return;
     }
#endif
   LOG_LOCK();
   eina_log_print_unlocked(domain, level, file, fnc, line, fmt, args);
   LOG_UNLOCK();
}

/**
 * @}
 */
