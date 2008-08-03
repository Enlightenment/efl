/* vim: set sw=4 ts=4 sts=4 et: */
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

#ifdef EAPI
#undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
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

#include "efreet_base.h"
#include "efreet_ini.h"
#include "efreet_icon.h"
#include "efreet_desktop.h"
#include "efreet_menu.h"
#include "efreet_utils.h"
#include "efreet_uri.h"

EAPI int efreet_init(void);
EAPI int efreet_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
