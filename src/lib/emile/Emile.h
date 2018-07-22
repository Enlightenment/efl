/* EMILE - EFL serialization, compression and crypto library.
 * Copyright (C) 2013 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@samsung.com>
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
#ifndef EMILE_H_
#define EMILE_H_

#include <Eina.h>
#include <interfaces/efl_gfx_types.eot.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
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

/**
 * @brief Emile serialization, compression and ciphering public API calls.
 *
 * These routines are used for Emile Library interaction
 *
 * @date 2013 (created)
 */

/**
 * @file Emile.h
 * @brief The file that provide the Emile function
 *
 * This header provides the Emile management functions.
 */

/**
 * @defgroup Emile_Group Top level functions
 * @ingroup Emile
 * Function that affect Emile as a whole.
 *
 * @{
 */

/**
 * Initialize the Emile library
 *
 * The first time this function is called, it will perform all the internal
 * initialization required for the library to function properly and
 * increment the initialization counter. Any subsequent call only
 * increment this counter and return its new value, so it's safe to call
 * this function more than once.
 *
 * @return The new init count. Will be 0 if initialization failed.
 *
 * @since 1.14
 */
EAPI int emile_init(void);

/**
 * Shut down the Emile library
 *
 * If emile_init() was called more than once for the running application,
 * emile_shutdown() will decrement the initialization counter and return its
 * new value, without doing anything else. When the counter reaches 0, all
 * of the internal elements will be shutdown and any memory used freed.
 *
 * @return The new init count.
 * @since 1.14
 */
EAPI int emile_shutdown(void);

/**
 * @}
 */

#include "emile_cipher.h"
#include "emile_compress.h"
#include "emile_image.h"
#include "emile_base64.h"

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
