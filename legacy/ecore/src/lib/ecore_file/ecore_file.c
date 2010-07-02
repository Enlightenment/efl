/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <libgen.h>
#endif

#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS  64
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "ecore_file_private.h"

int _ecore_file_log_dom = -1;
static int _ecore_file_init_count = 0;

/* externally accessible functions */
/**
 * Initialize Ecore_File and the services it will use. Call this function
 * once before you use any of the ecore file functions.
 * @return Return the number howoften ecore_file_init() was call succesfully;
 *         0 if it failed.
 */
EAPI int
ecore_file_init()
{
   if (++_ecore_file_init_count != 1)
     return _ecore_file_init_count;
   _ecore_file_log_dom = eina_log_domain_register("EcoreFile", ECORE_FILE_DEFAULT_LOG_COLOR);
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
 * Shutdown the Ecore_File
 * @return returns the number of libraries that still uses Ecore_File
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
   return _ecore_file_init_count;
}

/**
 * Get the time of the last modification to the give file
 * @param file The name of the file
 * @return Return the time of the last data modification, if an error should
 *         occur it will return 0
 */
EAPI long long
ecore_file_mod_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_mtime;
}

/**
 * Get the size of the given file
 * @param  file The name of the file
 * @return The size of the file in byte
 */
EAPI long long
ecore_file_size(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_size;
}

/**
 * Check if file exists
 * @param  file The name of the file
 * @return EINA_TRUE if file exists on local filesystem, EINA_FALSE otherwise
 */
EAPI Eina_Bool
ecore_file_exists(const char *file)
{
   struct stat st;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * Check if file is a directory
 * @param  file The name of the file
 * @return EINA_TRUE if file exist and is a directory, EINA_FALSE otherwise
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
 * Create a new directory
 * @param  dir The name of the directory to create
 * @return EINA_TRUE on successfull creation, EINA_FALSE on failure
 *
 * The directory is created with the mode: S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
 */
EAPI Eina_Bool
ecore_file_mkdir(const char *dir)
{
   if (mkdir(dir, default_mode) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * Create complete directory in a batch.
 *
 * @param dirs list of directories, null terminated.
 * @return number of successfull directories created, -1 if dirs is NULL.
 *
 * @see ecore_file_mkdir() and ecore_file_mkpaths()
 */
EAPI int
ecore_file_mkdirs(const char **dirs)
{
   int i = 0;

   if (!dirs) return -1;

   for (; *dirs != NULL; dirs++)
     if (ecore_file_mkdir(*dirs))
       i++;
   return i;
}

/**
 * Create complete list of sub-directories in a batch (optimized).
 *
 * @param base the base directory to act on, will be created if does
 *     not exists.
 * @param subdirs list of directories, null terminated. These are
 *     created similarly to ecore_file_mkdir(), so same mode and whole
 *     path to that point must exists. So if creating base/a/b/c,
 *     provide subdirs with "a", "a/b" and "a/b/c" in that order!
 *
 * @return number of successfull directories created, -1 if subdirs or
 *     base is NULL or invalid.
 *
 * @see ecore_file_mkdir() and ecore_file_mkpaths()
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
   for (; *subdirs != NULL; subdirs++)
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
 * Delete the given dir
 * @param  dir The name of the directory to delete
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * Delete the given file
 * @param  file The name of the file to delete
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * Remove the given file or directory
 * @param  file The name of the file or directory to delete
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool
ecore_file_remove(const char *file)
{
   if (remove(file) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * Delete a directory and all its contents
 * @param  dir The name of the directory to delete
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * If dir is a link only the link is removed
 */
EAPI Eina_Bool
ecore_file_recursive_rm(const char *dir)
{
   DIR *dirp;
   struct dirent *dp;
   char path[PATH_MAX], buf[PATH_MAX];
   struct stat st;
   int ret;

   if (readlink(dir, buf, sizeof(buf)) > 0)
     return ecore_file_unlink(dir);

   ret = stat(dir, &st);
   if ((ret == 0) && (S_ISDIR(st.st_mode)))
     {
	ret = 1;
	if (stat(dir, &st) == -1) return EINA_FALSE;
	dirp = opendir(dir);
	if (dirp)
	  {
	     while ((dp = readdir(dirp)))
	       {
		  if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
		    {
		       snprintf(path, PATH_MAX, "%s/%s", dir, dp->d_name);
		       if (!ecore_file_recursive_rm(path))
			 ret = 0;
		    }
	       }
	     closedir(dirp);
	  }
	if (!ecore_file_rmdir(dir)) ret = 0;
        if (ret)
            return EINA_TRUE;
        else
            return EINA_FALSE;
     }
   else
     {
	if (ret == -1) return EINA_FALSE;
	return ecore_file_unlink(dir);
     }
}

static inline Eina_Bool
_ecore_file_mkpath_if_not_exists(const char *path)
{
   struct stat st;

   if (stat(path, &st) < 0)
     return ecore_file_mkdir(path);
   else if (!S_ISDIR(st.st_mode))
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

/**
 * Create a complete path
 * @param  path The path to create
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @see ecore_file_mkpaths() and ecore_file_mkdir()
 */
EAPI Eina_Bool
ecore_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   unsigned int i;

   if (ecore_file_is_dir(path))
     return EINA_TRUE;

   for (i = 0; path[i] != '\0'; ss[i] = path[i], i++)
     {
	if (i == sizeof(ss) - 1) return EINA_FALSE;
	if ((path[i] == '/') && (i > 0))
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
 * Create complete paths in a batch.
 *
 * @param paths list of paths, null terminated.
 * @return number of successfull paths created, -1 if paths is NULL.
 *
 * @see ecore_file_mkpath() and ecore_file_mkdirs()
 */
EAPI int
ecore_file_mkpaths(const char **paths)
{
   int i = 0;

   if (!paths) return -1;

   for (; *paths != NULL; paths++)
     if (ecore_file_mkpath(*paths))
       i++;
   return i;
}

/**
 * Copy a file
 * @param  src The name of the source file
 * @param  dst The name of the destination file
 * @return EINA_TRUE on success, EINA_FALSE on failure
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
 * Move a file
 * @param  src The name of the source file
 * @param  dst The name of the destination file
 * @return EINA_TRUE on success, EINA_FALSE on failure
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
	     stat(src, &st);
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
		  if (fd < 0)
		    {
		       perror("mkstemp");
		       goto FAIL;
		    }
		  close(fd);

		  // Copy to temp file
		  if (!ecore_file_cp(src, buf))
		    goto FAIL;

		  // Set file permissions of temp file to match src
		  chmod(buf, st.st_mode);

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
 * Create a symbolic link
 * @param  src The name of the file to link
 * @param  dest The name of link
 * @return EINA_TRUE on success, EINA_FALSE on failure
 */
EAPI Eina_Bool
ecore_file_symlink(const char *src, const char *dest)
{
   if (!symlink(src, dest)) return EINA_TRUE;

   return EINA_FALSE;
}

/**
 * Get the canonicalized absolute pathname
 * @param  file The file path
 * @return The canonicalized absolute pathname; on failure it will return
 *         an empty string
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
 * Get the filename from a give path
 * @param  path The complete path
 * @return Only the file name
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
 * Get the directory where file reside
 * @param  file The name of the file
 * @return The directory name
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
 * Check if file can be read
 * @param  file The name of the file
 * @return EINA_TRUE if the file is readable, EINA_FALSE otherwise
 */
EAPI Eina_Bool
ecore_file_can_read(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, R_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * Check if file can be written
 * @param  file The name of the file
 * @return EINA_TRUE if the file is writable, EINA_FALSE otherwise
 */
EAPI Eina_Bool
ecore_file_can_write(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, W_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * Check if file can be executed
 * @param  file The name of the file
 * @return EINA_TRUE if the file can be executed, EINA_FALSE otherwise
 */
EAPI Eina_Bool
ecore_file_can_exec(const char *file)
{
   if (!file) return EINA_FALSE;
   if (!access(file, X_OK)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * Get the path pointed by link
 * @param  link The name of the link
 * @return The path pointed by link or NULL
 */
EAPI char *
ecore_file_readlink(const char *link)
{
   char buf[PATH_MAX];
   int count;

   if ((count = readlink(link, buf, sizeof(buf))) < 0) return NULL;
   buf[count] = 0;
   return strdup(buf);
}

/**
 * Get the list of the files and directories in a given directory. The list
 * will be sorted with strcoll as compare function. That means that you may
 * want to set the current locale for the category LC_COLLATE with setlocale().
 * For more information see the manual pages of strcoll and setlocale.
 * The list will not contain the directory entries for '.' and '..'.
 * @param  dir The name of the directory to list
 * @return Return an Eina_List containing all the files in the directory;
 *         on failure it returns NULL.
 */
EAPI Eina_List *
ecore_file_ls(const char *dir)
{
   char *f;
   DIR *dirp;
   struct dirent *dp;
   Eina_List *list = NULL;

   dirp = opendir(dir);
   if (!dirp) return NULL;

   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	       f = strdup(dp->d_name);
	       list = eina_list_append(list, f);
	  }
     }
   closedir(dirp);

   list = eina_list_sort(list, eina_list_count(list), EINA_COMPARE_CB(strcoll));

   return list;
}

/**
 * FIXME: To be documented.
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
   while ((*p) && (isspace(*p))) p++;
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
	     if ((isspace(*p)) && (!((p > app) && (p[-1] != '\\'))))
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
	if (exe) free(exe);
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
	if (exe) free(exe);
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
		  /* techcincally this is wrong. double quotes also accept
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
	     else if (isspace(*p))
	       {
		  if (restart)
		    goto restart;
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
 * Add the escape sequence ('\\') to the given filename
 * @param  filename The file name
 * @return The file name with special characters escaped; if the length of the
 *         resulting string is longer than PATH_MAX it will return NULL
 */
EAPI char *
ecore_file_escape_name(const char *filename)
{
   const char *p;
   char *q;
   char buf[PATH_MAX];

   p = filename;
   q = buf;
   while (*p)
     {
	if ((q - buf) > (PATH_MAX - 6)) return NULL;
	if (
	    (*p == ' ') || (*p == '\t') || (*p == '\n') ||
	    (*p == '\\') || (*p == '\'') || (*p == '\"') ||
	    (*p == ';') || (*p == '!') || (*p == '#') ||
	    (*p == '$') || (*p == '%') || (*p == '&') ||
	    (*p == '*') || (*p == '(') || (*p == ')') ||
	    (*p == '[') || (*p == ']') || (*p == '{') ||
	    (*p == '}') || (*p == '|') || (*p == '<') ||
	    (*p == '>') || (*p == '?')
	    )
	  {
	     *q = '\\';
	     q++;
	  }
	*q = *p;
	q++;
	p++;
     }
   *q = 0;
   return strdup(buf);
}

/**
 * Remove the extension from a given path
 * @param  path The name of the file
 * @return A newly allocated string with the extension stripped out or NULL on errors
 */
EAPI char *
ecore_file_strip_ext(const char *path)
{
   char *p, *file = NULL;

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
 * Check if the given directory is empty. The '.' and '..' files will be ignored.
 * @param  dir The name of the directory to check
 * @return 1 if directory is empty, 0 if it has at least one file or -1 in case of errors
 */
EAPI int
ecore_file_dir_is_empty(const char *dir)
{
   DIR *dirp;
   struct dirent *dp;

   dirp = opendir(dir);
   if (!dirp) return -1;

   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	     closedir(dirp);
	     return 0;
	  }
     }
   
   closedir(dirp);
   return 1;
}
