/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS  64
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "ecore_file_private.h"


static int init = 0;

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
   if (++init != 1) return init;

//   if (!
       ecore_file_monitor_init();
//       )
//     goto error;
//   if (!
       ecore_file_path_init();
//       )
//     goto error;
//   if (!
       ecore_file_download_init();
//       )
//     goto error;
   return init;

error:

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return --init;
}

/**
 * Shutdown the Ecore_File
 * @return returns the number of libraries that still uses Ecore_File
 */
EAPI int
ecore_file_shutdown()
{
   if (--init != 0) return init;

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return init;
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
 * @return 1 if file exists on local filesystem, 0 otherwise
 */
EAPI int
ecore_file_exists(const char *file)
{
   struct stat st;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return 0;
   return 1;
}

/**
 * Check if file is a directory
 * @param  file The name of the file
 * @return 1 if file exist and is a directory, 0 otherwise
 */
EAPI int
ecore_file_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

/**
 * Create a new directory
 * @param  dir The name of the directory to create
 * @return 1 on successfull creation, 0 on failure
 *
 * The directory is created with the mode: S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
 */
EAPI int
ecore_file_mkdir(const char *dir)
{
   if (mkdir(dir, default_mode) < 0) return 0;
   return 1;
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
   if (!dirs) return -1;
   int i = 0;
   for (; *dirs != NULL; dirs++)
     if (ecore_file_mkdir(*dirs))
       i++;
   return i;
}

/**
 * Delete the given dir
 * @param  dir The name of the directory to delete
 * @return 1 on success, 0 on failure
 */
EAPI int
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return 0;
   return 1;
}

/**
 * Delete the given file
 * @param  file The name of the file to delete
 * @return 1 on success, 0 on failure
 */
EAPI int
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return 0;
   return 1;
}

/**
 * Delete a directory and all its contents
 * @param  dir The name of the directory to delete
 * @return 1 on success, 0 on failure
 *
 * If dir is a link only the link is removed
 */
EAPI int
ecore_file_recursive_rm(const char *dir)
{
   DIR                *dirp;
   struct dirent      *dp;
   char               path[PATH_MAX];
   char               buf[PATH_MAX];
   struct             stat st;
   int                ret;

   if (readlink(dir, buf, sizeof(buf)) > 0)
     {
	return ecore_file_unlink(dir);
     }

   ret = stat(dir, &st);
   if ((ret == 0) && (S_ISDIR(st.st_mode)))
     {
	ret = 1;
	if (stat(dir, &st) == -1) return 0;
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
        return ret;
     }
   else
     {
	if (ret == -1) return 0;
	return ecore_file_unlink(dir);
     }

   return 1;
}

static inline int
_ecore_file_mkpath_if_not_exists(const char *path)
{
   struct stat st;
   if (stat(path, &st) < 0)
     return ecore_file_mkdir(path);
   else if (!S_ISDIR(st.st_mode))
     return 0;
   else
     return 1;
}

/**
 * Create a complete path
 * @param  path The path to create
 * @return 1 on success, 0 on failure
 *
 * @see ecore_file_mkpaths() and ecore_file_mkdir()
 */
EAPI int
ecore_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   unsigned int i;

   if (ecore_file_is_dir(path))
     return 1;

   for (i = 0; path[i] != '\0'; ss[i] = path[i], i++)
     {
	if (i == sizeof(ss) - 1) return 0;
	if ((path[i] == '/') && (i > 0))
	  {
	     ss[i] = '\0';
	     if (!_ecore_file_mkpath_if_not_exists(ss))
	       return 0;
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
   if (!paths) return -1;
   int i = 0;
   for (; *paths != NULL; paths++)
     if (ecore_file_mkpath(*paths))
       i++;
   return i;
}

/**
 * Copy a file
 * @param  src The name of the source file
 * @param  dst The name of the destination file
 * @return 1 on success, 0 on failure
 */
EAPI int
ecore_file_cp(const char *src, const char *dst)
{
   FILE               *f1, *f2;
   char                buf[16384];
   char                realpath1[PATH_MAX];
   char                realpath2[PATH_MAX];
   size_t              num;
   int                 ret = 1;

   if (!realpath(src, realpath1)) return 0;
   if (realpath(dst, realpath2) && !strcmp(realpath1, realpath2)) return 0;

   f1 = fopen(src, "rb");
   if (!f1) return 0;
   f2 = fopen(dst, "wb");
   if (!f2)
     {
	fclose(f1);
	return 0;
     }
   while ((num = fread(buf, 1, sizeof(buf), f1)) > 0)
     {
	if (fwrite(buf, 1, num, f2) != num) ret = 0;
     }
   fclose(f1);
   fclose(f2);
   return ret;
}

/**
 * Move a file
 * @param  src The name of the source file
 * @param  dst The name of the destination file
 * @return 1 on success, 0 on failure
 */
EAPI int
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
   return 1;

FAIL:
   return 0;
}

/**
 * Create a symbolic link
 * @param  src The name of the file to link
 * @param  dest The name of link
 * @return 1 on success, 0 on failure
 */
EAPI int
ecore_file_symlink(const char *src, const char *dest)
{
   if (!symlink(src, dest)) return 1;

   return 0;
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
   char  buf[PATH_MAX];

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
   char               *p;
   char                buf[PATH_MAX];

   strncpy(buf, file, PATH_MAX);
   p = strrchr(buf, '/');
   if (!p)
     {
	return strdup(file);
     }

   if (p == buf) return strdup("/");

   *p = 0;
   return strdup(buf);
}

/**
 * Check if file can be read
 * @param  file The name of the file
 * @return 1 if the file is readable, 0 otherwise
 */
EAPI int
ecore_file_can_read(const char *file)
{
   if (!file) return 0;
   if (!access(file, R_OK)) return 1;
   return 0;
}

/**
 * Check if file can be written
 * @param  file The name of the file
 * @return 1 if the file is writable, 0 otherwise
 */
EAPI int
ecore_file_can_write(const char *file)
{
   if (!file) return 0;
   if (!access(file, W_OK)) return 1;
   return 0;
}

/**
 * Check if file can be executed
 * @param  file The name of the file
 * @return 1 if the file can be executed, 0 otherwise
 */
EAPI int
ecore_file_can_exec(const char *file)
{
   if (!file) return 0;
   if (!access(file, X_OK)) return 1;
   return 0;
}

/**
 * Get the path pointed by link
 * @param  link The name of the link
 * @return The path pointed by link or NULL
 */
EAPI char *
ecore_file_readlink(const char *link)
{
   char                buf[PATH_MAX];
   int                 count;

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
   char               *f;
   DIR                *dirp;
   struct dirent      *dp;
   Eina_List         *list = NULL;

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

   list = eina_list_sort(list, ECORE_SORT_MIN, ECORE_COMPARE_CB(strcoll));

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
     {
	file = strdup(path);
     }
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
