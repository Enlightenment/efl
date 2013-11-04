#ifndef EFREET_H
#define EFREET_H

/**
 * @file Efreet.h
 * @brief The file that must be included by any project wishing to use
 * Efreet. Efreet.h provides all of the necessary headers and includes to
 * work with Efreet.
 */

/**
 * @page efreet_main Efreet
 *
 * @section toc Table of Contents
 *
 * @li @ref efreet_main_intro
 * @li @ref efreet_main_compiling
 * @li @ref efreet_main_next_steps
 *
 * @section efreet_main_intro Introduction
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
 *
 * @section efreet_main_compiling How to compile
 *
 * Efreet is a library your application links to. The procedure for
 * this is very simple. You simply have to compile your application
 * with the appropriate compiler flags that the @c pkg-config script
 * outputs. Mime and Thrash are separated modules. For example, to
 * compile with mime support:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags efreet efreet-mime`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs efreet efreet-mime`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section efreet_main_next_steps Next Steps
 *
 * After you understood what Efreet is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Efreet_Base for base directory specification (XDG variables).
 * @li @ref Efreet_Desktop to access .desktop files
 * @li @ref Efreet_Menu to access menus of .desktop files
 * @li @ref Efreet_Mime to identify files based on extension or header.
 * @li @ref Efreet_Trash to access file trash implementation.
 * @li @ref Efreet_Ini for parsing INI-like key-value files.
 * @li @ref Efreet_Uri for URI parsing and encoding.
 * @li @ref Efreet_Utils general utilities.
 *
 */

#include <Eina.h>
#include <Efl_Config.h>

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

#define EFREET_VERSION_MAJOR EFL_VERSION_MAJOR
#define EFREET_VERSION_MINOR EFL_VERSION_MINOR
   
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
 * @return Value > @c 0 if the initialization was successful, @c 0 otherwise.
 * @brief Initializes the Efreet system
 */
EAPI int efreet_init(void);

/**
 * @return The number of times the init function has been called minus the
 * corresponding init call.
 * @brief Shuts down Efreet if a balanced number of init/shutdown calls have
 * been made
 */
EAPI int efreet_shutdown(void);

/**
 * @brief Resets language dependent variables and resets language dependent
 * caches This must be called whenever the locale is changed.
 * @since 1.7
 */
EAPI void efreet_lang_reset(void);

#ifdef __cplusplus
}
#endif

#endif
