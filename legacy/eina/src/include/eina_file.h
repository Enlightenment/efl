/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
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

#ifndef EINA_FILE_H_
#define EINA_FILE_H_

#include <limits.h>

#include "eina_types.h"
#include "eina_array.h"
#include "eina_iterator.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_File_Group File
 *
 * @{
 */

/**
 * @typedef Eina_File_Direct_Info
 * A typedef to #_Eina_File_Direct_Info.
 */
typedef struct _Eina_File_Direct_Info Eina_File_Direct_Info;

/**
 * @typedef Eina_File_Dir_List_Cb
 * Type for a callback to be called when iterating over the files of a
 * directory.
 */
typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);

/**
 * @typedef Eina_File_Type
 * file type in Eina_File_Direct_Info.
 */
typedef enum {
  EINA_FILE_UNKNOWN, /**< Unknown file type. */
  EINA_FILE_FIFO,    /**< Named pipe (FIFO) type (unused on Windows). */
  EINA_FILE_CHR,     /**< Character device type (unused on Windows). */
  EINA_FILE_DIR,     /**< Directory type. */
  EINA_FILE_BLK,     /**< Block device type (unused on Windows). */
  EINA_FILE_REG,     /**< Regular file type. */
  EINA_FILE_LNK,     /**< Symbolic link type. */
  EINA_FILE_SOCK,    /**< UNIX domain socket type (unused on Windows). */
  EINA_FILE_WHT      /**< Whiteout file type (unused on Windows). */
} Eina_File_Type;

/* Why do this? Well PATH_MAX may vary from when eina itself is compiled
 * to when the app using eina is compiled. exposing the path buffer below
 * cant safely and portably vary based on how/when you compile. it should
 * always be the same for both eina inside AND for apps outside that use eina
 * so define this to 8192 - most PATH_MAX values are like 4096 or 1024 (with
 * windows i think being 260), so 8192 should cover almost all cases. there
 * is a possibility that PATH_MAX could be more than 8192. if anyone spots
 * a path_max that is bigger - let us know, but, for now we will assume
 * it never happens */
#define EINA_PATH_MAX 8192
/**
 * @struct _Eina_File_Direct_Info
 * A structure to store informations of a path.
 */
struct _Eina_File_Direct_Info
{
   size_t               path_length; /**< size of the whole path */
   size_t               name_length; /**< size of the filename/basename component */
   size_t               name_start; /**< where the filename/basename component starts */
   Eina_File_Type       type; /**< file type */
   char                 path[EINA_PATH_MAX]; /**< the path */
};

/**
 * @def EINA_FILE_DIR_LIST_CB
 * @brief cast to an #Eina_File_Dir_List_Cb.
 *
 * @param function The function to cast.
 *
 * This macro casts @p function to Eina_File_Dir_List_Cb.
 */
#define EINA_FILE_DIR_LIST_CB(function) ((Eina_File_Dir_List_Cb)function)

EAPI Eina_Bool eina_file_dir_list(const char           *dir,
                                  Eina_Bool             recursive,
                                  Eina_File_Dir_List_Cb cb,
                                  void                 *data) EINA_ARG_NONNULL(1, 3);
EAPI Eina_Array    *eina_file_split(char *path) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
EAPI Eina_Iterator *eina_file_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
EAPI Eina_Iterator *eina_file_stat_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;
EAPI Eina_Iterator *eina_file_direct_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_FILE_H_ */
