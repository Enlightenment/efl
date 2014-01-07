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

#ifndef EIO_INLINE_HELPER_H__
# define EIO_INLINE_HELPER_H__

/**
 * @addtogroup Eio_Helper
 *
 * @{
 */

/**
 * @brief Return last access time to a file
 * @param st The stat buffer as given by eio_file_stat callback.
 * @return last access time.
 *
 * This function return the st_atime field, last access time, as double like all EFL time call.
 */
static inline double
eio_file_atime(const Eina_Stat *st)
{
   if (!st) return 0.0;
   return (double) st->atime;
}

/**
 * @brief Return last modification time of a file
 * @param st The stat buffer as given by eio_file_stat callback.
 * @return last modification time.
 *
 * This function return the st_mtime field, last modification time, as double like all EFL time call.
 */
static inline double
eio_file_mtime(const Eina_Stat *st)
{
   if (!st) return 0.0;
   return (double) st->mtime;
}

/**
 * @brief Return file length.
 * @param st The stat buffer as given by eio_file_stat callback.
 * @return the length of a file.
 *
 * This function is just an accessor to st_size and return the file length.
 */
static inline long long
eio_file_size(const Eina_Stat *st)
{
   if (!st) return 0;
   return st->size;
}

/**
 * @brief Return if path is a directory.
 * @param st The stat buffer as given by eio_file_stat callback.
 * @return EINA_TRUE if the path is a directory.
 *
 * This function tell you if the stated path is a directory or not.
 */
static inline Eina_Bool
eio_file_is_dir(const Eina_Stat *st)
{
   if (!st) return EINA_FALSE;
   return (S_ISDIR(st->mode)) ? EINA_TRUE : EINA_FALSE;
}

/**
 * @brief Return if path is a length.
 * @param st The stat buffer as given by eio_file_stat callback.
 * @return EINA_TRUE if the path is a length.
 *
 * This function tell you if the stated path is a length or not.
 */
static inline Eina_Bool
eio_file_is_lnk(const Eina_Stat *st)
{
   if (!st) return EINA_FALSE;
   return (S_ISLNK(st->mode)) ? EINA_TRUE : EINA_FALSE;
}

/**
 * @}
 */

#endif
