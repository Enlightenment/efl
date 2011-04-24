#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>

#ifdef HAVE_DLADDR
# include <dlfcn.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"
#include "eina_prefix.h"

#ifdef _WIN32
# define DSEP_C '\\'
# define DSEP_S "\\"
#else
# define DSEP_C '/'
# define DSEP_S "/"
#endif /* _WIN32 */

struct _Eina_Prefix
{
   char *exe_path;
   
   char *prefix_path;
   char *prefix_path_bin;
   char *prefix_path_data;
   char *prefix_path_lib;
   char *prefix_path_locale;
   
   unsigned char fallback : 1;
   unsigned char no_common_prefix : 1;
   unsigned char env_used : 1;
};

/* local subsystem globals */

#define STRDUP_REP(x, y) do { if (x) free(x); x = strdup(y); } while (0)
#define IF_FREE_NULL(p) do { if (p) { free(p); p = NULL; } } while (0)

/* local subsystem functions */

static int
_fallback(Eina_Prefix *pfx, const char *pkg_bin, const char *pkg_lib,
          const char *pkg_data, const char *pkg_locale, const char *envprefix)
{
   char *p;

   STRDUP_REP(pfx->prefix_path, pkg_bin);
   if (!pfx->prefix_path) return 0;
   p = strrchr(pfx->prefix_path, DSEP_C);
   if (p) *p = 0;
   STRDUP_REP(pfx->prefix_path_bin, pkg_bin);
   STRDUP_REP(pfx->prefix_path_lib, pkg_lib);
   STRDUP_REP(pfx->prefix_path_data, pkg_data);
   STRDUP_REP(pfx->prefix_path_locale, pkg_locale);
   fprintf(stderr,
           "WARNING: Could not determine its installed prefix for '%s'\n"
           "      so am falling back on the compiled in default:\n"
           "        %s\n"
           "      implied by the following:\n"
           "        bindir    = %s\n"
           "        libdir    = %s\n"
           "        datadir   = %s\n"
           "        localedir = %s\n"
           "      Try setting the following environment variables:\n"
           "        %s_PREFIX     - points to the base prefix of install\n"
           "      or the next 4 variables\n"
           "        %s_BIN_DIR    - provide a specific binary directory\n"
           "        %s_LIB_DIR    - provide a specific library directory\n"
           "        %s_DATA_DIR   - provide a specific data directory\n"
           "        %s_LOCALE_DIR - provide a specific locale directory\n"
           , envprefix, 
           pfx->prefix_path, pkg_bin, pkg_lib, pkg_data, pkg_locale,
           envprefix, envprefix, envprefix, envprefix, envprefix);
   pfx->fallback = 1;
   return 1;
}

#ifndef _WIN32
static int
_try_proc(Eina_Prefix *pfx, void *symbol)
{
   FILE *f;
   char buf[4096];

   f = fopen("/proc/self/maps", "rb");
   if (!f) return 0;
   while (fgets(buf, sizeof(buf), f))
     {
	int len;
	char *p, mode[5] = "";
	unsigned long ptr1 = 0, ptr2 = 0;

	len = strlen(buf);
	if (buf[len - 1] == '\n')
	  {
	     buf[len - 1] = 0;
	     len--;
	  }
	if (sscanf(buf, "%lx-%lx %4s", &ptr1, &ptr2, mode) == 3)
	  {
	     if (!strcmp(mode, "r-xp"))
	       {
		  if (((void *)ptr1 <= symbol) && (symbol < (void *)ptr2))
		    {
		       p = strchr(buf, '/');
		       if (p)
			 {
			    if (len > 10)
			      {
				 if (!strcmp(buf + len - 10, " (deleted)"))
                                    buf[len - 10] = 0;
			      }
                            STRDUP_REP(pfx->exe_path, p);
			    fclose(f);
			    return 1;
			 }
		       else break;
		    }
	       }
	  }
     }
   fclose(f);
   return 0;
}
#endif

static int
_try_argv(Eina_Prefix *pfx, const char *argv0)
{
   char *path, *p, *cp, *s;
   int len, lenexe;
   char buf[PATH_MAX], buf2[PATH_MAX], buf3[PATH_MAX];

   /* 1. is argv0 abs path? */
#ifdef _WIN32
   if (argv0[0] && (argv0[1] == ':'))
#else
   if (argv0[0] == '/')
#endif
     {
        STRDUP_REP(pfx->exe_path, argv0);
	if (access(pfx->exe_path, X_OK) == 0) return 1;
        IF_FREE_NULL(pfx->exe_path);
	return 0;
     }
   /* 2. relative path */
   if (strchr(argv0, DSEP_C))
     {
	if (getcwd(buf3, sizeof(buf3)))
	  {
	     snprintf(buf2, sizeof(buf2), "%s" DSEP_S "%s", buf3, argv0);
	     if (realpath(buf2, buf))
	       {
                  STRDUP_REP(pfx->exe_path, buf);
		  if (access(pfx->exe_path, X_OK) == 0) return 1;
                  IF_FREE_NULL(pfx->exe_path);
	       }
	  }
     }
   /* 3. argv0 no path - look in PATH */
   path = getenv("PATH");
   if (!path) return 0;
   p = path;
   cp = p;
   lenexe = strlen(argv0);
   while ((p = strchr(cp, ':')))
     {
	len = p - cp;
	s = malloc(len + 1 + lenexe + 1);
	if (s)
	  {
	     strncpy(s, cp, len);
	     s[len] = DSEP_C;
	     strcpy(s + len + 1, argv0);
	     if (realpath(s, buf))
	       {
		  if (access(buf, X_OK) == 0)
		    {
                       STRDUP_REP(pfx->exe_path, buf);
		       free(s);
		       return 1;
		    }
	       }
	     free(s);
	  }
        cp = p + 1;
     }
   cp = getcwd(buf3, sizeof(buf3));
   if (cp)
     {
        len = strlen(cp);
        s = malloc(len + 1 + lenexe + 1);
        if (s)
          {
             strncpy(s, cp, len);
             s[len] = DSEP_C;
             strcpy(s + len + 1, argv0);
             if (realpath(s, buf))
               {
                  if (access(buf, X_OK) == 0)
                    {
                       STRDUP_REP(pfx->exe_path, buf);
                       free(s);
                       return 1;
                    }
               }
             free(s);
          }
     }
   /* 4. big problems. arg[0] != executable - weird execution */
   return 0;
}

static int
_get_env_var(char **var, const char *env, const char *prefix, const char *dir)
{
   char buf[PATH_MAX];
   const char *s = getenv(env);
      
   if (s)
     {
        STRDUP_REP(*var, s);
        return 1;
     }
   else if (prefix)
     {
        snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", prefix, dir);
        STRDUP_REP(*var, buf);
        return 1;
     }
   return 0;
}

static int
_get_env_vars(Eina_Prefix *pfx,
              const char *envprefix,
              const char *bindir,
              const char *libdir,
              const char *datadir,
              const char *localedir)
{
   char env[1024];
   const char *s;
   int ret = 0;
   
   snprintf(env, sizeof(env), "%s_PREFIX", envprefix);
   if ((s = getenv(env))) STRDUP_REP(pfx->prefix_path, s);
   snprintf(env, sizeof(env), "%s_BIN_DIR", envprefix);
   ret += _get_env_var(&pfx->prefix_path_bin, env, s, bindir);
   snprintf(env, sizeof(env), "%s_LIB_DIR", envprefix);
   ret += _get_env_var(&pfx->prefix_path_lib, env, s, libdir);
   snprintf(env, sizeof(env), "%s_DATA_DIR", envprefix);
   ret += _get_env_var(&pfx->prefix_path_data, env, s, datadir);
   snprintf(env, sizeof(env), "%s_LOCALE_DIR", envprefix);
   ret += _get_env_var(&pfx->prefix_path_locale, env, s, localedir);
   return ret;
}

/* externally accessible functions */
EAPI Eina_Prefix *
eina_prefix_new(const char *argv0, void *symbol, const char *envprefix,
                const char *sharedir, const char *magicsharefile,
                const char *pkg_bin, const char *pkg_lib,
                const char *pkg_data, const char *pkg_locale)
{
   Eina_Prefix *pfx;
   char *p, buf[4096], *tmp, *magic = NULL;
   struct stat st;
   const char *p1, *p2;
   const char *pkg_bin_p = NULL;
   const char *pkg_lib_p = NULL;
   const char *pkg_data_p = NULL;
   const char *pkg_locale_p = NULL;
   const char *bindir = "bin";
   const char *libdir = "lib";
   const char *datadir = "share";
   const char *localedir = "share";

   pfx = calloc(1, sizeof(Eina_Prefix));
   if (!pfx) return NULL;
   
   /* if provided with a share dir use datadir/sharedir as the share dir */
   if (sharedir)
     {
        int len;

        len = snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", datadir, sharedir);
        if (len > 0)
          {
#ifdef _WIN32
             /* on win32 convert / to \ for path here */
             for (p = buf + strlen(datadir) + strlen(DSEP_S); *p; p++)
               {
                  if (*p == '/') *p = DSEP_C;
               }
#endif
             tmp = alloca(len + 1);
             strcpy(tmp, buf);
             datadir = tmp;
          }
     }
   if (magicsharefile)
     {
        magic = alloca(strlen(magicsharefile));
        strcpy(magic, magicsharefile);
#ifdef _WIN32
        /* on win32 convert / to \ for path here */
        for (p = magic; *p; p++)
          {
             if (*p == '/') *p = DSEP_C;
          }
#endif
     }

   /* look at compile-time package bin/lib/datadir etc. and figure out the
    * bin, lib and data dirs from these, if possible. i.e.
    *   bin = /usr/local/bin
    *   lib = /usr/local/lib
    *   data = /usr/local/share/enlightenment
    * thus they all have a common prefix string of /usr/local/ and
    *   bindir = bin
    *   libdir = lib
    *   datadir = share/enlightenment
    * this addresses things like libdir is lib64 or lib32 or other such
    * junk distributions like to do so then:
    *   bin = /usr/local/bin
    *   lib = /usr/local/lib64
    *   data = /usr/local/share/enlightenment
    * then
    *   bindir = bin
    *   libdir = lib64
    *   datadir = share/enlightennment
    * all with a common prefix that can be relocated
    */
   /* 1. check last common char in bin and lib strings */
   for (p1 = pkg_bin, p2 = pkg_lib; *p1 && *p2; p1++, p2++)
     {
        if (*p1 != *p2)
          {
             pkg_bin_p = p1;
             pkg_lib_p = p2;
             break;
          }
     }
   /* 1. check last common char in bin and data strings */
   for (p1 = pkg_bin, p2 = pkg_data; *p1 && *p2; p1++, p2++)
     {
        if (*p1 != *p2)
          {
             pkg_data_p = p2;
             break;
          }
     }
   /* 1. check last common char in bin and locale strings */
   for (p1 = pkg_bin, p2 = pkg_locale; *p1 && *p2; p1++, p2++)
     {
        if (*p1 != *p2)
          {
             pkg_locale_p = p2;
             break;
          }
     }
   /* 2. if all the common string offsets match we compiled with a common prefix */
   if (((pkg_bin_p - pkg_bin) == (pkg_lib_p - pkg_lib))
       && ((pkg_bin_p - pkg_bin) == (pkg_data_p - pkg_data))
       && ((pkg_bin_p - pkg_bin) == (pkg_locale_p - pkg_locale))
      )
     {
        bindir = pkg_bin_p;
        libdir = pkg_lib_p;
        datadir = pkg_data_p;
        localedir = pkg_locale_p;
     }
   /* 3. some galoot thought it awesome not to give us a common prefix at compile time
    * so fall back to the compile time directories. we are no longer relocatable */
   else
     {
        STRDUP_REP(pfx->prefix_path_bin, pkg_bin);
        STRDUP_REP(pfx->prefix_path_lib, pkg_lib);
        STRDUP_REP(pfx->prefix_path_data, pkg_data);
        STRDUP_REP(pfx->prefix_path_locale, pkg_locale);
        pfx->no_common_prefix = 1;
     }
   
   /* if user provides env vars - then use that or also more specific sub
    * dirs for bin, lib, data and locale */
   if ((envprefix) &&
       (_get_env_vars(pfx, envprefix, bindir, libdir, datadir, localedir) > 0))
     {
        pfx->env_used = 1;
        return pfx;
     }
   
#ifdef HAVE_DLADDR
   if (symbol)
     {
        Dl_info info_dl;
        
        if (dladdr(symbol, &info_dl))
          {
             STRDUP_REP(pfx->exe_path, info_dl.dli_fname);
          }
     }
#endif   
   /* no env var - examine process and possible argv0 */
   if ((argv0) && (!pfx->exe_path) && (symbol))
     {
#ifndef _WIN32
        if (!_try_proc(pfx, symbol))
          {
#endif             
             if (!_try_argv(pfx, argv0))
               {
                  _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale,
                            envprefix);
                  return pfx;
               }
#ifndef _WIN32
          }
#endif        
     }
   if (!pfx->exe_path)
     {
        _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale, envprefix);
        return pfx;
     }
   /* _exe_path is now a full absolute path TO this exe - figure out rest */
   /*   if
    * exe        = /blah/whatever/bin/exe
    *   or
    * exe        = /blah/whatever/lib/libexe.so
    *   then
    * prefix     = /blah/whatever
    * bin_dir    = /blah/whatever/bin
    * data_dir   = /blah/whatever/share/enlightenment
    * lib_dir    = /blah/whatever/lib
    */
   p = strrchr(pfx->exe_path, DSEP_C);
   if (p)
     {
	p--;
	while (p >= pfx->exe_path)
	  {
	     if (*p == DSEP_C)
	       {
		  pfx->prefix_path = malloc(p - pfx->exe_path + 1);
		  if (pfx->prefix_path)
		    {
		       strncpy(pfx->prefix_path, pfx->exe_path, 
                               p - pfx->exe_path);
		       pfx->prefix_path[p - pfx->exe_path] = 0;

		       /* bin */
		       snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", 
                                pfx->prefix_path, bindir);
                       STRDUP_REP(pfx->prefix_path_bin, buf);
		       /* lib */
		       snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", 
                                pfx->prefix_path, libdir);
                       STRDUP_REP(pfx->prefix_path_lib, buf);
		       /* locale */
		       snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", 
                                pfx->prefix_path, localedir);
                       STRDUP_REP(pfx->prefix_path_locale, buf);
		       /* check if magic file is there - then our guess is right */
                       if (magic)
                          snprintf(buf, sizeof(buf), 
                                   "%s" DSEP_S "%s" DSEP_S "%s", 
                                   pfx->prefix_path, datadir, magic);
		       if ((!magic) || (stat(buf, &st) == 0))
			 {
			    snprintf(buf, sizeof(buf), "%s" DSEP_S "%s", 
                                     pfx->prefix_path, datadir);
                            STRDUP_REP(pfx->prefix_path_data, buf);
			 }
		       /* magic file not there. time to start hunting! */
		       else
                          _fallback(pfx, pkg_bin, pkg_lib, pkg_data, 
                                    pkg_locale, envprefix);
		    }
		  else
                     _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale, 
                               envprefix);
                  return pfx;
	       }
	     p--;
	  }
     }
   _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale, envprefix);
   return pfx;
}

EAPI void
eina_prefix_free(Eina_Prefix *pfx)
{
   if (!pfx) return;
   
   IF_FREE_NULL(pfx->exe_path);
   IF_FREE_NULL(pfx->prefix_path);
   IF_FREE_NULL(pfx->prefix_path_bin);
   IF_FREE_NULL(pfx->prefix_path_data);
   IF_FREE_NULL(pfx->prefix_path_lib);
   IF_FREE_NULL(pfx->prefix_path_locale);
   free(pfx);
}

EAPI const char *
eina_prefix_get(Eina_Prefix *pfx)
{
   if (!pfx) return "";
   return pfx->prefix_path;
}

EAPI const char *
eina_prefix_bin_get(Eina_Prefix *pfx)
{
   if (!pfx) return "";
   return pfx->prefix_path_bin;
}

EAPI const char *
eina_prefix_lib_get(Eina_Prefix *pfx)
{
   if (!pfx) return "";
   return pfx->prefix_path_lib;
}

EAPI const char *
eina_prefix_data_get(Eina_Prefix *pfx)
{
   if (!pfx) return "";
   return pfx->prefix_path_data;
}

EAPI const char *
eina_prefix_locale_get(Eina_Prefix *pfx)
{
   if (!pfx) return "";
   return pfx->prefix_path_locale;
}
