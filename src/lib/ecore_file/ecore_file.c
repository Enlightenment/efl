#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <libgen.h>
#endif

#ifdef _WIN32
# include <direct.h>
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

int _ecore_file_log_dom = -1;
static int _ecore_file_init_count = 0;

/* externally accessible functions */

/**
 * @addtogroup Ecore_File_Group Ecore_File - Files and directories convenience functions
 *
 * @{
 */

/**
 * @brief Initialize the Ecore_File library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up Ecore_File and the services it will use
 * (monitoring, downloading, PATH related feature). It returns 0 on
 * failure, otherwise it returns the number of times it has already
 * been called.
 *
 * When Ecore_File is not used anymore, call ecore_file_shutdown()
 * to shut down the Ecore_File library.
 */
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

/**
 * @brief Shut down the Ecore_File library.
 *
 * @return 0 when the library is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Ecore_File library. It returns 0 when it has
 * been called the same number of times than ecore_file_init(). In that case
 * it shuts down all the services it uses.
 */
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

/**
 * @brief Get the time of the last modification to the given file.
 *
 * @param file The name of the file.
 * @return Return the time of the last data modification, or 0 on
 * failure.
 *
 * This function returns the time of the last modification of
 * @p file. On failure, it returns 0.
 */
EAPI long long
ecore_file_mod_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_mtime;
}

/**
 * @brief Get the size of the given file.
 *
 * @param file The name of the file.
 * @return Return the size of the file in bytes, or 0 on failure.
 *
 * This function returns the size of @p file in bytes. On failure, it
 * returns 0.
 */
EAPI long long
ecore_file_size(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_size;
}

/**
 * @brief Check if the given file exists.
 *
 * @param file The name of the file.
 * @return @c EINA_TRUE if the @p file exists, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file exists on local filesystem,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_exists(const char *file)
{
   struct stat st;
   if (!file) return EINA_FALSE;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Check if the given file is a directory.
 *
 * @param file The name of the file.
 * @return @c EINA_TRUE if the file exists and is a directory, @c EINA_FALSE
 * otherwise.
 *
 * This function returns @c EINA_TRUE if @p file exists exists and is a
 * directory on local filesystem, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return EINA_FALSE;
   if (S_ISDIR(st.st_mode)) return EINA_TRUE;
   return EINA_FALSE;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

/**
 * @brief Create a new directory.
 *
 * @param  dir The name of the directory to create
 * @return @c EINA_TRUE on successful creation, @c EINA_FALSE otherwise.
 *
 * This function creates the directory @p dir with the mode S_IRUSR |
 * S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH. On
 * success, it returns @c EINA_TRUE, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_mkdir(const char *dir)
{
   if (mkdir(dir, default_mode) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Create complete directory in a batch.
 *
 * @param dirs The list of directories, null terminated.
 * @return The number of successful directories created, -1 if dirs is
 * @c NULL.
 *
 * This function creates all the directories that are in the null
 * terminated array @p dirs. The function loops over the directories
 * and call ecore_file_mkdir(). This function returns -1 if @p dirs is
 * @c NULL, otherwise if returns the number of suceesfully created
 * directories.
 */
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

/**
 * @brief Create complete list of sub-directories in a batch (optimized).
 *
 * @param base The base directory to act on.
 * @param subdirs The list of directories, null terminated.
 * @return number of successful directories created, -1 on failure.
 *
 * This function creates all the directories that are in the null
 * terminated array @p dirs in the @p base directory. If @p base does
 * not exist, it will be created. The function loops over the directories
 * and call ecore_file_mkdir(). The whole path of the directories must
 * exist. So if base/a/b/c wants to be created, @p subdirs must
 * contain "a", "a/b" and "a/b/c", in that order. This function
 * returns -1 if @p dirs or @p base are @c NULL, or if @p base is
 * empty ("\0"). It returns 0 is @p base is not a directory or
 * invalid, or if it can't be created. Otherwise if returns the number
 * of suceesfully created directories.
 */
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
        struct stat st;

#ifndef HAVE_ATFILE_SOURCE
        eina_strlcpy(buf + baselen, *subdirs, sizeof(buf) - baselen);
        if (stat(buf, &st) == 0)
#else
        if (fstatat(fd, *subdirs, &st, 0) == 0)
#endif
          {
             if (S_ISDIR(st.st_mode))
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
                  if (mkdir(buf, default_mode) == 0)
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

/**
 * @brief Delete the given directory.
 *
 * @param  dir The name of the directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function deletes @p dir. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Delete the given file.
 *
 * @param  file The name of the file to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function deletes @p file. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Remove the given file or directory.
 *
 * @param  file The name of the file or directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function removes @p file. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_remove(const char *file)
{
   if (remove(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Delete the given directory and all its contents.
 *
 * @param  dir The name of the directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function delete @p dir and all its contents. If @p dir is a
 * link only the link is removed. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_recursive_rm(const char *dir)
{
   struct stat st;

#ifdef _WIN32
   char buf[PATH_MAX];

   if (readlink(dir, buf, sizeof(buf) - 1) > 0)
     return ecore_file_unlink(dir);
   if (stat(dir, &st) == -1)
     return EINA_FALSE;
#else
   if (lstat(dir, &st) == -1)
     return EINA_FALSE;
#endif

   if (S_ISDIR(st.st_mode))
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
   struct stat st;

   /* Windows: path like C: or D: etc are valid, but stat() returns an error */
#ifdef _WIN32
   if ((strlen(path) == 2) &&
       ((path[0] >= 'a' && path[0] <= 'z') ||
        (path[0] >= 'A' && path[0] <= 'Z')) &&
       (path[1] == ':'))
     return EINA_TRUE;
#endif

   if (stat(path, &st) < 0)
     return ecore_file_mkdir(path);
   else if (!S_ISDIR(st.st_mode))
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

/**
 * @brief Create a complete path.
 *
 * @param  path The path to create
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function creates @p path and all the subdirectories it
 * contains. The separator is '/' or '\'. If @p path exists, this
 * function returns @c EINA_TRUE immediately. It returns @c EINA_TRUE on
 * success, @c EINA_FALSE otherwise.
 */
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

/**
 * @brief Create complete paths in a batch.
 *
 * @param paths list of paths, null terminated.
 * @return number of successful paths created, -1 if paths is NULL.
 *
 * This function creates all the directories that are in the null
 * terminated array @p paths. The function loops over the directories
 * and call ecore_file_mkpath(), hence on Windows, '\' must be
 * replaced by '/' before calling that function. This function
 * returns -1 if @p paths is @c NULL. Otherwise if returns the number
 * of suceesfully created directories.
 */
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

/**
 * @brief Copy the given file to the given destination.
 *
 * @param  src The name of the source file.
 * @param  dst The name of the destination file.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function copies @p src to @p dst. If the absolute path name of
 * @p src and @p dst can not be computed, or if they are equal, or if
 * the copy fails, the function returns @c EINA_FALSE, otherwise it
 * returns @c EINA_TRUE.
 */
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

/**
 * @brief Move the given file to the given destination.
 *
 * @param  src The name of the source file.
 * @param  dst The name of the destination file.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function moves @p src to @p dst. It returns @c EINA_TRUE on
 * success, @c EINA_FALSE otherwise.
 */
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
             struct stat st;

             // Make sure this is a regular file before
             // we do anything fancy.
             if (stat(src, &st) == -1)
                 goto FAIL;
             if (S_ISREG(st.st_mode))
               {
                  char *dir;

                  dir = ecore_file_dir_get(dst);
                  // Since we can't directly rename, try to
                  // copy to temp file in the dst directory
                  // and then rename.
                  snprintf(buf, sizeof(buf), "%s/.%s.tmp.XXXXXX",
                           dir, ecore_file_file_get(dst));
                  free(dir);
                  fd = mkstemp(buf);
                  if (fd < 0) goto FAIL;
                  close(fd);

                  // Copy to temp file
                  if (!ecore_file_cp(src, buf))
                    goto FAIL;

                  // Set file permissions of temp file to match src
                  if (chmod(buf, st.st_mode) == -1) goto FAIL;

                  // Try to atomically move temp file to dst
                  if (rename(buf, dst))
                    {
                       // If we still cannot atomically move
                       // do a normal copy and hope for the best.
                       if (!ecore_file_cp(buf, dst))
                         goto FAIL;
                    }

                  // Delete temporary file and src
                  ecore_file_unlink(buf);
                  ecore_file_unlink(src);
                  goto PASS;
               }
          }
        goto FAIL;
     }

PASS:
   return EINA_TRUE;

FAIL:
   return EINA_FALSE;
}

/**
 * @brief Create a symbolic link.
 *
 * @param  src The name of the file to link.
 * @param  dest The name of link.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function create the symbolic link @p dest of @p src. This
 * function does not work on Windows. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool
ecore_file_symlink(const char *src, const char *dest)
{
   if (!symlink(src, dest)) return EINA_TRUE;

   return EINA_FALSE;
}

/**
 * @brief Get the canonicalized absolute path name.
 *
 * @param  file The file path.
 * @return The canonicalized absolute pathname or an empty string on
 * failure.
 *
 * This function returns the absolute path name of @p file as a newly
 * allocated string. If @p file is @c NULL, or on error, this function
 * returns an empty string. Otherwise, it returns the absolute path
 * name. When not needed anymore, the returned value must be freed.
 */
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

/**
 * Get the filename from a given path.
 *
 * @param  path The complete path.
 * @return The file name.
 *
 * This function returns the file name of @p path. If @p path is
 * @c NULL, the functions returns @c NULL.
 */
EAPI const char *
ecore_file_file_get(const char *path)
{
   char *result = NULL;

   if (!path) return NULL;
   if ((result = strrchr(path, '/'))) result++;
   else result = (char *)path;
   return result;
}

/**
 * @brief Get the directory where the given file resides.
 *
 * @param  file The name of the file.
 * @return The directory name.
 *
 * This function returns the directory where @p file resides as anewly
 * allocated string. If @p file is @c NULL or on error, this function
 * returns @c NULL. When not needed anymore, the returned value must
 * be freed.
 */
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

/**
 * @brief Check if the given file can be read.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file is readable, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be read, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool
ecore_file_can_read(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, R_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @brief Check if the given file can be written.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file is writable, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be written, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool
ecore_file_can_write(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, W_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @brief Check if the given file can be executed.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file can be executed, @c EINA_FALSE
 * otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be executed, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool
ecore_file_can_exec(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, X_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @brief Get the path pointed by the given link.
 *
 * @param  lnk The name of the link.
 * @return The path pointed by link or NULL.
 *
 * This function returns the path pointed by @p link as a newly
 * allocated string. This function does not work on Windows. On
 * failure, the function returns @c NULL. When not needed anymore, the
 * returned value must be freed.
 */
EAPI char *
ecore_file_readlink(const char *lnk)
{
   char buf[PATH_MAX];
   int count;

   if ((count = readlink(lnk, buf, sizeof(buf) - 1)) < 0) return NULL;
   buf[count] = 0;
   return strdup(buf);
}

/**
 * @brief Get the list of the files and directories in the given
 * directory.
 *
 * @param  dir The name of the directory to list
 * @return Return an Eina_List containing all the files in the directory;
 *         on failure it returns NULL.
 *
 * This function returns a list of allocated strings of all the files
 * and directories contained in @p dir. The list will be sorted with
 * strcoll as compare function. That means that you may want to set
 * the current locale for the category LC_COLLATE with
 * setlocale(). For more information see the manual pages of strcoll
 * and setlocale. The list will not contain the directory entries for
 * '.' and '..'. On failure, @c NULL is returned. When not needed
 * anymore, the list elements must be freed.
 */
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

/**
 * @brief Return the executable from the given command.
 *
 * @param app The application command, with parameters.
 * @return The executable from @p app as a newly allocated string. Arguments 
 * are removed and escape characters are handled. If @p app is @c NULL, or 
 * on failure, the function returns @c NULL. When not needed anymore, the 
 * returned value must be freed.
 */
EAPI char *
ecore_file_app_exe_get(const char *app)
{
   char *p, *pp, *exe1 = NULL, *exe2 = NULL;
   char *exe = NULL;
   int in_quot_dbl = 0, in_quot_sing = 0, restart = 0;

   if (!app) return NULL;

   p = (char *)app;
restart:
   while ((*p) && (isspace((unsigned char)*p))) p++;
   exe1 = p;
   while (*p)
     {
        if (in_quot_sing)
          {
             if (*p == '\'')
               in_quot_sing = 0;
          }
        else if (in_quot_dbl)
          {
             if (*p == '\"')
               in_quot_dbl = 0;
          }
        else
          {
             if (*p == '\'')
               in_quot_sing = 1;
             else if (*p == '\"')
               in_quot_dbl = 1;
             if ((isspace((unsigned char)*p)) && ((p <= app) || (p[-1] == '\\')))
               break;
          }
        p++;
     }
   exe2 = p;
   if (exe2 == exe1) return NULL;
   if (*exe1 == '~')
     {
        char *homedir;
        int len;

        /* Skip ~ */
        exe1++;

        homedir = getenv("HOME");
        if (!homedir) return NULL;
        len = strlen(homedir);
        exe = malloc(len + exe2 - exe1 + 2);
        if (!exe) return NULL;
        pp = exe;
        if (len)
          {
             strcpy(exe, homedir);
             pp += len;
             if (*(pp - 1) != '/')
               {
                  *pp = '/';
                  pp++;
               }
          }
     }
   else
     {
        exe = malloc(exe2 - exe1 + 1);
        if (!exe) return NULL;
        pp = exe;
     }
   p = exe1;
   restart = 0;
   in_quot_dbl = 0;
   in_quot_sing = 0;
   while (*p)
     {
        if (in_quot_sing)
          {
             if (*p == '\'')
               in_quot_sing = 0;
             else
               {
                  *pp = *p;
                  pp++;
               }
          }
        else if (in_quot_dbl)
          {
             if (*p == '\"')
               in_quot_dbl = 0;
             else
               {
                  /* technically this is wrong. double quotes also accept
                   * special chars:
                   *
                   * $, `, \
                   */
                  *pp = *p;
                  pp++;
               }
          }
        else
          {
             /* technically we should handle special chars:
              *
              * $, `, \, etc.
              */
             if ((p > exe1) && (p[-1] == '\\'))
               {
                  if (*p != '\n')
                    {
                       *pp = *p;
                       pp++;
                    }
               }
             else if ((p > exe1) && (*p == '='))
               {
                  restart = 1;
                  *pp = *p;
                  pp++;
               }
             else if (*p == '\'')
               in_quot_sing = 1;
             else if (*p == '\"')
               in_quot_dbl = 1;
             else if (isspace((unsigned char)*p))
               {
                  if (restart)
                    {
                       if (exe) free(exe);
                       exe = NULL;
                       goto restart;
                    }
                  else
                    break;
               }
             else
               {
                  *pp = *p;
                  pp++;
               }
          }
        p++;
     }
   *pp = 0;
   return exe;
}

/**
 * @brief Add the escape sequence ('\\') to the given file name.
 *
 * @param  filename The file name.
 * @return The file name with special characters escaped.
 *
 * This function adds the escape sequence ('\\') to the given file
 * name and returns the result as a newly allocated string. If the
 * length of the returned string is longer than PATH_MAX, or on
 * failure, @c NULL is returned. When not needed anymore, the returned
 * value must be freed.
 */
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

/**
 * @brief Remove the extension from the given file name.
 *
 * @param  path The name of the file.
 * @return A newly allocated string with the extension stripped out or
 * @c NULL on errors.
 *
 * This function removes the extension from @p path and returns the
 * result as a newly allocated string. If @p path is @c NULL, or on
 * failure, the function returns @c NULL. When not needed anymore, the
 * returned value must be freed.
 */
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

/**
 * @brief Check if the given directory is empty.
 *
 * @param  dir The name of the directory to check.
 * @return @c 1 if directory is empty, @c 0 if it has at least one file or
 * @c -1 in case of errors.
 *
 * This functions checks if @p dir is empty. The '.' and '..' files
 * will be ignored. If @p dir is empty, 1 is returned, if it contains
 * at least one file, @c 0 is returned. On failure, @c -1 is returned.
 */
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

/**
 * @}
 */
