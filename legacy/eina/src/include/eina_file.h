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

#include "eina_types.h"

/**
 * @defgroup File_Group Memory File
 * @{
 */

typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);

EAPI void eina_file_dir_list(const char *dir, int recursive, Eina_File_Dir_List_Cb cb, void *data);
EAPI void eina_file_path_nth_get(const char *path, int n, char **left, char **right);

/** @} */

#endif /*EINA_FILE_H_*/
