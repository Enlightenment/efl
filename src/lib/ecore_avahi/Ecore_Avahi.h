/**
   @brief Ecore Avahi integration Library Public API Calls

   These routines are used for integrating Avahi with Ecore main loop
 */

#ifndef _ECORE_AVAHI_H
# define _ECORE_AVAHI_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Ecore_Avahi Ecore main loop integration function.
 * @ingroup Ecore
 *
 * @{
 */

/**
 * @since 1.9
 */
typedef struct _Ecore_Avahi Ecore_Avahi; /**< A handle for an Avahi instance. */

/**
 * @brief Create an AvahiPoll context and integrate it within Ecore main loop.
 *
 * @return A handler that reference the AvahiPoll context
 * @since 1.9
 */
EAPI Ecore_Avahi *ecore_avahi_add(void);

/**
 * @brief Delete the specified handler of an AvahiPoll.
 *
 * @param handler The actual handler to destroy.
 * @since 1.9
 *
 * Be aware there should not be any reference still using that handler before
 * destroying it.
 */
EAPI void         ecore_avahi_del(Ecore_Avahi *handler);

/**
 * @brief Get the AvahiPoll structure to integrate with Ecore main loop.
 *
 * @param handler The handler to get the AvahiPoll structure from.
 * @return return the actual AvahiPoll structure to use with Avahi.
 * @since 1.9
 */
EAPI const void  *ecore_avahi_poll_get(Ecore_Avahi *handler); // return AvahiPoll

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
