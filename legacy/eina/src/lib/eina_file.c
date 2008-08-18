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

#define _GNU_SOURCE
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "eina_file.h"
#include "eina_private.h"

/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/

/**
 * Lis all files on the directory calling the function for every file found
 * @param dir The directory name
 * @param recursive Iterate recursively in the directory
 * @param cb The callback to be called
 * @param data The data to pass to the callback
 */
EAPI Eina_Bool
eina_file_dir_list(const char *dir, Eina_Bool recursive, Eina_File_Dir_List_Cb cb, void *data)
{
	struct dirent *de;
	DIR *d;

	if (!cb) return EINA_FALSE;

	d = opendir(dir);
	if (!d) return EINA_FALSE;

	while ((de = readdir(d)))
	{
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;

		cb(de->d_name, dir, data);
		/* d_type is only available on linux and bsd (_BSD_SOURCE) */

		if (recursive == EINA_TRUE) {
			char *path;

			path = alloca(strlen(dir) + strlen(de->d_name) + 2);
			strcpy(path, dir);
			strcat(path, "/");
			strcat(path, de->d_name);

			if (de->d_type == DT_UNKNOWN) {
				struct stat st;

				if (stat(path, &st))
					continue ;

				if (!S_ISDIR(st.st_mode))
					continue ;
			} else if (de->d_type != DT_DIR) {
				continue ;
			}

			eina_file_dir_list(path, recursive, cb, data);
		}
	}

	closedir(d);

	return EINA_TRUE;
}
