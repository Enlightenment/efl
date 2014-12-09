#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#define _POSIX
#include <io.h>
#include <lmcons.h>

#include "Evil.h"
#include "pwd.h"


static struct passwd pw;

struct passwd *
getpwnam(const char *n)
{
   static char user_name[UNLEN + 1];
   TCHAR       name[UNLEN + 1];
   DWORD       length;
   BOOLEAN     res;
#ifdef UNICODE
   char       *a_name;
# endif /* UNICODE */

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

   pw.pw_name = (res ? user_name : NULL);
   pw.pw_passwd = NULL;
   pw.pw_uid = 0;
   pw.pw_gid = 0;
   pw.pw_change = 0;
   pw.pw_class = NULL;
   pw.pw_gecos = (res ? user_name : NULL);
   pw.pw_dir = (char *)evil_homedir_get();
   pw.pw_shell = getenv("SHELL");
   if (!pw.pw_shell)
     pw.pw_shell = "sh";
   pw.pw_expire = 0;
   pw.pw_fields = 0;

   return &pw;
}

struct passwd *
getpwuid(uid_t uid)
{
   return getpwnam(getlogin());
   (void)uid;
}
