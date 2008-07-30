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

