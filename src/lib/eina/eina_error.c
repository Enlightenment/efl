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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "eina_config.h"
#include "eina_private.h"


/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_error.h"
#include "eina_stringshare.h"
#include "eina_lock.h"
#include "eina_str.h"
#ifdef EINA_HAVE_THREADS
#include "eina_hash.h"
#endif

/* TODO
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

typedef struct _Eina_Error_Message Eina_Error_Message;
struct _Eina_Error_Message
{
   Eina_Bool string_allocated;
   const char *string;
};

#ifdef EINA_HAVE_THREADS
static Eina_Spinlock _eina_errno_msgs_lock;
static Eina_Hash *_eina_errno_msgs = NULL;
#endif
static Eina_Error_Message *_eina_errors = NULL;
static size_t _eina_errors_count = 0;
static size_t _eina_errors_allocated = 0;

/* used to differentiate registered errors from errno.h */
#define EINA_ERROR_REGISTERED_BIT (1 << 30)
#define EINA_ERROR_REGISTERED_CHECK(err) ((err) & EINA_ERROR_REGISTERED_BIT)

#define EINA_ERROR_FROM_INDEX(idx) ((idx) | EINA_ERROR_REGISTERED_BIT)
#define EINA_ERROR_TO_INDEX(err) ((err) & (~EINA_ERROR_REGISTERED_BIT))

static Eina_Error _eina_last_error;
static Eina_TLS _eina_last_key;

static Eina_Error_Message *
_eina_error_msg_alloc(void)
{
   size_t idx;

   if (_eina_errors_count == _eina_errors_allocated)
     {
        void *tmp;
        size_t size;

        if (EINA_UNLIKELY(_eina_errors_allocated == 0))
           size = 24;
        else
           size = _eina_errors_allocated + 8;

        tmp = realloc(_eina_errors, sizeof(Eina_Error_Message) * size);
        if (!tmp)
           return NULL;

        _eina_errors = tmp;
        _eina_errors_allocated = size;
     }

   idx = _eina_errors_count;
   _eina_errors_count++;
   return _eina_errors + idx;
}

#ifdef _WIN32
# define HAVE_STRERROR_R
# ifdef STRERROR_R_CHAR_P
#  undef STRERROR_R_CHAR_P
# endif
/* Windows has strerror_s(), similar to POSIX strerror_r() */
static inline int strerror_r(int errnum, char *buf, size_t buflen)
{
   return strerror_s(buf, buflen, errnum);
}
#endif

/**
 * @endcond
 */


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

EAPI Eina_Error EINA_ERROR_OUT_OF_MEMORY = ENOMEM;

/**
 * @endcond
 */

/**
 * @internal
 * @brief Initialize the error module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the error module of Eina. It is called by
 * eina_init().
 *
 * This function registers the error #EINA_ERROR_OUT_OF_MEMORY.
 *
 * @see eina_init()
 */
Eina_Bool
eina_error_init(void)
{
   if (!eina_tls_new(&_eina_last_key))
     return EINA_FALSE;

#ifdef EINA_HAVE_THREADS
   if (!eina_spinlock_new(&_eina_errno_msgs_lock)) goto failed_lock;
   _eina_errno_msgs = eina_hash_int32_new(EINA_FREE_CB(eina_stringshare_del));
   if (!_eina_errno_msgs) goto failed_hash;
#endif

   return EINA_TRUE;

#ifdef EINA_HAVE_THREADS
 failed_hash:
   eina_spinlock_free(&_eina_errno_msgs_lock);
 failed_lock:
   eina_tls_free(_eina_last_key);
   _eina_last_error = 0;
   return EINA_FALSE;
#endif
}

/**
 * @internal
 * @brief Shut down the error module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the error module set up by
 * eina_error_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_error_shutdown(void)
{
   Eina_Error_Message *eem, *eem_end;

   eem = _eina_errors;
   eem_end = eem + _eina_errors_count;

   for (; eem < eem_end; eem++)
      if (eem->string_allocated)
         eina_stringshare_del(eem->string);

   free(_eina_errors);
   _eina_errors = NULL;
   _eina_errors_count = 0;
   _eina_errors_allocated = 0;

#ifdef EINA_HAVE_THREADS
   eina_hash_free(_eina_errno_msgs);
   _eina_errno_msgs = NULL;
   eina_spinlock_free(&_eina_errno_msgs_lock);
#endif

   eina_tls_free(_eina_last_key);
   _eina_last_error = 0;

   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI Eina_Error
eina_error_msg_register(const char *msg)
{
   Eina_Error_Message *eem;

   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, 0);

   eem = _eina_error_msg_alloc();
   if (!eem)
      return 0;

   eem->string_allocated = EINA_TRUE;
   eem->string = eina_stringshare_add(msg);
   if (!eem->string)
     {
        _eina_errors_count--;
        return 0;
     }

   return EINA_ERROR_FROM_INDEX(_eina_errors_count); /* identifier = index + 1 (== _count). */
}

EAPI Eina_Error
eina_error_msg_static_register(const char *msg)
{
   Eina_Error_Message *eem;

   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, 0);

   eem = _eina_error_msg_alloc();
   if (!eem)
      return 0;

   eem->string_allocated = EINA_FALSE;
   eem->string = msg;
   return EINA_ERROR_FROM_INDEX(_eina_errors_count); /* identifier = index + 1 (== _count). */
}

EAPI Eina_Bool
eina_error_msg_modify(Eina_Error error, const char *msg)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(EINA_ERROR_REGISTERED_CHECK(error), EINA_FALSE);
   error = EINA_ERROR_TO_INDEX(error);
   if (error < 1)
      return EINA_FALSE;

   if ((size_t)error > _eina_errors_count)
      return EINA_FALSE;

   if (_eina_errors[error - 1].string_allocated)
     {
        const char *tmp;

        if (!(tmp = eina_stringshare_add(msg)))
           return EINA_FALSE;

        eina_stringshare_del(_eina_errors[error - 1].string);
        _eina_errors[error - 1].string = tmp;
        return EINA_TRUE;
     }

   _eina_errors[error - 1].string = msg;
   return EINA_TRUE;
}

EAPI const char *
eina_error_msg_get(Eina_Error error)
{
   if (!EINA_ERROR_REGISTERED_CHECK(error))
     {
        const char unknown_prefix[] = "Unknown error ";
        const char *msg;

        /* original behavior of this function did not return strings
         * for unknown errors, so skip 0 ("Success") and
         * "Unknown error $N".
         */
        if (error == 0) return NULL;

#ifndef EINA_HAVE_THREADS
        msg = strerror(error);
        if (strncmp(msg, unknown_prefix, sizeof(unknown_prefix) -1) == 0)
          msg = NULL;
#else /* EINA_HAVE_THREADS */
        /* strerror() is not thread safe, so use a local buffer with
         * strerror_r() and cache resolved strings in a hash so we can
         * return the stringshared refernece.
         */
        if (eina_spinlock_take(&_eina_errno_msgs_lock) != EINA_LOCK_SUCCEED)
          {
             EINA_SAFETY_ERROR("could not take spinlock for errno messages hash!");
             return NULL;
          }
        msg = eina_hash_find(_eina_errno_msgs, &error);
        eina_spinlock_release(&_eina_errno_msgs_lock);

        if (!msg)
          {
             char buf[256] = "";
             const char *str = NULL;

#ifdef HAVE_STRERROR_R
# ifndef STRERROR_R_CHAR_P
             int ret;

             ret = strerror_r(error, buf, sizeof(buf)); /* XSI */
             if (ret == 0)
               str = buf;
             else if (ret == EINVAL)
               return NULL;
# else /* STRERROR_R_CHAR_P */
             str = strerror_r(error, buf, sizeof(buf)); /* GNU */
# endif /* ! STRERROR_R_CHAR_P */
#else
              /* not so good fallback. Usually strerror(err) will
               * return a const string if a known error (what we use),
               * and will return a pointer to a global modified string
               * formatted with "Unknown error XXXX".. which we just
               * ignore... so while it's not super-correct, this
               * should work well.
               */
             eina_strlcpy(buf, strerror(error), sizeof(buf));
             str = buf;
#endif /* HAVE_STRERROR_R */

             if (!str)
               EINA_SAFETY_ERROR("strerror_r() failed");
             else
               {
                  if (strncmp(str, unknown_prefix, sizeof(unknown_prefix) -1) == 0)
                    msg = NULL;
                  else
                    {
                       msg = eina_stringshare_add(str);
                       if (eina_spinlock_take(&_eina_errno_msgs_lock) != EINA_LOCK_SUCCEED)
                         {
                            EINA_SAFETY_ERROR("could not take spinlock for errno messages hash!");
                            return NULL;
                         }
                       eina_hash_add(_eina_errno_msgs, &error, msg);
                       eina_spinlock_release(&_eina_errno_msgs_lock);
                    }
               }
          }
#endif
        return msg;
     }

   error = EINA_ERROR_TO_INDEX(error);

   if (error < 1)
      return NULL;

   if ((size_t)error > _eina_errors_count)
      return NULL;

   return _eina_errors[error - 1].string;
}

EAPI Eina_Error
eina_error_get(void)
{
   if (eina_main_loop_is())
     return _eina_last_error;

   return (Eina_Error)(uintptr_t) eina_tls_get(_eina_last_key);
}

EAPI void
eina_error_set(Eina_Error err)
{
   if (eina_main_loop_is())
     _eina_last_error = err;
   else
     eina_tls_set(_eina_last_key, (void*)(uintptr_t) err);
}

EAPI Eina_Error
eina_error_find(const char *msg)
{
   size_t i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, 0);

   for (i = 0; i < _eina_errors_count; i++)
     {
        if (_eina_errors[i].string_allocated)
          {
             if (_eina_errors[i].string == msg)
               return EINA_ERROR_FROM_INDEX(i + 1);
          }
        if (!strcmp(_eina_errors[i].string, msg))
          return EINA_ERROR_FROM_INDEX(i + 1);
     }

   /* not bothering to lookup errno.h as we don't have a "maximum
    * error", thus we'd need to loop up to some arbitrary constant and
    * keep comparing if strerror() returns something meaningful.
    */

   return 0;
}
