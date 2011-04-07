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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <assert.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef EFL_HAVE_POSIX_THREADS
# include <pthread.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_inlist.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_log.h"

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
   size_t namelen;
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

#ifdef EFL_HAVE_THREADS

static Eina_Bool _threads_enabled = EINA_FALSE;
static Eina_Bool _threads_inited = EINA_FALSE;

# ifdef EFL_HAVE_POSIX_THREADS

typedef pthread_t Thread;

static pthread_t _main_thread;

#  define SELF() pthread_self()
#  define IS_MAIN(t)  pthread_equal(t, _main_thread)
#  define IS_OTHER(t) EINA_UNLIKELY(!IS_MAIN(t))
#  define CHECK_MAIN(...)                                         \
   do {                                                           \
      if (!IS_MAIN(pthread_self())) {                             \
         fprintf(stderr,                                          \
                 "ERR: not main thread! current=%lu, main=%lu\n", \
                 (unsigned long)pthread_self(),                   \
                 (unsigned long)_main_thread);                    \
         return __VA_ARGS__;                                      \
      }                                                           \
   } while (0)

#  ifdef EFL_HAVE_POSIX_THREADS_SPINLOCK

static pthread_spinlock_t _log_lock;

static Eina_Bool _eina_log_spinlock_init(void)
{
   if (pthread_spin_init(&_log_lock, PTHREAD_PROCESS_PRIVATE) == 0)
     return EINA_TRUE;

   fprintf(stderr,
           "ERROR: pthread_spin_init(%p, PTHREAD_PROCESS_PRIVATE): %s\n",
           &_log_lock, strerror(errno));
   return EINA_FALSE;
}

#   define LOG_LOCK()                                                  \
   if (_threads_enabled)                                               \
         do {                                                          \
            if (0) {                                                   \
               fprintf(stderr, "+++LOG LOG_LOCKED!   [%s, %lu]\n",     \
                       __FUNCTION__, (unsigned long)pthread_self()); } \
            if (EINA_UNLIKELY(_threads_enabled)) {                     \
               pthread_spin_lock(&_log_lock); }                        \
         } while (0)
#   define LOG_UNLOCK()                                                \
   if (_threads_enabled)                                               \
         do {                                                          \
            if (EINA_UNLIKELY(_threads_enabled)) {                     \
               pthread_spin_unlock(&_log_lock); }                      \
            if (0) {                                                   \
               fprintf(stderr,                                         \
                       "---LOG LOG_UNLOCKED! [%s, %lu]\n",             \
                       __FUNCTION__, (unsigned long)pthread_self()); } \
         } while (0)
#   define INIT() _eina_log_spinlock_init()
#   define SHUTDOWN() pthread_spin_destroy(&_log_lock)

#  else /* ! EFL_HAVE_POSIX_THREADS_SPINLOCK */

static pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;
#   define LOG_LOCK() if(_threads_enabled) {pthread_mutex_lock(&_log_mutex); }
#   define LOG_UNLOCK() if(_threads_enabled) {pthread_mutex_unlock(&_log_mutex); }
#   define INIT() (1)
#   define SHUTDOWN() do {} while (0)

#  endif /* ! EFL_HAVE_POSIX_THREADS_SPINLOCK */

# else /* EFL_HAVE_WIN32_THREADS */

typedef DWORD Thread;

static DWORD _main_thread;

#  define SELF() GetCurrentThreadId()
#  define IS_MAIN(t)  (t == _main_thread)
#  define IS_OTHER(t) EINA_UNLIKELY(!IS_MAIN(t))
#  define CHECK_MAIN(...)                                         \
   do {                                                           \
      if (!IS_MAIN(GetCurrentThreadId())) {                       \
         fprintf(stderr,                                          \
                 "ERR: not main thread! current=%lu, main=%lu\n", \
                 GetCurrentThreadId(), _main_thread);             \
         return __VA_ARGS__;                                      \
      }                                                           \
   } while (0)

static HANDLE _log_mutex = NULL;

#  define LOG_LOCK() if(_threads_enabled) WaitForSingleObject(_log_mutex, INFINITE)
#  define LOG_UNLOCK() if(_threads_enabled) ReleaseMutex(_log_mutex)
#  define INIT() ((_log_mutex = CreateMutex(NULL, FALSE, NULL)) ? 1 : 0)
#  define SHUTDOWN()  if (_log_mutex) CloseHandle(_log_mutex)

# endif /* EFL_HAVE_WIN32_THREADS */

#else /* ! EFL_HAVE_THREADS */

# define LOG_LOCK() do {} while (0)
# define LOG_UNLOCK() do {} while (0)
# define IS_MAIN(t)  (1)
# define IS_OTHER(t) (0)
# define CHECK_MAIN(...) do {} while (0)
# define INIT() (1)
# define SHUTDOWN() do {} while (0)

#endif /* ! EFL_HAVE_THREADS */


// List of domains registered
static Eina_Log_Domain *_log_domains = NULL;
static unsigned int _log_domains_count = 0;
static size_t _log_domains_allocated = 0;

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

/* NOTE: if you change this, also change:
 *   eina_log_print_level_name_get()
 *   eina_log_print_level_name_color_get()
 */
static const char *_names[] = {
   "CRI",
   "ERR",
   "WRN",
   "INF",
   "DBG",
};

#ifdef _WIN32
static int
eina_log_win32_color_get(const char *domain_str)
{
   char *str;
   char *tmp;
   char *tmp2;
   int code = -1;
   int lighted = 0;
   int ret = 0;

   str = strdup(domain_str);
   if (!str)
      return 0;

   /* this should not append */
   if (str[0] != '\033')
     {
        free(str);
        return 0;
     }

   /* we skip the first char and the [ */
   tmp = tmp2 = str + 2;
   while (*tmp != 'm')
     {
        if (*tmp == ';')
          {
             *tmp = '\0';
             code = atol(tmp2);
             tmp++;
             tmp2 = tmp;
          }

        tmp++;
     }
   *tmp = '\0';
   if (code < 0)
      code = atol(tmp2);
   else
      lighted = atol(tmp2);

   free(str);

   if (code < lighted)
     {
        int c;

        c = code;
        code = lighted;
        lighted = c;
     }

   if (lighted)
      ret = FOREGROUND_INTENSITY;

   if (code == 31)
      ret |= FOREGROUND_RED;
   else if (code == 32)
      ret |= FOREGROUND_GREEN;
   else if (code == 33)
      ret |= FOREGROUND_RED | FOREGROUND_GREEN;
   else if (code == 34)
      ret |= FOREGROUND_BLUE;
   else if (code == 36)
      ret |= FOREGROUND_GREEN | FOREGROUND_BLUE;
   else if (code == 37)
      ret |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

   return ret;
}
#endif

static inline unsigned int
eina_log_pid_get(void)
{
   return (unsigned int)getpid();
}

static inline void
eina_log_print_level_name_get(int level, const char **p_name)
{
   static char buf[4];
   /* NOTE: if you change this, also change
    *    eina_log_print_level_name_color_get()
    *    eina_log_level_name_get() (at eina_inline_log.x)
    */
   if (EINA_UNLIKELY(level < 0))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else if (EINA_UNLIKELY(level >= EINA_LOG_LEVELS))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else
      *p_name = _names[level];
}

#ifdef _WIN32
static inline void
eina_log_print_level_name_color_get(int level,
                                    const char **p_name,
                                    int *p_color)
{
   static char buf[4];
   /* NOTE: if you change this, also change:
    *   eina_log_print_level_name_get()
    */
   if (EINA_UNLIKELY(level < 0))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else if (EINA_UNLIKELY(level >= EINA_LOG_LEVELS))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else
      *p_name = _names[level];

   *p_color = eina_log_win32_color_get(eina_log_level_color_get(level));
}
#else
static inline void
eina_log_print_level_name_color_get(int level,
                                    const char **p_name,
                                    const char **p_color)
{
   static char buf[4];
   /* NOTE: if you change this, also change:
    *   eina_log_print_level_name_get()
    */
   if (EINA_UNLIKELY(level < 0))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else if (EINA_UNLIKELY(level >= EINA_LOG_LEVELS))
     {
        snprintf(buf, sizeof(buf), "%03d", level);
        *p_name = buf;
     }
   else
      *p_name = _names[level];

   *p_color = eina_log_level_color_get(level);
}
#endif

#define DECLARE_LEVEL_NAME(level) const char *name; \
   eina_log_print_level_name_get(level, &name)
#ifdef _WIN32
# define DECLARE_LEVEL_NAME_COLOR(level) const char *name; int color; \
   eina_log_print_level_name_color_get(level, &name, &color)
#else
# define DECLARE_LEVEL_NAME_COLOR(level) const char *name, *color; \
   eina_log_print_level_name_color_get(level, &name, &color)
#endif

/** No threads, No color */
static void
eina_log_print_prefix_NOthreads_NOcolor_file_func(FILE *fp,
                                                  const Eina_Log_Domain *d,
                                                  Eina_Log_Level level,
                                                  const char *file,
                                                  const char *fnc,
                                                  int line)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s<%u>:%s %s:%d %s() ", name, eina_log_pid_get(), 
           d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_NOthreads_NOcolor_NOfile_func(FILE *fp,
                                                    const Eina_Log_Domain *d,
                                                    Eina_Log_Level level,
                                                    const char *file __UNUSED__,
                                                    const char *fnc,
                                                    int line __UNUSED__)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s<%u>:%s %s() ", name, eina_log_pid_get(), d->domain_str, 
           fnc);
}

static void
eina_log_print_prefix_NOthreads_NOcolor_file_NOfunc(FILE *fp,
                                                    const Eina_Log_Domain *d,
                                                    Eina_Log_Level level,
                                                    const char *file,
                                                    const char *fnc __UNUSED__,
                                                    int line)
{
   DECLARE_LEVEL_NAME(level);
   fprintf(fp, "%s<%u>:%s %s:%d ", name, eina_log_pid_get(), d->domain_str, 
           file, line);
}

/* No threads, color */
static void
eina_log_print_prefix_NOthreads_color_file_func(FILE *fp,
                                                const Eina_Log_Domain *d,
                                                Eina_Log_Level level,
                                                const char *file,
                                                const char *fnc,
                                                int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
#ifdef _WIN32_WCE
   fprintf(fp, "%s<%u>:%s %s:%d %s() ", name, eina_log_pid_get(), 
           d->domain_str, file, line, fnc);
#elif _WIN32
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           color);
   fprintf(fp, "%s", name);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, ":");
   SetConsoleTextAttribute(GetStdHandle(
                              STD_OUTPUT_HANDLE),
                           eina_log_win32_color_get(d->domain_str));
   fprintf(fp, "%s", d->name);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, " %s:%d ", file, line);
   SetConsoleTextAttribute(GetStdHandle(
                              STD_OUTPUT_HANDLE),
                           FOREGROUND_INTENSITY | FOREGROUND_RED |
                           FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, "%s()", fnc);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, " ");
#else
   fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s %s:%d "
           EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
           color, name, eina_log_pid_get(), d->domain_str, file, line, fnc);
#endif
}

static void
eina_log_print_prefix_NOthreads_color_NOfile_func(FILE *fp,
                                                  const Eina_Log_Domain *d,
                                                  Eina_Log_Level level,
                                                  const char *file __UNUSED__,
                                                  const char *fnc,
                                                  int line __UNUSED__)
{
   DECLARE_LEVEL_NAME_COLOR(level);
#ifdef _WIN32_WCE
   fprintf(fp, "%s<%u>:%s %s() ", name, eina_log_pid_get(), d->domain_str, 
           fnc);
#elif _WIN32
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           color);
   fprintf(fp, "%s", name);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, ":");
   SetConsoleTextAttribute(GetStdHandle(
                              STD_OUTPUT_HANDLE),
                           eina_log_win32_color_get(d->domain_str));
   fprintf(fp, "%s", d->name);
   SetConsoleTextAttribute(GetStdHandle(
                              STD_OUTPUT_HANDLE),
                           FOREGROUND_INTENSITY | FOREGROUND_RED |
                           FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, "%s()", fnc);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, " ");
#else
   fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s "
           EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
           color, name, eina_log_pid_get(), d->domain_str, fnc);
#endif
}

static void
eina_log_print_prefix_NOthreads_color_file_NOfunc(FILE *fp,
                                                  const Eina_Log_Domain *d,
                                                  Eina_Log_Level level,
                                                  const char *file,
                                                  const char *fnc __UNUSED__,
                                                  int line)
{
   DECLARE_LEVEL_NAME_COLOR(level);
#ifdef _WIN32_WCE
   fprintf(fp, "%s<%u>:%s %s:%d ", name, eina_log_pid_get(), d->domain_str, 
           file, line);
#elif _WIN32
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           color);
   fprintf(fp, "%s", name);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, ":");
   SetConsoleTextAttribute(GetStdHandle(
                              STD_OUTPUT_HANDLE),
                           eina_log_win32_color_get(d->domain_str));
   fprintf(fp, "%s", d->name);
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                           FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
   fprintf(fp, " %s:%d ", file, line);
#else
   fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s %s:%d ",
           color, name, eina_log_pid_get(), d->domain_str, file, line);
#endif
}

/** threads, No color */
#ifdef EFL_HAVE_THREADS
static void
eina_log_print_prefix_threads_NOcolor_file_func(FILE *fp,
                                                const Eina_Log_Domain *d,
                                                Eina_Log_Level level,
                                                const char *file,
                                                const char *fnc,
                                                int line)
{
   Thread cur;

   DECLARE_LEVEL_NAME(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
        fprintf(fp, "%s<%u>:%s[T:%lu] %s:%d %s() ",
                name, eina_log_pid_get(), d->domain_str, 
                (unsigned long)cur, file, line, fnc);
        return;
     }
   fprintf(fp, "%s<%u>:%s %s:%d %s() ", 
           name, eina_log_pid_get(), d->domain_str, file, line, fnc);
}

static void
eina_log_print_prefix_threads_NOcolor_NOfile_func(FILE *fp,
                                                  const Eina_Log_Domain *d,
                                                  Eina_Log_Level level,
                                                  const char *file __UNUSED__,
                                                  const char *fnc,
                                                  int line __UNUSED__)
{
   Thread cur;

   DECLARE_LEVEL_NAME(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
        fprintf(fp, "%s<%u>:%s[T:%lu] %s() ",
                name, eina_log_pid_get(), d->domain_str, 
                (unsigned long)cur, fnc);
        return;
     }
   fprintf(fp, "%s<%u>:%s %s() ", 
           name, eina_log_pid_get(), d->domain_str, fnc);
}

static void
eina_log_print_prefix_threads_NOcolor_file_NOfunc(FILE *fp,
                                                  const Eina_Log_Domain *d,
                                                  Eina_Log_Level level,
                                                  const char *file,
                                                  const char *fnc __UNUSED__,
                                                  int line)
{
   Thread cur;

   DECLARE_LEVEL_NAME(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
        fprintf(fp, "%s<%u>:%s[T:%lu] %s:%d ",
                name, eina_log_pid_get(), d->domain_str, (unsigned long)cur, 
                file, line);
        return;
     }
   
   fprintf(fp, "%s<%u>:%s %s:%d ", 
           name, eina_log_pid_get(), d->domain_str, file, line);
}

/* threads, color */
static void
eina_log_print_prefix_threads_color_file_func(FILE *fp,
                                              const Eina_Log_Domain *d,
                                              Eina_Log_Level level,
                                              const char *file,
                                              const char *fnc,
                                              int line)
{
   Thread cur;

   DECLARE_LEVEL_NAME_COLOR(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
# ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                color);
        fprintf(fp, "%s", name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, ":");
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                eina_log_win32_color_get(d->domain_str));
        fprintf(fp, "%s[T:", d->name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, "[T:");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_GREEN | FOREGROUND_BLUE);
        fprintf(fp, "%lu", (unsigned long)cur);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, "] %s:%d ", file, line);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | FOREGROUND_RED |
                                FOREGROUND_GREEN | FOREGROUND_BLUE);
        fprintf(fp, "%s()", fnc);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, " ");
# else
        fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s[T:"
                EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] %s:%d "
                EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
                color, name, eina_log_pid_get() ,d->domain_str, 
                (unsigned long)cur, file, line, fnc);
# endif
        return;
     }

# ifdef _WIN32
   eina_log_print_prefix_NOthreads_color_file_func(fp,
                                                   d,
                                                   level,
                                                   file,
                                                   fnc,
                                                   line);
# else
   fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s %s:%d "
           EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
           color, name, eina_log_pid_get(), d->domain_str, file, line, fnc);
# endif
}

static void
eina_log_print_prefix_threads_color_NOfile_func(FILE *fp,
                                                const Eina_Log_Domain *d,
                                                Eina_Log_Level level,
                                                const char *file __UNUSED__,
                                                const char *fnc,
                                                int line __UNUSED__)
{
   Thread cur;

   DECLARE_LEVEL_NAME_COLOR(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
# ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                color);
        fprintf(fp, "%s", name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, ":");
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                eina_log_win32_color_get(d->domain_str));
        fprintf(fp, "%s[T:", d->name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, "[T:");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_GREEN | FOREGROUND_BLUE);
        fprintf(fp, "%lu", (unsigned long)cur);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | FOREGROUND_RED |
                                FOREGROUND_GREEN | FOREGROUND_BLUE);
        fprintf(fp, "%s()", fnc);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, " ");
# else
        fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s[T:"
                EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] "
                EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
                color, name, eina_log_pid_get(), d->domain_str, 
                (unsigned long)cur, fnc);
# endif
        return;
     }

# ifdef _WIN32
   eina_log_print_prefix_NOthreads_color_NOfile_func(fp,
                                                     d,
                                                     level,
                                                     file,
                                                     fnc,
                                                     line);
# else
   fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s "
           EINA_COLOR_HIGH "%s()" EINA_COLOR_RESET " ",
           color, name, eina_log_pid_get(), d->domain_str, fnc);
# endif
}

static void
eina_log_print_prefix_threads_color_file_NOfunc(FILE *fp,
                                                const Eina_Log_Domain *d,
                                                Eina_Log_Level level,
                                                const char *file,
                                                const char *fnc __UNUSED__,
                                                int line)
{
   Thread cur;

   DECLARE_LEVEL_NAME_COLOR(level);
   cur = SELF();
   if (IS_OTHER(cur))
     {
# ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                color);
        fprintf(fp, "%s", name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, ":");
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                eina_log_win32_color_get(d->domain_str));
        fprintf(fp, "%s[T:", d->name);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, "[T:");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_GREEN | FOREGROUND_BLUE);
        fprintf(fp, "%lu", (unsigned long)cur);
        SetConsoleTextAttribute(GetStdHandle(
                                   STD_OUTPUT_HANDLE),
                                FOREGROUND_RED | FOREGROUND_GREEN |
                                FOREGROUND_BLUE);
        fprintf(fp, "] %s:%d ", file, line);
# else
        fprintf(fp, "%s%s<%u>" EINA_COLOR_RESET ":%s[T:"
                EINA_COLOR_ORANGE "%lu" EINA_COLOR_RESET "] %s:%d ",
                color, name, eina_log_pid_get(), d->domain_str, 
                (unsigned long)cur, file, line);
# endif
        return;
     }

# ifdef _WIN32
   eina_log_print_prefix_NOthreads_color_file_NOfunc(fp,
                                                     d,
                                                     level,
                                                     file,
                                                     fnc,
                                                     line);
# else
        fprintf(fp, "%s%s" EINA_COLOR_RESET ":%s %s:%d ",
           color, name, d->domain_str, file, line);
# endif
}
#endif /* EFL_HAVE_THREADS */

static void (*_eina_log_print_prefix)(FILE *fp, const Eina_Log_Domain *d,
                                      Eina_Log_Level level, const char *file,
                                      const char *fnc,
                                      int line) =
   eina_log_print_prefix_NOthreads_color_file_func;

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
   _eina_log_print_prefix = \
      eina_log_print_prefix_ ## NOthread ## threads_ ## NOcolor ## color_ ## \
      NOfile \
      ## file_ ## NOfunc ## func

#ifdef EFL_HAVE_THREADS
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
        d =
           malloc(sizeof(char) *
                  (color_len + name_len + strlen(EINA_COLOR_RESET) + 1));
        if (!d)
           return NULL;

               memcpy((char *)d,                          color, color_len);
               memcpy((char *)(d + color_len),            name,  name_len);
               memcpy((char *)(d + color_len + name_len), EINA_COLOR_RESET,
               strlen(EINA_COLOR_RESET));
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(d,    NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   d->level = EINA_LOG_LEVEL_UNKNOWN;
   d->deleted = EINA_FALSE;

   if ((color) && (!_disable_color))
      d->domain_str = eina_log_domain_str_get(name, color);
   else
      d->domain_str = eina_log_domain_str_get(name, NULL);

   d->name = strdup(name);
   d->namelen = strlen(name);

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

   if (!(start = getenv(EINA_LOG_ENV_LEVELS)))
      return;

   // name1:level1,name2:level2,name3:level3,...
   while (1)
     {
        Eina_Log_Domain_Level_Pending *p;
        char *end = NULL;
        char *tmp = NULL;
        long int level;

        end = strchr(start, ':');
        if (!end)
           break;

        // Parse level, keep going if failed
        level = strtol((char *)(end + 1), &tmp, 10);
        if (tmp == (end + 1))
           goto parse_end;

        // Parse name
        p = malloc(sizeof(Eina_Log_Domain_Level_Pending) + end - start + 1);
        if (!p)
           break;

        p->namelen = end - start;
        memcpy((char *)p->name, start, end - start);
        ((char *)p->name)[end - start] = '\0';
        p->level = level;

        _pending_list = eina_inlist_append(_pending_list, EINA_INLIST_GET(p));

parse_end:
        start = strchr(tmp, ',');
        if (start)
           start++;
        else
           break;
     }
}

static void
eina_log_domain_parse_pending_globs(void)
{
   const char *start;

   if (!(start = getenv(EINA_LOG_ENV_LEVELS_GLOB)))
      return;

   // name1:level1,name2:level2,name3:level3,...
   while (1)
     {
        Eina_Log_Domain_Level_Pending *p;
        char *end = NULL;
        char *tmp = NULL;
        long int level;

        end = strchr(start, ':');
        if (!end)
           break;

        // Parse level, keep going if failed
        level = strtol((char *)(end + 1), &tmp, 10);
        if (tmp == (end + 1))
           goto parse_end;

        // Parse name
        p = malloc(sizeof(Eina_Log_Domain_Level_Pending) + end - start + 1);
        if (!p)
           break;

        p->namelen = 0; /* not that useful */
        memcpy((char *)p->name, start, end - start);
        ((char *)p->name)[end - start] = '\0';
        p->level = level;

        _glob_list = eina_inlist_append(_glob_list, EINA_INLIST_GET(p));

parse_end:
        start = strchr(tmp, ',');
        if (start)
           start++;
        else
           break;
     }
}

static inline int
eina_log_domain_register_unlocked(const char *name, const char *color)
{
   Eina_Log_Domain_Level_Pending *pending = NULL;
   size_t namelen;
   unsigned int i;

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
   namelen = _log_domains[i].namelen;

   EINA_INLIST_FOREACH(_pending_list, pending)
   {
      if ((namelen == pending->namelen) && (strcmp(pending->name, name) == 0))
        {
           _log_domains[i].level = pending->level;
           _pending_list =
              eina_inlist_remove(_pending_list, EINA_INLIST_GET(pending));
           free(pending);
           break;
        }
   }

   if (_log_domains[i].level == EINA_LOG_LEVEL_UNKNOWN)
     {
        EINA_INLIST_FOREACH(_glob_list, pending)
        {
           if (!fnmatch(pending->name, name, 0))
             {
                _log_domains[i].level = pending->level;
                break;
             }
        }
     }

   // Check if level is still UNKNOWN, set it to global
   if (_log_domains[i].level == EINA_LOG_LEVEL_UNKNOWN)
      _log_domains[i].level = _log_level;

   return i;
}

static inline Eina_Bool
eina_log_term_color_supported(const char *term)
{
   const char *tail;

   if (!term)
      return EINA_FALSE;

   tail = term + 1;
   switch (term[0])
     {
      /* list of known to support color terminals,
       * take from gentoo's portage.
       */

      case 'x': /* xterm and xterm-color */
         return ((strncmp(tail, "term", sizeof("term") - 1) == 0) &&
                 ((tail[sizeof("term") - 1] == '\0') ||
                  (strcmp(tail + sizeof("term") - 1, "-color") == 0)));

      case 'E': /* Eterm */
      case 'a': /* aterm */
      case 'k': /* kterm */
         return (strcmp(tail, "term") == 0);

      case 'r': /* xrvt or rxvt-unicode */
         return ((strncmp(tail, "xvt", sizeof("xvt") - 1) == 0) &&
                 ((tail[sizeof("xvt") - 1] == '\0') ||
                  (strcmp(tail + sizeof("xvt") - 1, "-unicode") == 0)));

      case 's': /* screen */
         return (strcmp(tail, "creen") == 0);

      case 'g': /* gnome */
         return (strcmp(tail, "nome") == 0);

      case 'i': /* interix */
         return (strcmp(tail, "nterix") == 0);

      default:
         return EINA_FALSE;
     }
}

static inline void
eina_log_domain_unregister_unlocked(int domain)
{
   Eina_Log_Domain *d;

   if ((unsigned int)domain >= _log_domains_count)
      return;

   d = &_log_domains[domain];
   eina_log_domain_free(d);
   d->deleted = 1;
}

static inline void
eina_log_print_unlocked(int domain,
                        Eina_Log_Level level,
                        const char *file,
                        const char *fnc,
                        int line,
                        const char *fmt,
                        va_list args)
{
   Eina_Log_Domain *d;

#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY((unsigned int)domain >= _log_domains_count) ||
       EINA_UNLIKELY(domain < 0))
     {
        if (file && fnc && fmt)
           fprintf(
              stderr,
              "CRI: %s:%d %s() eina_log_print() unknown domain %d, original message format '%s'\n",
              file,
              line,
              fnc,
              domain,
              fmt);
        else
           fprintf(
              stderr,
              "CRI: eina_log_print() unknown domain %d, original message format '%s'\n",
              domain,
              fmt ? fmt : "");

        if (_abort_on_critical)
           abort();

        return;
     }

#endif
   d = _log_domains + domain;
#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(d->deleted))
     {
           fprintf(stderr,
                "ERR: eina_log_print() domain %d is deleted\n",
                domain);
        return;
     }

#endif

   if (level > d->level)
      return;

#ifdef _WIN32
   {
      char *wfmt;
      char *tmp;

      wfmt = strdup(fmt);
      if (!wfmt)
        {
           fprintf(stderr, "ERR: %s: can not allocate memory\n", __FUNCTION__);
           return;
        }

      tmp = wfmt;
      while (strchr(tmp, '%'))
        {
           tmp++;
           if (*tmp == 'z')
              *tmp = 'I';
        }
      _print_cb(d, level, file, fnc, line, wfmt, _print_cb_data, args);
      free(wfmt);
   }
#else
   _print_cb(d, level, file, fnc, line, fmt, _print_cb_data, args);
#endif

   if (EINA_UNLIKELY(_abort_on_critical) &&
       EINA_UNLIKELY(level <= _abort_level_on_critical))
      abort();
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
   int color_disable;

   assert((sizeof(_names) / sizeof(_names[0])) == EINA_LOG_LEVELS);

   if ((tmp = getenv(EINA_LOG_ENV_COLOR_DISABLE)))
      color_disable = atoi(tmp);
   else
      color_disable = -1;

   /* Check if color is explicitly disabled */
   if (color_disable == 1)
      _disable_color = EINA_TRUE;

#ifndef _WIN32
   /* color was not explicitly disabled or enabled, guess it */
   else if (color_disable == -1)
     {
        if (!eina_log_term_color_supported(getenv("TERM")))
           _disable_color = EINA_TRUE;
        else
          {
             /* if not a terminal, but redirected to a file, disable color */
             int fd;

             if (_print_cb == eina_log_print_cb_stderr)
                fd = STDERR_FILENO;
             else if (_print_cb == eina_log_print_cb_stdout)
                fd = STDOUT_FILENO;
             else
                fd = -1;

             if ((fd >= 0) && (!isatty(fd)))
                _disable_color = EINA_TRUE;
          }
     }
#endif

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

#ifdef EFL_HAVE_THREADS

/**
 * @internal
 * @brief Activate the log mutex.
 *
 * This function activate the mutex in the eina log module. It is called by
 * eina_threads_init().
 *
 * @see eina_threads_init()
 */
void
eina_log_threads_init(void)
{
   if (_threads_inited) return;
   _main_thread = SELF();
   if (!INIT()) return;
   _threads_inited = EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the log mutex.
 *
 * This function shuts down the mutex in the log module.
 * It is called by eina_threads_shutdown().
 *
 * @see eina_threads_shutdown()
 */
void
eina_log_threads_shutdown(void)
{
   if (!_threads_inited) return;
   CHECK_MAIN();
   SHUTDOWN();
   _threads_enabled = EINA_FALSE;
   _threads_inited = EINA_FALSE;
}

#endif

/*============================================================================*
*                                   API                                      *
*============================================================================*/

/**
 * @cond LOCAL
 */

EAPI int EINA_LOG_DOMAIN_GLOBAL = 0;

/**
 * @endcond
 */

EAPI void
eina_log_threads_enable(void)
{
#ifdef EFL_HAVE_THREADS
   if (_threads_enabled) return;
   if (!_threads_inited) eina_log_threads_init();
   _threads_enabled = EINA_TRUE;
   eina_log_print_prefix_update();
#endif
}

EAPI void
eina_log_print_cb_set(Eina_Log_Print_Cb cb, void *data)
{
   LOG_LOCK();
   _print_cb = cb;
   _print_cb_data = data;
   eina_log_print_prefix_update();
   LOG_UNLOCK();
}

EAPI void
eina_log_level_set(int level)
{
   _log_level = level;
   if (EINA_LIKELY((EINA_LOG_DOMAIN_GLOBAL >= 0) &&
                   ((unsigned int)EINA_LOG_DOMAIN_GLOBAL < _log_domains_count)))
      _log_domains[EINA_LOG_DOMAIN_GLOBAL].level = level;
}

EAPI int
eina_log_level_get(void)
{
   return _log_level;
}

EAPI Eina_Bool
eina_log_main_thread_check(void)
{
#ifdef EFL_HAVE_THREADS
   return ((!_threads_enabled) || IS_MAIN(SELF()));
#else
   return EINA_TRUE;
#endif
}

EAPI void
eina_log_color_disable_set(Eina_Bool disabled)
{
   _disable_color = disabled;
}

EAPI Eina_Bool
eina_log_color_disable_get(void)
{
   return _disable_color;
}

EAPI void
eina_log_file_disable_set(Eina_Bool disabled)
{
   _disable_file = disabled;
}

EAPI Eina_Bool
eina_log_file_disable_get(void)
{
   return _disable_file;
}

EAPI void
eina_log_function_disable_set(Eina_Bool disabled)
{
   _disable_function = disabled;
}

EAPI Eina_Bool
eina_log_function_disable_get(void)
{
   return _disable_function;
}

EAPI void
eina_log_abort_on_critical_set(Eina_Bool abort_on_critical)
{
   _abort_on_critical = abort_on_critical;
}

EAPI Eina_Bool
eina_log_abort_on_critical_get(void)
{
   return _abort_on_critical;
}

EAPI void
eina_log_abort_on_critical_level_set(int critical_level)
{
   _abort_level_on_critical = critical_level;
}

EAPI int
eina_log_abort_on_critical_level_get(void)
{
   return _abort_level_on_critical;
}

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

EAPI void
eina_log_domain_unregister(int domain)
{
   EINA_SAFETY_ON_FALSE_RETURN(domain >= 0);
   LOG_LOCK();
   eina_log_domain_unregister_unlocked(domain);
   LOG_UNLOCK();
}

EAPI void
eina_log_domain_level_set(const char *domain_name, int level)
{
   Eina_Log_Domain_Level_Pending *pending;
   size_t namelen;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(domain_name);

   namelen = strlen(domain_name);

   for (i = 0; i < _log_domains_count; i++)
     {
        if (_log_domains[i].deleted)
           continue;

        if ((namelen != _log_domains[i].namelen) ||
            (strcmp(_log_domains[i].name, domain_name) != 0))
           continue;

        _log_domains[i].level = level;
        return;
     }

   EINA_INLIST_FOREACH(_pending_list, pending)
   {
      if ((namelen == pending->namelen) &&
          (strcmp(pending->name, domain_name) == 0))
        {
           pending->level = level;
           return;
        }
   }

   pending = malloc(sizeof(Eina_Log_Domain_Level_Pending) + namelen + 1);
   if (!pending)
      return;

   pending->level = level;
   pending->namelen = namelen;
   memcpy(pending->name, domain_name, namelen + 1);

   _pending_list = eina_inlist_append(_pending_list, EINA_INLIST_GET(pending));
}

EAPI int
eina_log_domain_level_get(const char *domain_name)
{
   Eina_Log_Domain_Level_Pending *pending;
   size_t namelen;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(domain_name, EINA_LOG_LEVEL_UNKNOWN);

   namelen = strlen(domain_name);

   for (i = 0; i < _log_domains_count; i++)
     {
        if (_log_domains[i].deleted)
           continue;

        if ((namelen != _log_domains[i].namelen) ||
            (strcmp(_log_domains[i].name, domain_name) != 0))
           continue;

        return _log_domains[i].level;
     }

   EINA_INLIST_FOREACH(_pending_list, pending)
   {
      if ((namelen == pending->namelen) &&
          (strcmp(pending->name, domain_name) == 0))
         return pending->level;
   }

   EINA_INLIST_FOREACH(_glob_list, pending)
   {
      if (!fnmatch(pending->name, domain_name, 0))
         return pending->level;
   }

   return _log_level;
}

EAPI int
eina_log_domain_registered_level_get(int domain)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(domain >= 0, EINA_LOG_LEVEL_UNKNOWN);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((unsigned int)domain < _log_domains_count,
                                   EINA_LOG_LEVEL_UNKNOWN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(_log_domains[domain].deleted,
                                  EINA_LOG_LEVEL_UNKNOWN);
   return _log_domains[domain].level;
}

EAPI void
eina_log_print_cb_stderr(const Eina_Log_Domain *d,
                         Eina_Log_Level level,
                         const char *file,
                         const char *fnc,
                         int line,
                         const char *fmt,
                         __UNUSED__ void *data,
                         va_list args)
{
   _eina_log_print_prefix(stderr, d, level, file, fnc, line);
   vfprintf(stderr, fmt, args);
   putc('\n', stderr);
}

EAPI void
eina_log_print_cb_stdout(const Eina_Log_Domain *d,
                         Eina_Log_Level level,
                         const char *file,
                         const char *fnc,
                         int line,
                         const char *fmt,
                         __UNUSED__ void *data,
                         va_list args)
{
   _eina_log_print_prefix(stdout, d, level, file, fnc, line);
   vprintf(fmt, args);
   putchar('\n');
}

EAPI void
eina_log_print_cb_file(const Eina_Log_Domain *d,
                       __UNUSED__ Eina_Log_Level level,
                       const char *file,
                       const char *fnc,
                       int line,
                       const char *fmt,
                       void *data,
                       va_list args)
{
   FILE *f = data;
#ifdef EFL_HAVE_THREADS
   if (_threads_enabled)
     {
        Thread cur;

        cur = SELF();
        if (IS_OTHER(cur))
          {
             fprintf(f, "%s[T:%lu] %s:%d %s() ", d->name, (unsigned long)cur,
	        file, line, fnc);
             goto end;
          }
     }

#endif
   fprintf(f, "%s<%u> %s:%d %s() ", d->name, eina_log_pid_get(), 
           file, line, fnc);
#ifdef EFL_HAVE_THREADS
end:
#endif
   vfprintf(f, fmt, args);
   putc('\n', f);
}

EAPI void
eina_log_print(int domain, Eina_Log_Level level, const char *file,
               const char *fnc, int line, const char *fmt, ...)
{
   va_list args;

#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(!file))
     {
        fputs("ERR: eina_log_print() file == NULL\n", stderr);
        return;
     }

   if (EINA_UNLIKELY(!fnc))
     {
        fputs("ERR: eina_log_print() fnc == NULL\n", stderr);
        return;
     }

   if (EINA_UNLIKELY(!fmt))
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

EAPI void
eina_log_vprint(int domain, Eina_Log_Level level, const char *file,
                const char *fnc, int line, const char *fmt, va_list args)
{
#ifdef EINA_SAFETY_CHECKS
   if (EINA_UNLIKELY(!file))
     {
        fputs("ERR: eina_log_print() file == NULL\n", stderr);
        return;
     }

   if (EINA_UNLIKELY(!fnc))
     {
        fputs("ERR: eina_log_print() fnc == NULL\n", stderr);
        return;
     }

   if (EINA_UNLIKELY(!fmt))
     {
        fputs("ERR: eina_log_print() fmt == NULL\n", stderr);
        return;
     }

#endif
   LOG_LOCK();
   eina_log_print_unlocked(domain, level, file, fnc, line, fmt, args);
   LOG_UNLOCK();
}
