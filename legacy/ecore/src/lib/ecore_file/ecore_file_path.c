/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_file_private.h"

static int init = 0;
static Ecore_List *__ecore_file_path_bin = NULL;

static Ecore_List *_ecore_file_path_from_env(const char *env);

int
ecore_file_path_init(void)
{
   if (++init != 1) return init;
   __ecore_file_path_bin = _ecore_file_path_from_env("PATH");
   ecore_list_free_cb_set(__ecore_file_path_bin, free);
   return init;
}

int
ecore_file_path_shutdown(void)
{
   if (--init != 0) return init;
   ecore_list_destroy(__ecore_file_path_bin);
   __ecore_file_path_bin = NULL;
   return init;
}

Ecore_List *
_ecore_file_path_from_env(const char *env)
{
   Ecore_List *path;
   char *env_path, *p, *last;

   path = ecore_list_new();

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
	       ecore_list_append(path, strdup(last));
             last = p + 1;
          }
     }
   if (p > last)
     ecore_list_append(path, strdup(last));

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
   char *dir;

   if (!__ecore_file_path_bin) return 0;
   ecore_list_first_goto(__ecore_file_path_bin);
   while ((dir = ecore_list_next(__ecore_file_path_bin)) != NULL)
     {
	if (!strcmp(dir, in_dir)) return 1;
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
   char *dir;
   char  buf[PATH_MAX];

   if (!exe) return 0;
   if (ecore_file_can_exec(exe)) return 1;

   ecore_list_first_goto(__ecore_file_path_bin);
   while ((dir = ecore_list_next(__ecore_file_path_bin)) != NULL)
     {
	snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
	if (ecore_file_can_exec(buf)) return 1;
     }
   return 0;
}

/**
 * Get a list of all the applications installed on the system
 * @return An Ecore_List containing all the executable files in the system
 */
EAPI Ecore_List *
ecore_file_app_list(void)
{
   Ecore_List *list, *files;
   char  buf[PATH_MAX], *dir, *exe;
   
   list = ecore_list_new();
   if (!list) return NULL;
   ecore_list_free_cb_set(list, free);
   ecore_list_first_goto(__ecore_file_path_bin);
   while ((dir = ecore_list_next(__ecore_file_path_bin)) != NULL)
     {
	files = ecore_file_ls(dir);
	if (files)
	  {
	     ecore_list_first_goto(files);
	     while ((exe = ecore_list_next(files)) != NULL)
	       {
		  snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
		  if ((ecore_file_can_exec(buf)) &&
		      (!ecore_file_is_dir(buf)))
		    ecore_list_append(list, strdup(buf));
	       }
	     ecore_list_destroy(files);
	  }
     }
   return list;
}
