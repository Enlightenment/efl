#ifndef EINA_SLSTR_H_
#define EINA_SLSTR_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_tmpstr.h"
#include "eina_strbuf.h"
#include "eina_stringshare.h"
#include "eina_slice.h"

/**
 * @addtogroup Eina_Slstr Short lived strings
 * @ingroup Eina
 *
 * @brief API for short lived strings (thread- and scope-local)
 *
 * This set of APIs provide a convenience feature to create and return strings
 * that are meant to be consumed in the local scope of the calling code block.
 * The lifecycle of those strings is bound to the loop of the current thread
 * or until the clear function is called explicitly.
 *
 * These strings will be automatically deleted.
 *
 * These functions shall return NULL only if out of memory.
 *
 * Do not call free or any similar function on a string created with this API!
 *
 * @since 1.19
 */

typedef const char Eina_Slstr;

/**
 * @brief Create a new short lived string by duplicating another string.
 *
 * @param[in] string An existing string, it will be copied.
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * char local[200];
 * sprintf(local, "Hello %d", value);
 * return eina_slstr_copy_new(local);
 * @endcode
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_copy_new(const char *string);

/**
 * @brief Create a new short lived string by taking ownership of a string.
 *
 * @param[in] string An existing string. It will not be duplicated.
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * char *local = strdup("Hello");
 * return eina_slstr_steal_new(local);
 * @endcode
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_steal_new(char *string);

/**
 * @brief Create a new short lived string by taking ownership of a stringshare.
 *
 * @param[in] string An existing stringshare, one reference belongs to this slstr.
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * Eina_Stringshare *local = eina_stringshare_add("Hello");
 * return eina_slstr_stringshare_new(local);
 * @endcode
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_stringshare_new(Eina_Stringshare *string);

/**
 * @brief Create a new short lived string by taking ownership of a tmpstr.
 *
 * @param[in] string An existing tmpstr, it will be freed later.
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * Eina_Tmpstr *local = eina_tmpstr_add("Hello");
 * return eina_slstr_tmpstr_new(local);
 * @endcode
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_tmpstr_new(Eina_Tmpstr *string);

/**
 * @brief Create a new short lived string by taking ownership of a strbuf.
 *
 * @param[in] string An existing strbuf, that will be released (i.e.. steal + free).
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * Eina_Strbuf *local = eina_strbuf_new();
 * eina_strbuf_append(local, "Hello");
 * eina_strbuf_append(local, " world");
 * return eina_slstr_strbuf_new(local);
 * @endcode
 *
 * @note Use eina_slstr_steal_new() if the strbuf will be used after this call.
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_strbuf_new(Eina_Strbuf *string);

/**
 * @brief Create a new short lived string using sprintf.
 *
 * @param[in] fmt Format string for printf
 * @param[in] args List of format parameters for printf
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * @since 1.19
 */
EAPI Eina_Slstr *
eina_slstr_vasprintf_new(const char *fmt, va_list args);

/**
 * @brief Create a new short lived string using sprintf.
 *
 * @param[in] fmt Format string for printf
 * @param[in] args List of format parameters for printf
 * @return A new Eina_Slstr or NULL if out of memory.
 *
 * Usage example:
 * @code
 * return eina_slstr_printf("Hello world %d!", 42);
 * @endcode
 *
 * @since 1.19
 */
static inline Eina_Slstr *
eina_slstr_printf(const char *fmt, ...)
{
   Eina_Slstr *str;
   va_list args;

   va_start(args, fmt);
   str = eina_slstr_vasprintf_new(fmt, args);
   va_end(args);

   return str;
}

#ifdef EINA_SLSTR_INTERNAL
/**
 * @brief Internal function to clear the strings.
 *
 * This internal function will be called by the local thread's loop to free
 * all the strings. Do not call this function unless you are absolutely certain
 * that no string in the queue will be used after this point.
 *
 * @since 1.19
 */
EAPI void
eina_slstr_local_clear(void);
#endif

#endif
