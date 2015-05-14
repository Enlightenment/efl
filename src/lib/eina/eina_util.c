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

EAPI Eina_Tmpstr *
eina_environment_home_get(void)
{
#ifdef _WIN32
   char *home;

   home = getenv("USERPROFILE");
   if (!home) home = getenv("WINDIR");
   if (!home) home = "C:\\";

   return eina_tmpstr_add(home);
#else
   return eina_tmpstr_add(getenv("HOME"));
#endif
}

EAPI Eina_Tmpstr *
eina_environment_tmp_get(void)
{
   char *tmp;

#ifdef _WIN32
   tmp = getenv("TMP");
   if (!tmp) tmp = getenv("TEMP");
   if (!tmp) tmp = getenv("USERPROFILE");
   if (!tmp) tmp = getenv("WINDIR");
   if (!tmp) tmp = "C:\\";

   return eina_tmpstr_add(tmp);
#else
   tmp = getenv("TMPDIR");
   if (!tmp) tmp = getenv("XDG_RUNTIME_DIR");
   if (!tmp) tmp = "/tmp";

   return eina_tmpstr_add(tmp);
#endif
}
