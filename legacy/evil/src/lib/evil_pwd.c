

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <windows.h>
#include <security.h>

#include "Evil.h"
#include "pwd.h"


#ifndef __CEGCC__


static struct passwd pw;

struct passwd *
getpwuid (uid_t uid)
{
   static char user_name[PATH_MAX];
   TCHAR       name[PATH_MAX];
   ULONG       length;
   BOOLEAN     res;
#ifdef UNICODE
   char       *a_name;
# endif /* UNICODE */

   length = PATH_MAX;
   /* get from USERPROFILE for win 98 ? */
#ifdef _WIN32_WINNT
   res = GetUserNameEx(NameDisplay, name, &length);
#else
   res = GetUserNameEx(NameWindowsCeLocal, name, &length);
#endif
#ifdef UNICODE
   if (res)
     {
        a_name = evil_wchar_to_char(name);
        if (a_name)
          {
             int length;

             length = strlen(a_name);
             if (length >= PATH_MAX)
               length = PATH_MAX;
             memcpy(user_name, a_name, length);
             user_name[length] = '\0';
             free(a_name);
          }
        else
          res = 0;
     }
#endif /* UNICODE */
   pw.pw_name = (res ? user_name : NULL);
   pw.pw_passwd = NULL;
   pw.pw_uid = uid;
   pw.pw_gid = 0;
   pw.pw_change = 0;
   pw.pw_class = NULL;
   pw.pw_gecos = (res ? user_name : NULL);
   pw.pw_dir = (char *)evil_homedir_get();
   pw.pw_shell = getenv("SHELL");
   if (pw.pw_shell == NULL)
     pw.pw_shell = "sh";
   pw.pw_expire = 0;
   pw.pw_fields = 0;

   return &pw;
}


#endif /* ! __CEGCC__ */
