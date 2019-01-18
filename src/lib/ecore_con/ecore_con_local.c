#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <libgen.h>

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_Con.h"
#include "ecore_con_private.h"

EAPI char *
ecore_con_local_path_new(Eina_Bool is_system, const char *name, int port)
{
#if _WIN32
   char buf[256 - sizeof(PIPE_NS)] = "";

   /* note: using '!' instead of '|' since at least on wine '|' causes
    * ERROR_INVALID_NAME
    */

   if (!is_system)
     {
        TCHAR user[sizeof(buf) - sizeof("ecore!u!n!1")] = "unknown";
        DWORD userlen = sizeof(user);
        if (!GetUserName(user, &userlen))
          {
             char *msg = _efl_net_windows_error_msg_get(GetLastError());
             ERR("GetUserName(%p, %lu): %s", user, userlen, msg);
             free(msg);
          }
        if (port < 0)
          snprintf(buf, sizeof(buf), "ecore!%s!%s", user, name);
        else
          snprintf(buf, sizeof(buf), "ecore!%s!%s!%d", user, name, port);
     }
   else
     {
        if (port < 0)
          snprintf(buf, sizeof(buf), "ecore_service!%s", name);
        else
          snprintf(buf, sizeof(buf), "ecore_service!%s!%d", name, port);
     }
   return strdup(buf);
#else
   char buf[4096];
   const char *homedir;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   if (!is_system)
     {
        if (port < 0)
          eina_vpath_resolve_snprintf(buf, sizeof(buf), "(:usr.run:)/.ecore/%s", name);
        else
          eina_vpath_resolve_snprintf(buf, sizeof(buf), "(:usr.run:)/.ecore/%s/%i", name, port);

        return strdup(buf);
     }
   else
     {
        if (port < 0)
          {
             if (name[0] == '/')
               return strdup(name);
             else
               {
                  homedir = eina_environment_tmp_get();
                  snprintf(buf, sizeof(buf), "%s/.ecore_service|%s",
                           homedir, name);
                  return strdup(buf);
               }
          }
        else
          {
             if (name[0] == '/')
               snprintf(buf, sizeof(buf), "%s|%i", name, port);
             else
               {
                  homedir = eina_environment_tmp_get();
                  snprintf(buf, sizeof(buf), "%s/.ecore_service|%s|%i",
                           homedir, name, port);
               }
             return strdup(buf);
          }
     }
#endif
}

void
_ecore_con_local_mkpath(const char *path, mode_t mode)
{
   char *s, *d, *itr;

   if (!path) return;
   EINA_SAFETY_ON_TRUE_RETURN(path[0] != '/');

   s = strdup(path);
   EINA_SAFETY_ON_NULL_RETURN(s);
   d = dirname(s);
   EINA_SAFETY_ON_NULL_RETURN(d);

   for (itr = d + 1; *itr != '\0'; itr++)
     {
        if (*itr == '/')
          {
             *itr = '\0';
             if (mkdir(d, mode) != 0)
               {
                  if (errno != EEXIST)
                    {
                       ERR("could not create parent directory '%s' of path '%s': %s", d, path, strerror(errno));
                       goto end;
                    }
               }
             *itr = '/';
          }
     }

   if (mkdir(d, mode) != 0)
     {
        if (errno != EEXIST)
          ERR("could not create parent directory '%s' of path '%s': %s", d, path, strerror(errno));
        else
          {
             struct stat st;
             if ((stat(d, &st) != 0) || (!S_ISDIR(st.st_mode)))
               ERR("could not create parent directory '%s' of path '%s': exists but is not a directory", d, path);
          }
     }

 end:
   free(s);
}
