/**
   @brief Ecore Avahi integration Library Public API Calls.

   These routines are used for integrating Avahi with Ecore main loop.
 */

#ifndef _ECORE_AVAHI_H
# define _ECORE_AVAHI_H

#include <ecore_avahi_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Ecore_Avahi_Group Ecore_Avahi - Avahi main loop integration function.
 * @ingroup Ecore
 *
 * @{
 */

/**
 * @since 1.9
 */
typedef struct _Ecore_Avahi Ecore_Avahi; /**< A handle for an Avahi instance. */

/**
 * @brief Creates an AvahiPoll context and integrate it within Ecore main loop.
 *
 * @return A handler that reference the AvahiPoll context
 * @since 1.9
 */
ECORE_AVAHI_API Ecore_Avahi *ecore_avahi_add(void);

/**
 * @brief Deletes the specified handler of an AvahiPoll.
 *
 * @param handler The actual handler to destroy.
 * @since 1.9
 *
 * Be aware there should not be any reference still using that handler before
 * destroying it.
 */
ECORE_AVAHI_API void         ecore_avahi_del(Ecore_Avahi *handler);

/**
 * @brief Gets the AvahiPoll structure to integrate with Ecore main loop.
 *
 * @param handler The handler to get the AvahiPoll structure from.
 * @return return the actual AvahiPoll structure to use with Avahi.
 * @since 1.9
 */
ECORE_AVAHI_API const void  *ecore_avahi_poll_get(Ecore_Avahi *handler); // return AvahiPoll

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
