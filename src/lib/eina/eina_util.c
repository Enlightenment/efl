/* EINA - EFL data type library
 * Copyright (C) 2015 Vincent Torri
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

#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
# include <string.h>
#else
# include <sys/types.h>
# include <pwd.h>
# include <string.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_tmpstr.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI const char *
eina_environment_home_get(void)
{
   static char *home = NULL;

   if (home) return home;
#ifdef _WIN32
   home = getenv("USERPROFILE");
   if (!home) home = getenv("WINDIR");
   if (!home &&
       (getenv("HOMEDRIVE") && getenv("HOMEPATH")))
     {
        char buf[PATH_MAX];

        snprintf(buf, sizeof(buf), "%s%s",
                 getenv("HOMEDRIVE"), getenv("HOMEPATH"));
        home = strdup(buf);
        return home;
     }
   if (!home) home = "C:\\";
#else
# if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid()) home = getenv("HOME");
# endif
   if (!home)
     {
# ifdef HAVE_GETPWENT
        struct passwd pwent, *pwent2 = NULL;
        char pwbuf[8129];

        if (!getpwuid_r(geteuid(), &pwent, pwbuf, sizeof(pwbuf), &pwent2))
          {
             if ((pwent2) && (pwent.pw_dir))
               {
                  home = strdup(pwent.pw_dir);
                  return home;
               }
          }
# endif
        home = "/tmp";
     }
#endif
   home = strdup(home);
   return home;
}

EAPI const char *
eina_environment_tmp_get(void)
{
   static char *tmp = NULL;

   if (tmp) return tmp;
#ifdef _WIN32
   tmp = getenv("TMP");
   if (!tmp) tmp = getenv("TEMP");
   if (!tmp) tmp = getenv("USERPROFILE");
   if (!tmp) tmp = getenv("WINDIR");
   if (!tmp) tmp = "C:\\";
#else
# if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
# endif
     {
        tmp = getenv("TMPDIR");
        if (!tmp) tmp = getenv("TMP");
        if (!tmp) tmp = getenv("TEMPDIR");
        if (!tmp) tmp = getenv("TEMP");
     }
   if (!tmp) tmp = "/tmp";
#endif

#if defined(__MACH__) && defined(__APPLE__)
   if (tmp && tmp[strlen(tmp) -1] == '/')
     {
        char *tmp2 = strdup(tmp);
        tmp2[strlen(tmp2) - 1] = 0x0;
        tmp = tmp2;
        return tmp;
     }
#endif

   tmp = strdup(tmp);
   return tmp;
}
