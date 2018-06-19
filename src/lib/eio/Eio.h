/* EIO - Core asynchronous input/output operation library.
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Gustavo Sverzut Barbieri <barbieri@gmail.com>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <smhouston88@gmail.com>
 *           Guillaume "kuri" Friloux <guillaume.friloux@asp64.com>
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

#ifndef EIO_H__
# define EIO_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>
#include <Efl_Config.h>

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
 * @typedef Eio_File
 * Generic asynchronous I/O reference.
 * @ingroup Eio
 */
typedef struct _Eio_File Eio_File;

typedef Eina_Bool (*Eio_Filter_Direct_Cb)(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "Eio_Legacy.h"
#endif
#ifdef EFL_EO_API_SUPPORT
#include "Eio_Eo.h"
#endif

/**
 * @brief get access time from a Eina_Stat
 * @param stat the structure to get the atime from
 * @return the last accessed time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 * @note some filesystem don't update that information.
 */
static inline double eio_file_atime(const Eina_Stat *stat);

/**
 * @brief get modification time from a Eina_Stat
 * @param stat the structure to get the mtime from
 * @return the last modification time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 */
static inline double eio_file_mtime(const Eina_Stat *stat);

/**
 * @brief get the size of the file described in Eina_Stat
 * @param stat the structure to get the size from
 * @return the size of the file
 */
static inline long long eio_file_size(const Eina_Stat *stat);

/**
 * @brief tell if the stated path was a directory or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_dir(const Eina_Stat *stat);

/**
 * @brief tell if the stated path was a link or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_lnk(const Eina_Stat *stat);

/**
 * @ingroup Eio
 *
 * @brief Set the polling interval to control the fallback monitor behavior
 * @param interval The interval (in seconds) to poll
 * @since 1.21
 */
EAPI void eio_monitoring_interval_set(double interval);

#include "eio_inline_helper.x"

#define EIO_VERSION_MAJOR EFL_VERSION_MAJOR
#define EIO_VERSION_MINOR EFL_VERSION_MINOR

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
