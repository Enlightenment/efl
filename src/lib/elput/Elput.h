#ifndef _ELPUT_H
# define _ELPUT_H

# ifdef EAPI
#  undef EAPI
# endif

# ifdef _MSC_VER
#  ifdef BUILDING_DLL
#   define EAPI __declspec(dllexport)
#  else // ifdef BUILDING_DLL
#   define EAPI __declspec(dllimport)
#  endif // ifdef BUILDING_DLL
# else // ifdef _MSC_VER
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else // if __GNUC__ >= 4
#    define EAPI
#   endif // if __GNUC__ >= 4
#  else // ifdef __GNUC__
#   define EAPI
#  endif // ifdef __GNUC__
# endif // ifdef _MSC_VER

# ifdef EFL_BETA_API_SUPPORT

/**
 * @file
 * @brief Ecore functions for dealing with libinput
 *
 * @defgroup Elput_Group Elput - libinput integration
 * @ingrup Ecore
 *
 * Elput provides a wrapper and functions for using libinput
 *
 * @li @ref Elput_Init_Group
 *
 */

/**
 * @defgroup Elput_Init_Group Library Init and Shutdown functions
 *
 * Functions that start and shutdown the Elput library
 */

/**
 * Initialize the Elput library
 *
 * @return The number of times the library has been initialized without being
 *         shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Elput_Init_Group
 * @since 1.18
 */
EAPI int elput_init(void);

/**
 * Shutdown the Elput library
 *
 * @return The number of times the library has been initialized without being
 *         shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Elput_Init_Group
 * @since 1.18
 */
EAPI int elput_shutdown(void);

# endif

# undef EAPI
# define EAPI

#endif
