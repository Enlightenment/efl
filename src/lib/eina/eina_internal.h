#ifndef EINA_INTERNAL_H
# define EINA_INTERNAL_H

/*
 * eina_internal.h
 *
 * Lists public functions that are meant for internal use by EFL only and are
 * not stable API.
 */

#include "eina_types.h"
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
EINA_API void __eina_promise_cancel_all(void);

EINA_API void __eina_promise_cancel_data(void *data);

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
EINA_API void eina_vpath_interface_app_set(const char *app_name, Eina_Prefix *p);

/**
 * Create the desktop specific vpaths
 *
 * The virtual paths will be named usr.<field-name-of-struct>
 *
 * If you do NOT call this api the virtual paths for usr.* will be unset.
 */
EINA_API void eina_vpath_interface_user_set(Eina_Vpath_Interface_User *user);

void eina_xdg_env_init(void);

#endif
