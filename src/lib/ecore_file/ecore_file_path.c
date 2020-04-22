#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdlib.h>
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
        if (!strcmp(dir, in_dir))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_file_app_installed(const char *exe)
{
   Eina_List *l;
   char *dir;
   char  buf[PATH_MAX];

   if (!exe) return EINA_FALSE;
   if (((!strncmp(exe, "/", 1)) ||
        (!strncmp(exe, "./", 2)) ||
        (!strncmp(exe, "../", 3))) &&
       ecore_file_can_exec(exe)) return EINA_TRUE;

   EINA_LIST_FOREACH(__ecore_file_path_bin, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
        if (ecore_file_can_exec(buf))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

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
