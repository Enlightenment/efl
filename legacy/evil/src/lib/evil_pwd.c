

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "pwd.h"


#ifndef __CEGCC__


static struct passwd pw;

struct passwd *
getpwuid (uid_t uid)
{
   static char user_name[PATH_MAX];
   DWORD  length;
   BOOL res;

   length = PATH_MAX;
   /* get from USERPROFILE for win 98 ? */
   res = GetUserName(user_name, &length);
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
