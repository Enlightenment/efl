/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

static int init = 0;

/* externally accessible functions */
int
ecore_file_init()
{
   if (++init > 1) return init;

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
   if (--init > 0) return init;

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
   static int          have_uid = 0;
   static uid_t        uid = -1;
   static gid_t        gid = -1;
   struct stat st;
   int                 ok;

   if (!file) return 0;
   if (stat(file, &st) < 0) return 0;

   ok = 0;
   if (!have_uid) uid = getuid();
   if (!have_uid) gid = getgid();
   have_uid = 1;
   if (st.st_uid == uid)
     {
	if (st.st_mode & S_IXUSR) ok = 1;
     }
   else if (st.st_gid == gid)
     {
	if (st.st_mode & S_IXGRP) ok = 1;
     }
   else
     {
	if (st.st_mode & S_IXOTH) ok = 1;
     }
   return(ok);
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
   while ((dp = readdir(dirp)))
     {
	if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
	  {
	     char *file, *f;

	     /* insertion sort */
	     ecore_list_goto_first(list);
	     while ((file = ecore_list_current(list)))
	       {
		  if (strcmp(file, dp->d_name) > 0)
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
		  ecore_list_append(list, f);
	       }
	  }
     }
   closedir(dirp);

   ecore_list_goto_first(list);
   return list;
}
