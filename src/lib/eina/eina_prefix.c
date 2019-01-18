/* EINA - EFL data type library
 * Copyright (C) 2011 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#if defined HAVE_DLADDR && ! defined _WIN32
# include <dlfcn.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"
#include "eina_log.h"
#include "eina_str.h"
#include "eina_file.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_prefix.h"

#ifdef _WIN32
# define PSEP_C ';'
#else
# define PSEP_C ':'
#endif /* _WIN32 */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

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

#define STRDUP_REP(x, y) do { if (x) free(x); x = strdup(y); } while (0)
#define IF_FREE_NULL(p) do { if (p) { free(p); p = NULL; } } while (0)

#ifndef EINA_LOG_COLOR_DEFAULT
#define EINA_LOG_COLOR_DEFAULT EINA_COLOR_CYAN
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_prefix_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_prefix_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_prefix_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_prefix_log_dom, __VA_ARGS__)

static int _eina_prefix_log_dom = -1;

static int
_path_join_multiple(char *buf, int bufsize, ...)
{
   va_list ap;
   int used = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, -1);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(bufsize < 1, -1);

   va_start(ap, bufsize);
   while (used < bufsize - 1)
     {
        const char *comp = va_arg(ap, const char *);
        int complen, seplen;
        if (!comp) break;


        seplen = (used == 0) ? 0 : 1;
        complen = strlen(comp);
        if (seplen + complen >= bufsize -1)
          {
             va_end(ap);
             buf[0] = '\0';
             return -1;
          }

        if (used > 0)
          buf[used] = EINA_PATH_SEP_C;

        memcpy(buf + used + seplen, comp, complen);
        used += complen + seplen;
     }
   va_end(ap);
   buf[used] = '\0';
   return used;
}

static void
_path_sep_fix(char *buf)
{
#ifdef _WIN32
   for (; *buf != '\0'; buf++)
     {
        if (*buf == '/')
          *buf = EINA_PATH_SEP_C;
     }
#else
   (void)buf;
#endif
}

static Eina_Bool
_path_absolute_check(const char *path)
{
#ifdef _WIN32
   return evil_path_is_absolute(path);
#else
   return (path[0] == EINA_PATH_SEP_C);
#endif
}

static int
_fallback(Eina_Prefix *pfx, const char *pkg_bin, const char *pkg_lib,
          const char *pkg_data, const char *pkg_locale, const char *envprefix)
{
   char *p;

   STRDUP_REP(pfx->prefix_path, pkg_bin);
   if (!pfx->prefix_path) return 0;
   p = strrchr(pfx->prefix_path, EINA_PATH_SEP_C);
   if (p) *p = 0;
   STRDUP_REP(pfx->prefix_path_bin, pkg_bin);
   STRDUP_REP(pfx->prefix_path_lib, pkg_lib);
   STRDUP_REP(pfx->prefix_path_data, pkg_data);
   STRDUP_REP(pfx->prefix_path_locale, pkg_locale);
   WRN("Could not determine its installed prefix for '%s'\n"
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
       "        %s_LOCALE_DIR - provide a specific locale directory",
       envprefix,
       pfx->prefix_path, pkg_bin, pkg_lib, pkg_data, pkg_locale,
       envprefix, envprefix, envprefix, envprefix, envprefix);
   pfx->fallback = 1;
   return 1;
}

static int
_try_proc(Eina_Prefix *pfx, void *symbol)
{
#ifndef _WIN32
   FILE *f;
   char buf[4096];

   f = fopen("/proc/self/maps", "rb");
   if (!f)
     {
        WRN("Couldn't read /proc/self/maps to lookup symbol=%p", symbol);
        return 0;
     }
   DBG("Check /proc/self/maps for symbol=%p", symbol);
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
                            INF("Found %p in /proc/self/maps: %s (%s)", symbol, pfx->exe_path, buf);
                            fclose(f);
                            return 1;
                         }
                       else
                         {
                            DBG("Found %p in /proc/self/maps but not a file (%s)", symbol, buf);
                            break;
                         }
                    }
               }
          }
     }
   fclose(f);
   WRN("Couldn't find symbol %p in a file in /proc/self/maps", symbol);
   return 0;
#else
   return 0;
   (void)pfx;
   (void)symbol;
#endif
}

static int
_try_argv(Eina_Prefix *pfx, const char *argv0)
{
   char *path, *p, *cp;
   int len, lenexe;
   char buf[PATH_MAX], buf2[PATH_MAX];

   /* 1. is argv0 abs path? */
   if (_path_absolute_check(argv0))
     {
        if (access(argv0, X_OK) == 0)
          {
             INF("Executable argv0 is full path = %s", argv0);
             STRDUP_REP(pfx->exe_path, argv0);
             return 1;
          }
        WRN("Non executable argv0: %s", argv0);
        return 0;
     }

   /* 2. relative path */
   if (strchr(argv0, EINA_PATH_SEP_C))
     {
        if (getcwd(buf2, sizeof(buf2)))
          {
             char joined[PATH_MAX];
             eina_file_path_join(joined, sizeof(joined), buf2, argv0);
             if (realpath(joined, buf))
               {
                  if (access(buf, X_OK) == 0)
                    {
                       INF("Executable relative argv0=%s, cwd=%s, realpath=%s",
                           argv0, buf2, buf);
                       STRDUP_REP(pfx->exe_path, buf);
                       return 1;
                    }
                  WRN("Non executable relative argv0=%s, cwd=%s, realpath=%s",
                      argv0, buf2, buf);
               }
             else
               WRN("No realpath for argv0=%s, cwd=%s", argv0, buf2);
          }
        else
          WRN("Couldn't get current directory to lookup argv0=%s", argv0);
     }

   /* 3. argv0 no path - look in PATH */
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() != geteuid()) return 0;
#endif
   path = getenv("PATH");
   if (!path)
     {
        DBG("No env PATH to lookup argv0=%s", argv0);
        return 0;
     }
   p = path;
   cp = p;
   lenexe = strlen(argv0);
   while ((p = strchr(cp, PSEP_C)))
     {
        len = p - cp;
        if ((len == 0) || (len + lenexe + 2 >= (int)sizeof(buf2)))
          {
             cp = p + 1;
             continue;
          }

        strncpy(buf2, cp, len);
        buf2[len] = EINA_PATH_SEP_C;
        strcpy(buf2 + len + 1, argv0);
        if (realpath(buf2, buf))
          {
             if (access(buf, X_OK) == 0)
               {
                  STRDUP_REP(pfx->exe_path, buf);
                  INF("Path %s is executable", pfx->exe_path);
                  return 1;
               }
             else
               DBG("Path not executable %s", buf);
          }
        else
          DBG("No realpath for argv0=%s in %.*s", argv0, len, cp);
        cp = p + 1;
     }
   /* 4. big problems. arg[0] != executable - weird execution */
   WRN("Couldn't find argv0=%s in current directory or env PATH=%s",
       argv0, path);
   return 0;
}

static int
_get_env_var(char **var, const char *envprefix, const char *envsuffix, const char *prefix, const char *dir)
{
   char env[1024];
   const char *s;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() != geteuid()) return 0;
#endif
   snprintf(env, sizeof(env), "%s_%s_DIR", envprefix, envsuffix);
   s = getenv(env);
   if (s)
     {
        INF("Have prefix env %s = %s", env, s);
        STRDUP_REP(*var, s);
        return 1;
     }
   else if (prefix)
     {
        char buf[PATH_MAX];
        eina_file_path_join(buf, sizeof(buf), prefix, dir);
        INF("Have %s_PREFIX = %s, use %s = %s", envprefix, prefix, env, buf);
        STRDUP_REP(*var, buf);
        return 1;
     }
   else
     {
        DBG("No env %s_PREFIX or %s for dir '%s'", envprefix, env, dir);
        STRDUP_REP(*var, "");
        return 0;
     }
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
   const char *prefix;
   int ret = 0;

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        snprintf(env, sizeof(env), "%s_PREFIX", envprefix);
        if ((prefix = getenv(env))) STRDUP_REP(pfx->prefix_path, prefix);

        ret += _get_env_var(&pfx->prefix_path_bin, envprefix, "BIN", prefix, bindir);
        ret += _get_env_var(&pfx->prefix_path_lib, envprefix, "LIB", prefix, libdir);
        ret += _get_env_var(&pfx->prefix_path_data, envprefix, "DATA", prefix, datadir);
        ret += _get_env_var(&pfx->prefix_path_locale, envprefix, "LOCALE", prefix, localedir);
     }
   return ret;
}

static int
_common_prefix_find(const char *bin, const char *lib, const char *data, const char *locale)
{
   const char *b = bin;
   const char *i = lib;
   const char *d = data;
   const char *o = locale;

   for (; (*b) && (*i) && (*d) && (*o); b++, i++, d++, o++)
     {
        if (*b != *i) break;
        if (*b != *d) break;
        if (*b != *o) break;
     }
   return b - bin;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI Eina_Prefix *
eina_prefix_new(const char *argv0, void *symbol, const char *envprefix,
                const char *sharedir, const char *magicsharefile,
                const char *pkg_bin, const char *pkg_lib,
                const char *pkg_data, const char *pkg_locale)
{
   Eina_Prefix *pfx;
   char *p, buf[4096], *tmp, *magic = NULL;
   struct stat st;
   int prefixlen;
   const char *bindir = "bin";
   const char *libdir = "lib";
   const char *datadir = "share";
   const char *localedir = "share";
   Eina_Bool from_lib = EINA_FALSE, from_bin = EINA_FALSE;

   DBG("EINA PREFIX: argv0=%s, symbol=%p, magicsharefile=%s, envprefix=%s",
       argv0, symbol, magicsharefile, envprefix);
   DBG("EINA PREFIX: share=%s, bin=%s, lib=%s, data=%s, locale=%s",
       sharedir, pkg_bin, pkg_lib, pkg_data, pkg_locale);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_bin, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_lib, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pkg_locale, NULL);

   pfx = calloc(1, sizeof(Eina_Prefix));
   if (!pfx) return NULL;

   /* if provided with a share dir use datadir/sharedir as the share dir */
   if (sharedir)
     {
        int len;

        len = eina_file_path_join(buf, sizeof(buf), datadir, sharedir);
        if (len > 0)
          {
             _path_sep_fix(buf + strlen(datadir) + strlen(EINA_PATH_SEP_S));
             tmp = alloca(len + 1);
             strcpy(tmp, buf);
             datadir = tmp;
          }
     }
   if (magicsharefile)
     {
        magic = alloca(strlen(magicsharefile) + 1);
        strcpy(magic, magicsharefile);
        _path_sep_fix(magic);
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
    * in theory this should also work with debians new multiarch style like
    *   bindir = bin
    *   libdir = lib/i386-linux-gnu
    *     or
    *   libdir = lib/x86_64-linux-gnu
    * all with a common prefix that can be relocated
    */
   prefixlen = _common_prefix_find(pkg_bin, pkg_lib, pkg_data, pkg_locale);
   if (prefixlen > 0)
     {
        bindir = pkg_bin + prefixlen;
        libdir = pkg_lib + prefixlen;
        datadir = pkg_data + prefixlen;
        localedir = pkg_locale + prefixlen;
        DBG("Prefix common=%.*s, bin=%s, lib=%s, data=%s, locale=%s",
            prefixlen, pkg_bin, bindir, libdir, datadir, localedir);
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
        DBG("Can't work out a common prefix - compiled in fallback");
     }

   /* if user provides env vars - then use that or also more specific sub
    * dirs for bin, lib, data and locale */
   if ((envprefix) &&
       (_get_env_vars(pfx, envprefix, bindir, libdir, datadir, localedir) > 0))
     {
        pfx->env_used = 1;
        return pfx;
     }

   if (symbol)
     {
#ifdef HAVE_DLADDR
        Dl_info info_dl;

        if (dladdr(symbol, &info_dl))
          {
             if (info_dl.dli_fname)
               {
                  if (_path_absolute_check(info_dl.dli_fname))
                    {
                       INF("dladdr for symbol=%p: %s", symbol, info_dl.dli_fname);
                       char *rlink = realpath(info_dl.dli_fname, NULL);
                       if (rlink)
                         {
                            IF_FREE_NULL(pfx->exe_path);
                            pfx->exe_path = rlink;
                         }
                       else
                         {
                            STRDUP_REP(pfx->exe_path, info_dl.dli_fname);
                         }
                       from_lib = EINA_TRUE;
                    }
                  else
                    WRN("dladdr for symbol=%p: %s is relative", symbol, info_dl.dli_fname);
               }
             else
               WRN("no dladdr filename for symbol=%p", symbol);
          }
        else
          WRN("no dladdr for symbol=%p", symbol);
#endif

        if (!pfx->exe_path)
          _try_proc(pfx, symbol);
        /* no from_lib/from_bin as we're not sure it came from lib or bin! */
     }

   /* no env var or symbol - examine process and possible argv0 */
   if ((argv0) && (!pfx->exe_path))
     {
        if (!_try_argv(pfx, argv0))
          {
             WRN("Fallback - couldn't resolve based on argv0=%s", argv0);
             _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale,
                       envprefix);
             return pfx;
          }

        from_bin = EINA_TRUE;
     }

   if (!pfx->exe_path)
     {
        WRN("Fallback - no variables, symbol or argv0 could be used.");
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
    *
    * new case - debian multiarch goop.
    * exe        = /blah/whatever/lib/arch/libexe.so
    */
   DBG("From exe %s figure out the rest", pfx->exe_path);
   p = strrchr(pfx->exe_path, EINA_PATH_SEP_C);
   if (p)
     {
        p--;
        while (p >= pfx->exe_path)
          {
             if (*p == EINA_PATH_SEP_C)
               {
                  if (pfx->prefix_path) free(pfx->prefix_path);
                  pfx->prefix_path = malloc(p - pfx->exe_path + 1);
                  if (pfx->prefix_path)
                    {
                       Eina_Bool magic_found = EINA_FALSE;
                       int checks_passed = 0;

                       strncpy(pfx->prefix_path, pfx->exe_path,
                               p - pfx->exe_path);
                       pfx->prefix_path[p - pfx->exe_path] = 0;
                       DBG("Have prefix = %s", pfx->prefix_path);

                       /* bin */
                       eina_file_path_join(buf, sizeof(buf), pfx->prefix_path, bindir);
                       STRDUP_REP(pfx->prefix_path_bin, buf);
                       DBG("Have bin = %s", pfx->prefix_path_bin);
                       if ((!from_bin) && (stat(buf, &st) == 0))
                         checks_passed++;

                       /* lib */
                       eina_file_path_join(buf, sizeof(buf), pfx->prefix_path, libdir);
                       STRDUP_REP(pfx->prefix_path_lib, buf);
                       DBG("Have lib = %s", pfx->prefix_path_lib);
                       if ((!from_lib) && (stat(buf, &st) == 0))
                         checks_passed++;

                       /* locale */
                       eina_file_path_join(buf, sizeof(buf), pfx->prefix_path, localedir);
                       STRDUP_REP(pfx->prefix_path_locale, buf);
                       DBG("Have locale = %s", pfx->prefix_path_locale);
                       if (stat(buf, &st) == 0)
                         checks_passed++;

                       /* check if magic file is there - then our guess is right */
                       if (!magic)
                         DBG("No magic file");
                       else
                         {
                            DBG("Magic = %s", magic);
                            _path_join_multiple(buf, sizeof(buf),
                                                pfx->prefix_path,
                                                datadir,
                                                magic, NULL);
                            DBG("Check in %s", buf);

                            if (stat(buf, &st) == 0)
                              {
                                 checks_passed++;
                                 magic_found = EINA_TRUE;
                                 DBG("Magic path %s stat passed", buf);
                              }
                            else
                              WRN("Missing magic path %s", buf);
                         }

                       if (((!magic) && (checks_passed > 0)) ||
                           ((magic) && (magic_found)))
                         {
                            eina_file_path_join(buf, sizeof(buf), pfx->prefix_path, datadir);
                            STRDUP_REP(pfx->prefix_path_data, buf);
                         }
                       else
                         {
                            for (;p > pfx->exe_path; p--)
                              {
                                 if (*p == EINA_PATH_SEP_C)
                                   {
                                      p--;
                                      break;
                                   }
                              }
                            if (p > pfx->exe_path)
                              {
                                 int newlen = p - pfx->exe_path;
                                 DBG("Go back one directory (%.*s)", newlen, pfx->exe_path);
                                 continue;
                              }
                            WRN("No Prefix path (exhausted search depth)");
                            _fallback(pfx, pkg_bin, pkg_lib, pkg_data,
                                      pkg_locale, envprefix);
                         }
                    }
                  else
                    {
                       WRN("No Prefix path (alloc fail)");
                       _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale,
                                 envprefix);
                    }
                  return pfx;
               }
             p--;
          }
     }
   WRN("Final fallback");
   _fallback(pfx, pkg_bin, pkg_lib, pkg_data, pkg_locale, envprefix);
   return pfx;
}

EAPI void
eina_prefix_free(Eina_Prefix *pfx)
{
   EINA_SAFETY_ON_NULL_RETURN(pfx);

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
   EINA_SAFETY_ON_NULL_RETURN_VAL(pfx, "");
   return pfx->prefix_path;
}

EAPI const char *
eina_prefix_bin_get(Eina_Prefix *pfx)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pfx, "");
   return pfx->prefix_path_bin;
}

EAPI const char *
eina_prefix_lib_get(Eina_Prefix *pfx)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pfx, "");
   return pfx->prefix_path_lib;
}

EAPI const char *
eina_prefix_data_get(Eina_Prefix *pfx)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pfx, "");
   return pfx->prefix_path_data;
}

EAPI const char *
eina_prefix_locale_get(Eina_Prefix *pfx)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pfx, "");
   return pfx->prefix_path_locale;
}

Eina_Bool
eina_prefix_init(void)
{
   _eina_prefix_log_dom = eina_log_domain_register("eina_prefix",
                                                   EINA_LOG_COLOR_DEFAULT);
   if (_eina_prefix_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_prefix");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
eina_prefix_shutdown(void)
{
   eina_log_domain_unregister(_eina_prefix_log_dom);
   _eina_prefix_log_dom = -1;
   return EINA_TRUE;
}
