/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *	     Stephen "okra" Houston <unixtitan@gmail.com>
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

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>

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

/**
 * @defgroup Eio_Group Asynchronous Inout/Output library
 *
 * @brief A brief description
 *
 * A long description
 *
 * @{
 */

typedef enum _Eio_File_Op
{
  EIO_FILE_COPY,
  EIO_FILE_MOVE,
  EIO_DIR_COPY,
  EIO_DIR_MOVE
} Eio_File_Op;

typedef struct _Eio_File Eio_File;
typedef struct _Eio_Progress Eio_Progress;
typedef struct _Eio_Stat Eio_Stat;

typedef Eina_Bool (*Eio_Filter_Cb)(void *data, const char *file);
typedef void (*Eio_Main_Cb)(void *data, const char *file);

typedef Eina_Bool (*Eio_Filter_Direct_Cb)(void *data, const Eina_File_Direct_Info *info);
typedef void (*Eio_Main_Direct_Cb)(void *data, const Eina_File_Direct_Info *info);

typedef void (*Eio_Stat_Cb)(void *data, const struct stat *stat);
typedef void (*Eio_Progress_Cb)(void *data, const Eio_Progress *info);

typedef void (*Eio_Done_Cb)(void *data);
typedef void (*Eio_Error_Cb)(int error, void *data);

struct _Eio_Progress
{
   Eio_File_Op op;

   off_t current;
   off_t max;
   float percent;

   const char *source;
   const char *dest;
};

EAPI int eio_init(void);
EAPI int eio_shutdown(void);

EAPI Eio_File *eio_file_ls(const char *dir,
			   Eio_Filter_Cb filter_cb,
			   Eio_Main_Cb main_cb,
			   Eio_Done_Cb done_cb,
			   Eio_Error_Cb error_cb,
			   const void *data);

EAPI Eio_File *eio_file_direct_ls(const char *dir,
				  Eio_Filter_Direct_Cb filter_cb,
				  Eio_Main_Direct_Cb main_cb,
				  Eio_Done_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);

EAPI Eio_File *eio_file_direct_stat(const char *path,
				    Eio_Stat_Cb done_cb,
				    Eio_Error_Cb error_cb,
				    const void *data);

EAPI Eio_File *eio_file_unlink(const char *path,
			       Eio_Done_Cb done_cb,
			       Eio_Error_Cb error_cb,
			       const void *data);

EAPI Eio_File *eio_file_mkdir(const char *path,
			      mode_t mode,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI Eio_File *eio_file_move(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_file_copy(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_dir_move(const char *source,
			    const char *dest,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_copy(const char *source,
			    const char *dest,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_unlink(const char *path,
			      Eio_Progress_Cb progress_cb,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI Eina_Bool eio_file_cancel(Eio_File *ls);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif
