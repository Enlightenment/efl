/* os dependent file code. for unix-y like fs's only for now */
/* if your os doesn't use unix-like fs starting with "/" for the root and */
/* the file path separator isn't "/" then you may need to help out by */
/* adding in a new set of functions here */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
/* get the casefold feature! */
#include <unistd.h>
#include <sys/param.h>

#include "evas_common_private.h"
#include "evas_private.h"

#ifdef _WIN32
# define EVAS_PATH_SEPARATOR "\\"
#else
# define EVAS_PATH_SEPARATOR "/"
#endif

char *
evas_file_path_join(const char *path, const char *end)
{
   char *res = NULL;
   size_t len;

   if ((!path) && (!end)) return NULL;
   if (!path) return strdup(end);
   if (!end) return strdup(path);
   len = strlen(path);
   len += strlen(end);
   len += strlen(EVAS_PATH_SEPARATOR);
   res = malloc(len + 1);
   if (!res) return NULL;
   strcpy(res, path);
   strcat(res, EVAS_PATH_SEPARATOR);
   strcat(res, end);
   return res;
}

int
evas_file_path_exists(const char *path)
{
   struct stat st;

   if (!stat(path, &st)) return 1;
   return 0;
}

int
evas_file_path_is_file(const char *path)
{
   struct stat st;

   if (stat(path, &st) == -1) return 0;
   if (S_ISREG(st.st_mode)) return 1;
   return 0;
}

int
evas_file_path_is_dir(const char *path)
{
   struct stat st;

   if (stat(path, &st) == -1) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

Eina_List *
evas_file_path_list(char *path, const char *match, int match_case)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *files = NULL;
   int flags;

   flags = EINA_FNMATCH_PATHNAME;
   if (!match_case)
     flags |= EINA_FNMATCH_CASEFOLD;
#if defined FNM_IGNORECASE
   if (!match_case)
     flags |= FNM_IGNORECASE;
#else
/*#warning "Your libc does not provide case-insensitive matching!"*/
#endif

   it = eina_file_direct_ls(path);
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (match)
          {
             if (eina_fnmatch(match, info->path + info->name_start, flags))
               files = eina_list_append(files, strdup(info->path + info->name_start));
          }
        else
          files = eina_list_append(files, strdup(info->path + info->name_start));
     }
   if (it) eina_iterator_free(it);
   return files;
}

DATA64
evas_file_modified_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (st.st_ctime > st.st_mtime) return (DATA64)st.st_ctime;
   else return (DATA64)st.st_mtime;
   return 0;
}

char *
evas_file_path_resolve(const char *file)
{
#if 0
   char buf[PATH_MAX], *buf2;
#endif

   return strdup(file);
#if 0
   if (!realpath(file, buf)) return NULL;
   buf2 = strdup(buf);
   return buf2;
#endif
}
