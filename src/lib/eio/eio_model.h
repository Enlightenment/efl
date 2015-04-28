/**
 * @page emodel_eio_main Emodel_Eio
 *
 * @date 2014 (created)
 *
 * @brief Emodel_Eio Library Public API Calls
 *
 * @section toc Table of Contents
 *
 * @li @ref emodel_eio_main_intro
 *
 * @section eo_main_intro Introduction
 *
 * This module targets file operations using Emodel.

 *
 * Recommended reading:
 *
 * @li @ref Emodel
 * @li @ref Eo
 * @li @ref Eina
 *
 * @defgroup Emodel_Eio EIO implementation wrapper for Emodel
 *
 * @addtogroup Emodel_Eio
 * @{
 */

#ifndef _EMODEL_EIO_H
#define _EMODEL_EIO_H

#include <Eo.h>
#include <Efl.h>
#include <Eio.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EIO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EIO_BUILD */
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

#include <eio_model.eo.h>

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif //_EMODEL_EIO_H
