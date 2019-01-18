#ifndef EINA_INTERNAL_H
# define EINA_INTERNAL_H

/*
 * eina_internal.h
 *
 * Lists public functions that are meant for internal use by EFL only and are
 * not stable API.
 */

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
# define EAPI_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK __attribute__ ((weak))
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# else
/**
 * @def EAPI
 * @brief Used to export functions (by changing visibility).
 */
#  define EAPI
# endif
#endif

#include "eina_prefix.h"
#include "eina_promise.h"

typedef struct _Eina_Vpath_Interface_User Eina_Vpath_Interface_User;

struct _Eina_Vpath_Interface_User
{
   const char *desktop;
   const char *documents;
   const char *downloads;
   const char *music;
   const char *pictures;
   const char *pub;
   const char *templates;
   const char *videos;
   const char *data;
   const char *config;
   const char *cache;
   const char *run;
   const char *tmp;
};

/**
 * @brief Cancels all pending promise/futures.
 *
 * Internal function. Do not use.
 *
 * @internal
 */
EAPI void __eina_promise_cancel_all(void);

EAPI void __eina_promise_cancel_data(void *data);

/**
 * Make the app specific paths accessible as virtual path
 *
 * This will create :
 *   - app.dir
 *   - app.bin
 *   - app.lib
 *   - app.data
 *   - app.locale
 *   - app.config
 *   - app.cache
 *   - app.local
 *
 * If you do NOT call this api the virtual paths for app.* will be unset
 */
EAPI void eina_vpath_interface_app_set(const char *app_name, Eina_Prefix *p);

/**
 * Create the desktop specific vpaths
 *
 * The virtual paths will be named usr.<field-name-of-struct>
 *
 * If you do NOT call this api the virtual paths for usr.* will be unset.
 */
EAPI void eina_vpath_interface_user_set(Eina_Vpath_Interface_User *user);

void eina_xdg_env_init(void);

#undef EAPI
#define EAPI

#endif
