#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#undef alloca
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

#include <stdio.h>
#include <string.h>

#include "ecore_file_private.h"

static Eina_List *__ecore_file_path_bin = NULL;

static Eina_List *_ecore_file_path_from_env(const char *env);

void
ecore_file_path_init(void)
{
   __ecore_file_path_bin = _ecore_file_path_from_env("PATH");
}

void
ecore_file_path_shutdown(void)
{
   char *dir;

   EINA_LIST_FREE(__ecore_file_path_bin, dir)
     eina_stringshare_del(dir);
}

Eina_List *
_ecore_file_path_from_env(const char *env)
{
   Eina_List *path = NULL;
   char *env_tmp, *env_path, *p, *last;

   env_tmp = getenv(env);
   if (!env_tmp)
     return path;

   env_path = alloca(sizeof(char) * strlen(env_tmp) + 1);
   memset(env_path, 0, strlen(env_tmp));
   strcpy(env_path, env_tmp);
   last = env_path;
   for (p = env_path; *p; p++)
     {
        if (*p == ':')
          *p = '\0';

        if (!*p)
          {
             if (!ecore_file_path_dir_exists(last))
               path = eina_list_append(path, eina_stringshare_add(last));
             last = p + 1;
          }
     }
   if (p > last)
     path = eina_list_append(path, eina_stringshare_add(last));

   return path;
}

/**
 * @addtogroup Ecore_File_Group Ecore_File - Files and directories convenience functions
 *
 * @{
 */

/**
 * @brief Check if the given directory is in PATH.
 *
 * @param The name of the directory to search in PATH.
 * @return EINA_TRUE if the directory exist in PATH, EINA_FALSE otherwise.
 *
 * This function checks if @p in_dir is in the environment variable
 * PATH. If @p in_dir is @c NULL, or if PATH is empty, or @p in_dir is
 * not in PATH, the function returns EINA_FALSE, otherwise it returns
 * EINA_TRUE.
 */
EAPI Eina_Bool
ecore_file_path_dir_exists(const char *in_dir)
{
   Eina_List *l;
   char *dir;

   if (!in_dir)
     return EINA_FALSE;

   if (!__ecore_file_path_bin) return EINA_FALSE;
   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
        if (strcmp(dir, in_dir))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

/**
 * @brief Check if the given application is installed.
 *
 * @param  exe The name of the application
 * @return EINA_TRUE if the exe is in PATH and is executable,
 * EINA_FALSE otherwise.
 *
 *
 * This function checks if @p exe exists in PATH and is executable. If
 * @p exe is @c NULL or is not executable, the function returns
 * EINA_FALSE, otherwise it returns EINA_TRUE.
 */
EAPI Eina_Bool
ecore_file_app_installed(const char *exe)
{
   Eina_List *l;
   char *dir;
   char  buf[PATH_MAX];

   if (!exe) return EINA_FALSE;
   if (ecore_file_can_exec(exe)) return EINA_TRUE;

   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
        if (ecore_file_can_exec(buf))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

/**
 * @brief Get a list of all the applications installed on the system.
 *
 * @return An Eina_List containing all the executable files in the
 * system.
 *
 * This function returns a list of allocated strings of all the
 * executable files. If no files are found, the function returns
 * @c NULL. When not needed anymore, the element of the list must be
 * freed.
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

/**
 * @}
 */
