#ifndef EFREET_H
#define EFREET_H

/**
 * @file Efreet.h
 * @brief The file that must be included by any project wishing to use
 * Efreet. Efreet.h provides all of the necessary headers and includes to
 * work with Efreet.
 */

/**
 * @mainpage The Efreet Library
 *
 * @section intro Introduction
 *
 * Efreet is a library designed to help apps work several of the
 * Freedesktop.org standards regarding Icons, Desktop files and Menus. To
 * that end it implements the following specifications:
 *
 * @li XDG Base Directory Specification
 * @li Icon Theme Specification
 * @li Desktop Entry Specification
 * @li Desktop Menu Specification
 * @li FDO URI Specification
 * @li Shared Mime Info Specification
 * @li Trash Specification
 */

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFREET_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFREET_BUILD */
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define EFREET_VERSION_MAJOR 1
#define EFREET_VERSION_MINOR 0
   
   typedef struct _Efreet_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Efreet_Version;
   
   EAPI extern Efreet_Version *efreet_version;
   
#include "efreet_base.h"
#include "efreet_ini.h"
#include "efreet_icon.h"
#include "efreet_desktop.h"
#include "efreet_menu.h"
#include "efreet_utils.h"
#include "efreet_uri.h"

/**
 * @return Returns > 0 if the initialization was successful, 0 otherwise
 * @brief Initializes the Efreet system
 */
EAPI int efreet_init(void);

/**
 * @return Returns the number of times the init function as been called
 * minus the corresponding init call.
 * @brief Shuts down Efreet if a balanced number of init/shutdown calls have
 * been made
 */
EAPI int efreet_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
