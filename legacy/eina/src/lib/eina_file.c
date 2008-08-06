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

#include "eina_file.h"
#include "eina_private.h"
/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
/**
 * Lis all files on the directory calling the function for every file found
 * @param recursive Iterate recursively in the directory
 */
EAPI void eina_file_dir_list(const char *dir, int recursive, Eina_File_Dir_List_Cb cb, void *data)
{
	struct dirent *de;
	DIR *d;

	assert(cb);
	
	d = opendir(dir);
	if (!d)
		return;

	while ((de = readdir(d)))
	{
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;
		
		cb(de->d_name, dir, data);
		/* d_type is only available on linux and bsd (_BSD_SOURCE) */
		if ((recursive) && (de->d_type == DT_DIR))
		{
			char path[PATH_MAX];
			
			snprintf(path, PATH_MAX, "%s/%s", dir, de->d_name);
			eina_file_dir_list(path, recursive, cb, data);
		}
	}
	closedir(d);
}
/**
 * 
 */
EAPI void eina_file_path_nth_get(const char *path, int n, char **left, char **right)
{
	char *p;
	char *end;
	char *tmp;
	char *delim;
	int inc;
	int num = 0;
	
	if (!left && !right)
		return;
	
	if (n > 0)
	{
		p = (char *)path;
		inc = 1;
		end = (char *)path + strlen(path);
	}
	else
	{
		p = (char *)path + strlen(path);
		inc = -1;
		end = (char *)path;
	}
	for (tmp = p, delim = p; tmp != end && num != n; tmp += inc)
	{
		if (*tmp == '/')
		{
			num += inc;
			delim = tmp;
		}
	}
	if (left)
	{
		*left = strndup(path, delim - path + 1);
	}
	if (right)
	{
		*right = strdup(delim + 1);
	}
}

