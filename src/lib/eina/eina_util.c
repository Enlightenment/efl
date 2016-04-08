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

#ifdef _WIN32
static char home_storage[8];
#endif

EAPI const char *
eina_environment_home_get(void)
{
#ifdef _WIN32
   char *home;

   home = getenv("USERPROFILE");
   if (!home) home = getenv("WINDIR");
   if (!home &&
       (getenv("HOMEDRIVE") && getenv("HOMEPATH")))
     {
        memcpy(home_storage, getenv("HOMEDRIVE"), strlen(getenv("HOMEDRIVE")));
        memcpy(home_storage + strlen(getenv("HOMEDRIVE")),
               getenv("HOMEPATH"), strlen(getenv("HOMEPATH")) + 1);
        home = home_storage;
     }
   if (!home) home = "C:\\";

   return home;
#else
   return getenv("HOME");
#endif
}

EAPI const char *
eina_environment_tmp_get(void)
{
   char *tmp = NULL;

#ifdef _WIN32
   tmp = getenv("TMP");
   if (!tmp) tmp = getenv("TEMP");
   if (!tmp) tmp = getenv("USERPROFILE");
   if (!tmp) tmp = getenv("WINDIR");
   if (!tmp) tmp = "C:\\";

   return tmp;
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

   return tmp;
#endif
}
