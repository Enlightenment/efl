/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Vincent Torri
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#ifndef _WIN32
# define _GNU_SOURCE
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <dirent.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include <Evil.h>
#endif /* _WIN2 */

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#ifndef _WIN32
# define PATH_DELIM '/'
#else
# define PATH_DELIM '\\'
#endif

#include "eina_file.h"
#include "eina_private.h"

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/

/**
 * List all files on the directory calling the function for every file found
 * @param dir The directory name
 * @param recursive Iterate recursively in the directory
 * @param cb The callback to be called
 * @param data The data to pass to the callback
 */
EAPI Eina_Bool
eina_file_dir_list(const char *dir, Eina_Bool recursive, Eina_File_Dir_List_Cb cb, void *data)
{
#ifndef _WIN32
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
#else
	WIN32_FIND_DATA file;
	HANDLE          hSearch;
	char           *new_dir;
	TCHAR          *tdir;
	int             length_dir;

	if (!cb) return EINA_FALSE;
	if (!dir || (*dir == '\0')) return EINA_FALSE;

	length_dir = strlen(dir);
	new_dir = (char *)alloca(length_dir + 5);
	if (!new_dir) return EINA_FALSE;

	memcpy(new_dir, dir, length_dir);
	memcpy(new_dir + length_dir, "/*.*", 5);

#ifdef UNICODE
	tdir =  evil_char_to_wchar(new_dir);
#else
	tdir = new_dir;
#endif /* ! UNICODE */
	hSearch = FindFirstFile(tdir, &file);
#ifdef UNICODE
	free(tdir);
#endif /* UNICODE */

	if (hSearch == INVALID_HANDLE_VALUE) return EINA_FALSE;

	do
	{
		char *filename;

#ifdef UNICODE
		filename = evil_wchar_to_char(file.cFileName);
#else
		filename = file.cFileName;
#endif /* ! UNICODE */
		if (!strcmp(filename, ".") || !strcmp(filename, ".."))
			continue;

		cb(filename, dir, data);

		if (recursive == EINA_TRUE) {
			char *path;

			path = alloca(strlen(dir) + strlen(filename) + 2);
			strcpy(path, dir);
			strcat(path, "/");
			strcat(path, filename);

			if (!(file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue ;

			eina_file_dir_list(path, recursive, cb, data);
		}
#ifdef UNICODE
		free(filename);
#endif /* UNICODE */

	} while (FindNextFile(hSearch, &file));
	FindClose(hSearch);
#endif /* _WIN32 */

	return EINA_TRUE;
}

/**
 *
 */
EAPI Eina_Array *
eina_file_split(char *path)
{
	Eina_Array *ea;
	char *current;
	int length;

	if (!path) return NULL;

	ea = eina_array_new(16);

	if (!ea) return NULL;

	for (current = strchr(path, PATH_DELIM);
	     current != NULL;
	     path = current + 1, current = strchr(path, PATH_DELIM))
	{
		length = current - path;

		if (length <= 0) continue ;

		eina_array_push(ea, path);
		*current = '\0';
	}

	if (*path != '\0')
		eina_array_push(ea, path);

	return ea;
}
