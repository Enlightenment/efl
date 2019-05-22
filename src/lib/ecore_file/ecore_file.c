#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#ifdef _WIN32
# include <direct.h>
# include <evil_private.h> /* mkdir realpath */
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_ATFILE_SOURCE
# include <dirent.h>
#endif

#include "ecore_file_private.h"

/*
 * FIXME: the following functions will certainly not work on Windows:
 * ecore_file_app_exe_get()
 * ecore_file_escape_name()
 */

int _ecore_file_log_dom = -1;
static int _ecore_file_init_count = 0;

static Eina_Bool
_ecore_file_stat(const char *file,
                 long long *mtime,
                 long long *size,
                 mode_t *mode,
                 Eina_Bool *is_dir,
                 Eina_Bool *is_reg)
{
   struct stat st;
#ifdef _WIN32
   /*
    * On Windows, stat() returns -1 is file is a path finishing with
    * a slash or blackslash
    * see https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
    * ("Return Value" section)
    *
    * so we ensure that file never finishes with \ or /
    */
   char f[MAX_PATH];
   size_t len;

   len = strlen(file);
   if ((len + 1) > MAX_PATH)
     return EINA_FALSE;

   memcpy(f, file, len + 1);
   if ((f[len - 1] == '/') || (f[len - 1] == '\\'))
     f[len - 1] = '\0';

   if (stat(f, &st) < 0)
     return EINA_FALSE;
#else
   if (stat(file, &st) < 0)
     return EINA_FALSE;
#endif

   if (mtime) *mtime = st.st_mtime;
   if (size) *size = st.st_size;
   if (mode) *mode = st.st_mode;
   if (is_dir) *is_dir = S_ISDIR(st.st_mode);
   if (is_reg) *is_reg = S_ISREG(st.st_mode);

   return EINA_TRUE;
}

EAPI int
ecore_file_init()
{
   if (++_ecore_file_init_count != 1)
     return _ecore_file_init_count;

   if (!ecore_init())
     return --_ecore_file_init_count;

   _ecore_file_log_dom = eina_log_domain_register
     ("ecore_file", ECORE_FILE_DEFAULT_LOG_COLOR);
   if(_ecore_file_log_dom < 0)
     {
       EINA_LOG_ERR("Impossible to create a log domain for the ecore file module.");
       return --_ecore_file_init_count;
     }
   ecore_file_path_init();
   ecore_file_monitor_init();
   ecore_file_download_init();

   /* FIXME: were the tests disabled for a good reason ? */

   /*
   if (!ecore_file_monitor_init())
     goto shutdown_ecore_file_path;

   if (!ecore_file_download_init())
     goto shutdown_ecore_file_monitor;
   */

   return _ecore_file_init_count;

   /*
 shutdown_ecore_file_monitor:
   ecore_file_monitor_shutdown();
 shutdown_ecore_file_path:
   ecore_file_path_shutdown();

   return --_ecore_file_init_count;
   */
}

EAPI int
ecore_file_shutdown()
{
   if (--_ecore_file_init_count != 0)
     return _ecore_file_init_count;

   ecore_file_download_shutdown();
   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();

   eina_log_domain_unregister(_ecore_file_log_dom);
   _ecore_file_log_dom = -1;

   ecore_shutdown();

   return _ecore_file_init_count;
}

EAPI long long
ecore_file_mod_time(const char *file)
{
   long long time;

   if (!_ecore_file_stat(file, &time, NULL, NULL, NULL, NULL))
     return 0;

   return time;
}

EAPI long long
ecore_file_size(const char *file)
{
   long long size;

   if (!_ecore_file_stat(file, NULL, &size, NULL, NULL, NULL))
     return 0;

   return size;
}

EAPI Eina_Bool
ecore_file_exists(const char *file)
{
#ifdef _WIN32
   /* I prefer not touching the specific UNIX code... */
   return _ecore_file_stat(file, NULL, NULL, NULL, NULL, NULL);
#else
   struct stat st;
   if (!file) return EINA_FALSE;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return EINA_FALSE;
   return EINA_TRUE;
#endif
}

EAPI Eina_Bool
ecore_file_is_dir(const char *file)
{
   Eina_Bool is_dir;

   if (!_ecore_file_stat(file, NULL, NULL, NULL, &is_dir, NULL))
     return EINA_FALSE;

   return is_dir;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

EAPI Eina_Bool
ecore_file_mkdir(const char *dir)
{
   return (mkdir(dir, default_mode) == 0);
}

EAPI int
ecore_file_mkdirs(const char **dirs)
{
   int i = 0;

   if (!dirs) return -1;

   for (; *dirs; dirs++)
     if (ecore_file_mkdir(*dirs))
       i++;
   return i;
}

EAPI int
ecore_file_mksubdirs(const char *base, const char **subdirs)
{
#ifndef HAVE_ATFILE_SOURCE
   char buf[PATH_MAX];
   int baselen;
#else
   int fd;
   DIR *dir;
#endif
   int i;

   if (!subdirs) return -1;
   if ((!base) || (base[0] == '\0')) return -1;

   if ((!ecore_file_is_dir(base)) && (!ecore_file_mkpath(base)))
     return 0;

#ifndef HAVE_ATFILE_SOURCE
   baselen = eina_strlcpy(buf, base, sizeof(buf));
   if ((baselen < 1) || (baselen + 1 >= (int)sizeof(buf)))
     return 0;

   if (buf[baselen - 1] != '/')
     {
        buf[baselen] = '/';
        baselen++;
     }
#else
   dir = opendir(base);
   if (!dir)
     return 0;
   fd = dirfd(dir);
#endif

   i = 0;
   for (; *subdirs; subdirs++)
     {
#ifdef HAVE_ATFILE_SOURCE
        struct stat st;
#endif
        Eina_Bool is_dir;

#ifndef HAVE_ATFILE_SOURCE
        eina_strlcpy(buf + baselen, *subdirs, sizeof(buf) - baselen);
        if (_ecore_file_stat(buf, NULL, NULL, NULL, &is_dir, NULL))
          {
#else
        if (fstatat(fd, *subdirs, &st, 0) == 0)
          {
             is_dir = S_ISDIR(st.st_mode);
#endif
             if (is_dir)
               {
                  i++;
                  continue;
               }
          }
        else
          {
             if (errno == ENOENT)
               {
#ifndef HAVE_ATFILE_SOURCE
                  if (ecore_file_mkdir(buf))
#else
                  if (mkdirat(fd, *subdirs, default_mode) == 0)
#endif
                    {
                       i++;
                       continue;
                    }
                 }
            }
     }

#ifdef HAVE_ATFILE_SOURCE
   closedir(dir);
#endif

   return i;
}

EAPI Eina_Bool
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_file_remove(const char *file)
{
   if (remove(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_file_recursive_rm(const char *dir)
{
#ifndef _WIN32
   struct stat st;
#endif
   Eina_Bool is_dir;

#ifdef _WIN32
   if (!_ecore_file_stat(dir, NULL, NULL, NULL, &is_dir, NULL))
     return EINA_FALSE;
#else
   if (lstat(dir, &st) == -1)
     return EINA_FALSE;
   is_dir = S_ISDIR(st.st_mode);
#endif

   if (is_dir)
     {
        Eina_File_Direct_Info *info;
        Eina_Iterator *it;
        int ret;

        ret = 1;
        it = eina_file_direct_ls(dir);
        EINA_ITERATOR_FOREACH(it, info)
          {
             if (!ecore_file_recursive_rm(info->path))
               ret = 0;
          }
        eina_iterator_free(it);

        if (!ecore_file_rmdir(dir)) ret = 0;
        if (ret)
            return EINA_TRUE;
        else
            return EINA_FALSE;
     }
   else
     {
        return ecore_file_unlink(dir);
     }
}

static inline Eina_Bool
_ecore_file_mkpath_if_not_exists(const char *path)
{
   Eina_Bool is_dir;

   /* Windows: path like C: or D: etc are valid, but stat() returns an error */
#ifdef _WIN32
   if ((strlen(path) == 2) &&
       ((path[0] >= 'a' && path[0] <= 'z') ||
        (path[0] >= 'A' && path[0] <= 'Z')) &&
       (path[1] == ':'))
     return EINA_TRUE;
#endif

   if (!_ecore_file_stat(path, NULL, NULL, NULL, &is_dir, NULL))
     return ecore_file_mkdir(path);
   else if (!is_dir)
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

EAPI Eina_Bool
ecore_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);

   if (ecore_file_is_dir(path))
     return EINA_TRUE;

   for (i = 0; path[i] != '\0'; ss[i] = path[i], i++)
     {
        if (i == sizeof(ss) - 1) return EINA_FALSE;
        if (((path[i] == '/') || (path[i] == '\\')) && (i > 0))
          {
             ss[i] = '\0';
             if (!_ecore_file_mkpath_if_not_exists(ss))
               return EINA_FALSE;
          }
     }
   ss[i] = '\0';
   return _ecore_file_mkpath_if_not_exists(ss);
}

EAPI int
ecore_file_mkpaths(const char **paths)
{
   int i = 0;

   if (!paths) return -1;

   for (; *paths; paths++)
     if (ecore_file_mkpath(*paths))
       i++;
   return i;
}

EAPI Eina_Bool
ecore_file_cp(const char *src, const char *dst)
{
   FILE *f1, *f2;
   char buf[16384];
   char realpath1[PATH_MAX], realpath2[PATH_MAX];
   size_t num;
   Eina_Bool ret = EINA_TRUE;

   if (!realpath(src, realpath1)) return EINA_FALSE;
   if (realpath(dst, realpath2) && !strcmp(realpath1, realpath2)) return EINA_FALSE;

   f1 = fopen(src, "rb");
   if (!f1) return EINA_FALSE;
   f2 = fopen(dst, "wb");
   if (!f2)
     {
        fclose(f1);
        return EINA_FALSE;
     }
   while ((num = fread(buf, 1, sizeof(buf), f1)) > 0)
     {
        if (fwrite(buf, 1, num, f2) != num) ret = EINA_FALSE;
     }
   fclose(f1);
   fclose(f2);
   return ret;
}

EAPI Eina_Bool
ecore_file_mv(const char *src, const char *dst)
{
   char buf[PATH_MAX];
   int fd;

   if (rename(src, dst))
     {
        // File cannot be moved directly because
        // it resides on a different mount point.
        if (errno == EXDEV)
          {
             mode_t mode;
             Eina_Bool is_reg;

             // Make sure this is a regular file before
             // we do anything fancy.
             if (!_ecore_file_stat(src, NULL, NULL, &mode, NULL, &is_reg))
                 goto FAIL;
             if (is_reg)
               {
                  char *dir;
                  Eina_Tmpstr *tmpstr = NULL;

                  dir = ecore_file_dir_get(dst);
                  // Since we can't directly rename, try to
                  // copy to temp file in the dst directory
                  // and then rename.
                  snprintf(buf, sizeof(buf), "%s/.%s.tmp.XXXXXX",
                           dir, ecore_file_file_get(dst));
                  free(dir);
                  fd = eina_file_mkstemp(buf, &tmpstr);
                  if (fd < 0) goto FAIL;
                  close(fd);

                  // Copy to temp file
                  if (!ecore_file_cp(src, tmpstr))
                    {
                       eina_tmpstr_del(tmpstr);
                       goto FAIL;
                    }

                  // Set file permissions of temp file to match src
                  if (chmod(tmpstr, mode) == -1)
                    {
                       eina_tmpstr_del(tmpstr);
                       goto FAIL;
                    }

                  // Try to atomically move temp file to dst
                  if (rename(tmpstr, dst))
                    {
                       // If we still cannot atomically move
                       // do a normal copy and hope for the best.
                       if (!ecore_file_cp(tmpstr, dst))
                         {
                            eina_tmpstr_del(tmpstr);
                            goto FAIL;
                         }
                    }

                  // Delete temporary file and src
                  ecore_file_unlink(tmpstr);
                  ecore_file_unlink(src);
                  eina_tmpstr_del(tmpstr);
                  goto PASS;
               }
          }
#ifdef _WIN32
          if (errno == EEXIST)
            {
               struct _stat s;
               _stat(dst, &s);
               if (_S_IFREG & s.st_mode)
               {
                  ecore_file_unlink(dst);
                  if (rename(src, dst))
                    {
                       return EINA_TRUE;
                    }
               }
            }
#endif
        goto FAIL;
     }

PASS:
   return EINA_TRUE;

FAIL:
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_file_symlink(const char *src, const char *dest)
{
#ifndef _WIN32
   return !symlink(src, dest);
#else
   return EINA_FALSE;
   (void)src;
   (void)dest;
#endif
}

EAPI char *
ecore_file_realpath(const char *file)
{
   char buf[PATH_MAX];

   /*
    * Some implementations of realpath do not conform to the SUS.
    * And as a result we must prevent a null arg from being passed.
    */
   if (!file) return strdup("");
   if (!realpath(file, buf)) return strdup("");

   return strdup(buf);
}

EAPI const char *
ecore_file_file_get(const char *path)
{
   char *result = NULL;

   if (!path) return NULL;

   if ((result = strrchr(path, '/'))) result++;
   else result = (char *)path;

#ifdef _WIN32
   /*
    * Here, we know that there is no more / in the string beginning at
    * 'result'. So just check that there is no more \ from it.
    */
   {
      char *result_backslash;
      if ((result_backslash = strrchr(result, '\\')))
        result = ++result_backslash;
   }
#endif

   return result;
}

EAPI char *
ecore_file_dir_get(const char *file)
{
   char *p;
   char buf[PATH_MAX];

   if (!file) return NULL;
   strncpy(buf, file, PATH_MAX);
   buf[PATH_MAX - 1] = 0;
   p = dirname(buf);
   return strdup(p);
}

EAPI Eina_Bool
ecore_file_can_read(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, R_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_file_can_write(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, W_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_file_can_exec(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, X_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI char *
ecore_file_readlink(const char *link)
{
#ifndef _WIN32
   char buf[PATH_MAX];
   int count;

   if ((count = readlink(link, buf, sizeof(buf) - 1)) < 0) return NULL;
   buf[count] = 0;
   return strdup(buf);
#else
   return NULL;
   (void)link;
#endif
}

EAPI Eina_List *
ecore_file_ls(const char *dir)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *ls;
   Eina_List *list = NULL;

   ls = eina_file_direct_ls(dir);
   if (!ls) return NULL;

   EINA_ITERATOR_FOREACH(ls, info)
     {
        char *f;

        f = strdup(info->path + info->name_start);
        list = eina_list_append(list, f);
     }
   eina_iterator_free(ls);

   list = eina_list_sort(list, eina_list_count(list), EINA_COMPARE_CB(strcoll));

   return list;
}

EAPI char *
ecore_file_app_exe_get(const char *app)
{
   Eina_Strbuf *buf;
   char *exe;
   const char *p;
   Eina_Bool in_qout_double = EINA_FALSE;
   Eina_Bool in_qout_single = EINA_FALSE;

   if (!app) return NULL;
   buf = eina_strbuf_new();
   if (!buf) return NULL;
   p = app;
   if ((p[0] == '~') && (p[1] == '/'))
     {
        const char *home = eina_environment_home_get();
        if (home) eina_strbuf_append(buf, home);
        p++;
     }
   for (; *p; p++)
     {
        if (in_qout_double)
          {
             if (*p == '\\')
               {
                  if (p[1]) p++;
                  eina_strbuf_append_char(buf, *p);
               }
             else if (*p == '"') in_qout_double = EINA_FALSE;
             else eina_strbuf_append_char(buf, *p);
          }
        else if (in_qout_single)
          {
             if (*p == '\\')
               {
                  if (p[1]) p++;
                  eina_strbuf_append_char(buf, *p);
               }
             else if (*p == '\'') in_qout_single = EINA_FALSE;
             else eina_strbuf_append_char(buf, *p);
          }
        else
          {
             if (*p == '\\')
               {
                  if (p[1]) p++;
                  eina_strbuf_append_char(buf, *p);
               }
             else if (*p == '"') in_qout_double = EINA_TRUE;
             else if (*p == '\'') in_qout_single = EINA_TRUE;
             else
               {
                  if (isspace((unsigned char)(*p))) break;
                  eina_strbuf_append_char(buf, *p);
               }
          }
     }
   exe = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return exe;
}

EAPI char *
ecore_file_escape_name(const char *filename)
{
   const char *p;
   char *q;
   char buf[PATH_MAX];

   EINA_SAFETY_ON_NULL_RETURN_VAL(filename, NULL);

   p = filename;
   q = buf;
   while (*p)
     {
        if ((q - buf) > (PATH_MAX - 6)) return NULL;
        if (
            (*p == ' ') || (*p == '\\') || (*p == '\'') ||
            (*p == '\"') || (*p == ';') || (*p == '!') ||
            (*p == '#') || (*p == '$') || (*p == '%') ||
            (*p == '&') || (*p == '*') || (*p == '(') ||
            (*p == ')') || (*p == '[') || (*p == ']') ||
            (*p == '{') || (*p == '}') || (*p == '|') ||
            (*p == '<') || (*p == '>') || (*p == '?')
            )
          {
             *q = '\\';
             q++;
          }
        else if (*p == '\t')
          {
             *q = '\\';
             q++;
             *q = '\\';
             q++;
             *q = 't';
             q++;
             p++;
             continue;
          }
        else if (*p == '\n')
          {
            *q = '\\';
            q++;
            *q = '\\';
            q++;
            *q = 'n';
            q++;
            p++;
            continue;
          }

        *q = *p;
        q++;
        p++;
     }
   *q = 0;
   return strdup(buf);
}

EAPI char *
ecore_file_strip_ext(const char *path)
{
   char *p, *file = NULL;

   if (!path)
     return NULL;

   p = strrchr(path, '.');
   if (!p)
     file = strdup(path);
   else if (p != path)
     {
        file = malloc(((p - path) + 1) * sizeof(char));
        if (file)
          {
             memcpy(file, path, (p - path));
             file[p - path] = 0;
          }
     }

   return file;
}

EAPI int
ecore_file_dir_is_empty(const char *dir)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;

   it = eina_file_direct_ls(dir);
   if (!it) return -1;

   EINA_ITERATOR_FOREACH(it, info)
     {
        eina_iterator_free(it);
        return 0;
     }

   eina_iterator_free(it);
   return 1;
}
