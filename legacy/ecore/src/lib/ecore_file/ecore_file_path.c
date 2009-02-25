/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "ecore_file_private.h"

static int init = 0;
static Eina_List *__ecore_file_path_bin = NULL;

static Eina_List *_ecore_file_path_from_env(const char *env);

int
ecore_file_path_init(void)
{
   if (++init != 1) return init;
   __ecore_file_path_bin = _ecore_file_path_from_env("PATH");
   return init;
}

int
ecore_file_path_shutdown(void)
{
   char *dir;

   if (--init != 0) return init;
   EINA_LIST_FREE(__ecore_file_path_bin, dir)
     free(dir);
   return init;
}

Eina_List *
_ecore_file_path_from_env(const char *env)
{
   Eina_List *path = NULL;
   char *env_path, *p, *last;

   env_path = getenv(env);
   if (!env_path)
     return path;

   env_path = strdup(env_path);
   last = env_path;
   for (p = env_path; *p; p++)
     {
        if (*p == ':')
          *p = '\0';

        if (!*p)
          {
	     if (!ecore_file_path_dir_exists(last))
	       path = eina_list_append(path, strdup(last));
             last = p + 1;
          }
     }
   if (p > last)
     path = eina_list_append(path, strdup(last));

   free(env_path);
   return path;
}

/**
 * Check if the given directory is in PATH
 * @param The name of the directory to search in PATH
 * @return 1 if the directory exist in PATH, 0 otherwise
 */
EAPI int
ecore_file_path_dir_exists(const char *in_dir)
{
   Eina_List *l;
   char *dir;

   if (!__ecore_file_path_bin) return 0;
   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
	if (strcmp(dir, in_dir))
	  return 1;
     }

   return 0;
}

/**
 * Check if the given application is installed
 * @param  exe The name of the application
 * @return 1 if the exe is in PATH and is executable
 * 
 * This function check if the given name exist in PATH and is executable 
 */
EAPI int
ecore_file_app_installed(const char *exe)
{
   Eina_List *l;
   char *dir;
   char  buf[PATH_MAX];

   if (!exe) return 0;
   if (ecore_file_can_exec(exe)) return 1;

   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
	snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
	if (ecore_file_can_exec(buf))
	  return 1;
     }

   return 0;
}

/**
 * Get a list of all the applications installed on the system
 * @return An Eina_List containing all the executable files in the system
 */
EAPI Eina_List *
ecore_file_app_list(void)
{
   Eina_List *list = NULL;
   Eina_List *files;
   Eina_List *l;
   char  buf[PATH_MAX], *dir, *exe;
   
   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
	files = ecore_file_ls(dir);
	EINA_LIST_FREE(files, exe)
	       {
		  snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
		  if ((ecore_file_can_exec(buf)) &&
		      (!ecore_file_is_dir(buf)))
	       list = eina_list_append(list, strdup(buf));
	     free(exe);
	  }
     }

   return list;
}
