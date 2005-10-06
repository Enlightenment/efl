/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"
#include <ctype.h>

static int init = 0;

/* externally accessible functions */
int
ecore_file_init()
{
   if (++init != 1) return init;

   if (!ecore_file_monitor_init())
     goto error;
   if (!ecore_file_path_init())
     goto error;
   if (!ecore_file_download_init())
     goto error;
   return init;

error:

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return --init;
}

int
ecore_file_shutdown()
{
   if (--init != 0) return init;

   ecore_file_monitor_shutdown();
   ecore_file_path_shutdown();
   ecore_file_download_shutdown();

   return init;
}

time_t
ecore_file_mod_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   return st.st_mtime;
}

int
ecore_file_exists(const char *file)
{
   struct stat st;

   /*Workaround so that "/" returns a true, otherwise we can't monitor "/" in ecore_file_monitor*/
   if (stat(file, &st) < 0 && strcmp(file, "/")) return 0;
   return 1;
}

int
ecore_file_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

int
ecore_file_mkdir(const char *dir)
{
   if (mkdir(dir, default_mode) < 0) return 0;
   return 1;
}

int
ecore_file_rmdir(const char *dir)
{
   if (rmdir(dir) < 0) return 0;
   return 1;
}

int
ecore_file_unlink(const char *file)
{
   if (unlink(file) < 0) return 0;
   return 1;
}

int
ecore_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   int  i;

   ss[0] = 0;
   i = 0;
   while (path[i])
     {
	if (i == sizeof(ss) - 1) return 0;
	ss[i] = path[i];
	ss[i + 1] = 0;
	if (path[i] == '/')
	  {
	     ss[i] = 0;
	     if ((ecore_file_exists(ss)) && (!ecore_file_is_dir(ss))) return 0;
	     else if (!ecore_file_exists(ss)) ecore_file_mkdir(ss);
	     ss[i] = '/';
	  }
	i++;
     }
   if ((ecore_file_exists(ss)) && (!ecore_file_is_dir(ss))) return 0;
   else if (!ecore_file_exists(ss)) ecore_file_mkdir(ss);
   return 1;
}

int
ecore_file_cp(const char *src, const char *dst)
{
   FILE               *f1, *f2;
   char                buf[16384];
   size_t              num;

   f1 = fopen(src, "rb");
   if (!f1) return 0;
   f2 = fopen(dst, "wb");
   if (!f2)
     {
	fclose(f1);
	return 0;
     }
   while ((num = fread(buf, 1, 16384, f1)) > 0) fwrite(buf, 1, num, f2);
   fclose(f1);
   fclose(f2);
   return 1;
}

int
ecore_file_mv(const char *src, const char *dst)
{
   if (ecore_file_exists(dst)) return 0;
   if (rename(src, dst)) return 0;
   return 1;
}

char *
ecore_file_realpath(const char *file)
{
   char  buf[PATH_MAX];
   struct stat st;

   if (!realpath(file, buf) || stat(buf, &st)) return strdup("");
   return strdup(buf);
}

char *
ecore_file_get_file(char *path)
{
   char *result = NULL;

   if (!path) return NULL;
   if ((result = strrchr(path, '/'))) result++;
   else result = path;
   return result;
}

char *
ecore_file_get_dir(char *file)
{
   char               *p;
   char                buf[PATH_MAX];

   strncpy(buf, file, PATH_MAX);
   p = strrchr(buf, '/');
   if (!p)
     {
	return strdup(file);
     }
   *p = 0;
   return strdup(buf);
}

int
ecore_file_can_exec(const char *file)
{
   if (!file) return 0;
   if (!access(file, X_OK)) return 1;
   return 0;
}

char *
ecore_file_readlink(const char *link)
{
   char                buf[PATH_MAX];
   int                 count;

   if ((count = readlink(link, buf, sizeof(buf))) < 0) return NULL;
   buf[count] = 0;
   return strdup(buf);
}

Ecore_List *
ecore_file_ls(const char *dir)
{
   DIR                *dirp;
   struct dirent      *dp;
   Ecore_List        *list;

   dirp = opendir(dir);
   if (!dirp) return NULL;

   list = ecore_list_new();
   ecore_list_set_free_cb(list, free);

   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	     char *file, *f;

	     /* insertion sort */
	     ecore_list_goto_first(list);
	     while ((file = ecore_list_current(list)))
	       {
		  if (strcasecmp(file, dp->d_name) > 0)
		    {
		       f = strdup(dp->d_name);
		       ecore_list_insert(list, f);
		       break;
		    }
		  ecore_list_next(list);
	       }
	     /* nowhwre to go? just append it */
	     if (!file)
	       {
		  f = strdup(dp->d_name);
		  ecore_list_insert(list, f);
	       }
	  }
     }
   closedir(dirp);

   ecore_list_goto_first(list);
   return list;
}

char *
ecore_file_app_exe_get(const char *app)
{
   char *p, *pp, *exe1 = NULL, *exe2 = NULL;
   char *exe;
   int in_quot_dbl = 0, in_quot_sing = 0;
   
   p = (char *)app;
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
	     else if (*p == '\'')
	       in_quot_sing = 1;
	     else if (*p == '\"')
	       in_quot_dbl = 1;
	     else if (isspace(*p))
	       break;
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

char *
ecore_file_strip_ext(const char *path)
{
   char *p, *file;

   file = strdup(path);
   if (!file) return NULL;

   p = strrchr(file, '.');
   if (p)
     *p = 0;

   return file;
}
