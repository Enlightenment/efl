#include "e_util.h"
#include "e_mem.h"
#include "e_str.h"

static void         sort_alpha_internal(char **a, int l, int r);

double
e_get_time(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

pid_t
e_run(char *exe)
{
   pid_t               pid;

   pid = fork();
   if (pid)
      return pid;
   setsid();
   execl("/bin/sh", "/bin/sh", "-c", exe, NULL);
   exit(0);
   return 0;
}

pid_t
e_run_in_dir(char *exe, char *dir)
{
   pid_t               pid;

   pid = fork();
   if (pid)
      return pid;
   chdir(dir);
   setsid();
   execl("/bin/sh", "/bin/sh", "-c", exe, NULL);
   exit(0);
   return 0;
}

void
e_set_env(char *variable, char *content)
{
   char env[4096];
   
   sprintf(env, "%s=%s", variable, content);
   putenv(env);
}

uid_t
e_user_id(void)
{
   static uid_t        uid = 0xffffffff;

   if (uid != 0xffffffff)
      return uid;
   uid = getuid();
   return uid;
}

gid_t
e_group_id(void)
{
   static gid_t        gid = 0xffffffff;

   if (gid != 0xffffffff)
      return gid;
   gid = getgid();
   return gid;
}

static void
sort_alpha_internal(char **a, int l, int r)
{
   int                 i, j, m;
   char               *v, *t;

   if (r > l)
     {
	m = ((r + l) >> 1) + 1;
	if (strcmp(a[l], a[r]) > 0)
	  {
	     t = a[l];
	     a[l] = a[r];
	     a[r] = t;
	  }
	if (strcmp(a[l], a[m]) > 0)
	  {
	     t = a[l];
	     a[l] = a[m];
	     a[m] = t;
	  }
	if (strcmp(a[r], a[m]) > 0)
	  {
	     t = a[r];
	     a[r] = a[m];
	     a[m] = t;
	  }
	v = a[r];
	i = l - 1;
	j = r;
	for (;;)
	  {
	     while (strcmp(a[++i], v) < 0);
	     while (strcmp(a[--j], v) > 0);
	     if (i >= j)
		break;
	     t = a[i];
	     a[i] = a[j];
	     a[j] = t;
	  }
	t = a[i];
	a[i] = a[r];
	a[r] = t;
	sort_alpha_internal(a, l, i - 1);
	sort_alpha_internal(a, i + 1, r);
     }
}

void
sort_alpha(char **list, int num)
{
   sort_alpha_internal(list, 0, num - 1);
}

char               *
e_home_dir(void)
{
   char               *home;

   home = getenv("HOME");
   if (!home)
      home = getenv("TMPDIR");
   if (!home)
      home = "/tmp";
   return home;
}

const char         *
e_get_basename(const char *s)
{
   const char         *b;

   b = strrchr(s, '/');
   return ((b) ? (++b) : (s));
}

void
spawn_info_window(char *path)
{

   char                runline[8192];
   char               *realline;

   realline = e_string_escape(path);
   sprintf(runline, "efileinfo %s", realline);
   e_run(runline);
   IF_FREE(realline);

}
