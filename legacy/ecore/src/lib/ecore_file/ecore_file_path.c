/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"
#include <ctype.h>

static int init = 0;
static Ecore_List *__ecore_file_path_bin;

static Ecore_List *_ecore_file_path_from_env(const char *env);

int
ecore_file_path_init(void)
{
   if (++init != 1) return init;
   __ecore_file_path_bin = _ecore_file_path_from_env("PATH");
   return init;
}

int
ecore_file_path_shutdown(void)
{
   if (--init != 0) return init;
   ecore_list_destroy(__ecore_file_path_bin);
   return init;
}

Ecore_List *
_ecore_file_path_from_env(const char *env)
{
   Ecore_List *path;
   char *env_path, *p, *last;

   path = ecore_list_new();

   env_path = getenv(env);
   if (!env_path)
     return path;

   env_path = strdup(env_path);
   last = env_path;
   for (p = env_path; *p; p++)
     {
        if (*p == ':')
          *p = '\0';

        if (!*p)
          {
             ecore_list_append(path, strdup(last));
             last = p + 1;
          }
     }
   if (p > last)
     ecore_list_append(path, strdup(last));

   free(env_path);
   return path;
}

int
ecore_file_app_installed(const char *exe)
{
   char *dir;
   char  buf[PATH_MAX];

   if (!exe) return 0;
   if (ecore_file_can_exec(exe)) return 1;

   ecore_list_goto_first(__ecore_file_path_bin);
   while ((dir = ecore_list_next(__ecore_file_path_bin)) != NULL)
     {
	snprintf(buf, sizeof(buf), "%s/%s", dir, exe);
	if (ecore_file_can_exec(buf))
	  return 1;
     }
   return 0;
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
   exe = malloc(exe2 - exe1 + 1);
   if (!exe) return NULL;
   p = exe1;
   in_quot_dbl = 0;
   in_quot_sing = 0;
   pp = exe;
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
	     if ((p > app) && (p[-1] == '\\'))
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
