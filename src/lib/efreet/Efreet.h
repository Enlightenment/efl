#ifndef EFREET_H
#define EFREET_H

/**
 * @internal
 * @file Efreet.h
 *
 * @brief The file that must be included by any project wishing to use
 *        Efreet. Efreet.h provides all the necessary headers and includes to
 *        work with Efreet.
 */

/**
 * @internal
 * @defgroup Efreet_Group Efreet
 * @ingroup EFL_Group
 *
 * @page efreet_main Efreet
 *
 * @section toc Table of Contents
 *
 * @li @ref efreet_main_intro
 * @li @ref efreet_main_next_steps
 *
 * @section efreet_main_intro Introduction
 *
 * @brief Efreet is a library designed to help applications work with several of the
 *        Freedesktop.org standards regarding Icons, Desktop files, and Menus. On the
 *        same lines it implements the following specifications:
 *
 * @li XDG Base Directory Specification
 * @li Icon Theme Specification
 * @li Desktop Entry Specification
 * @li Desktop Menu Specification
 * @li FDO URI Specification
 * @li Shared Mime Info Specification
 * @li Trash Specification
 *
 * @section efreet_main_next_steps Next Steps
 *
 * After you understood what Efreet is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Efreet_Base_Group for base directory specification (XDG variables).
 * @li @ref Efreet_Desktop to access .desktop files
 * @li @ref Efreet_Menu to access menus of .desktop files
 * @li @ref Efreet_Mime_Group to identify files based on extension or header.
 * @li @ref Efreet_Trash_Group to access file trash implementation.
 * @li @ref Efreet_Ini for parsing INI-like key-value files.
 * @li @ref Efreet_Uri for URI parsing and encoding.
 * @li @ref Efreet_Utils_Group general utilities.
 *
 * @{
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
#define EFREET_VERSION_MINOR 8
   
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
 * @brief Initializes the Efreet system.
 *
 * @return The value > @c 0 if the initialization is successful, otherwise @c 0
 */
EAPI int efreet_init(void);

/**
 * @brief Shuts down Efreet if a balanced number of init/shutdown calls have been made.
 *
 * @return The number of times the init function has been called minus the
 *         corresponding init call
 */
EAPI int efreet_shutdown(void);

/**
 * @brief Resets the language dependent variables and resets the language dependent
 *        caches. This must be called whenever the locale is changed.
 * @since 1.7
 */
EAPI void efreet_lang_reset(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
