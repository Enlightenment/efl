#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#define _POSIX
#include <io.h>
#include <lmcons.h>

#include "evil_private.h"
#include "pwd.h"


static struct passwd pw = { NULL, NULL, 0, 0, 0, NULL, NULL, NULL, NULL, 0, 0 };

struct passwd *
getpwnam(const char *n)
{
   static char user_name[UNLEN + 1];
   static char user_gecos[UNLEN + 4];
   TCHAR       name[UNLEN + 1];
   DWORD       length;
   BOOLEAN     res;
#ifdef UNICODE
   char       *a_name;
# endif /* UNICODE */

   if (!n)
     return NULL;

   length = UNLEN + 1;
   res = GetUserName(name, &length);
   if (!res)
     return NULL;

#ifdef UNICODE
   a_name = evil_wchar_to_char(name);
   if (a_name)
     {
        int l;

        l = strlen(a_name);
        if (l >= PATH_MAX)
          l = PATH_MAX;
        memcpy(user_name, a_name, l);
        user_name[l] = '\0';
        free(a_name);
     }
   else
     return NULL;
#else
   memcpy(user_name, name, strlen(name) + 1);
#endif /* UNICODE */

   if (strcmp(n, user_name) != 0)
     return NULL;

   pw.pw_name = user_name;
   snprintf(user_gecos, sizeof(user_gecos), "%s,,,", user_name);
   pw.pw_gecos = user_gecos;
   pw.pw_dir = getenv("USERPROFILE");
   pw.pw_shell = getenv("SHELL");
   if (!pw.pw_shell)
     pw.pw_shell = "cmd.exe";

   return &pw;
}

struct passwd *
getpwuid(uid_t uid)
{
   return getpwnam(getlogin());
   (void)uid;
}
