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
#include "eina_array.h"

/**
 * @defgroup File_Group Memory File
 * @{
 */

typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);
#define EINA_FILE_DIR_LIST_CB(Function) ((Eina_File_Dir_List_Cb)Function)

EAPI Eina_Bool eina_file_dir_list(const char *dir, Eina_Bool recursive, Eina_File_Dir_List_Cb cb, void *data);
EAPI Eina_Array *eina_file_split(char *path);

/** @} */

#endif /*EINA_FILE_H_*/
