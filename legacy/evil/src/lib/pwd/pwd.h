#ifndef __EVIL_PWD_H__
#define __EVIL_PWD_H__


#include <time.h>

#include <Evil.h>


#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef EFL_EVIL_PWD_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVIL_PWD_BUILD */
#endif /* _WIN32 */


#ifdef __cplusplus
extern "C" {
#endif


struct passwd {
   char    *pw_name;       /* user name */
   char    *pw_passwd;     /* encrypted password */
   uid_t    pw_uid;        /* user uid */
   gid_t    pw_gid;        /* user gid */
   time_t   pw_change;     /* password change time */
   char    *pw_class;      /* user access class */
   char    *pw_gecos;      /* Honeywell login info */
   char    *pw_dir;        /* home directory */
   char    *pw_shell;      /* default shell */
   time_t   pw_expire;     /* account expiration */
   int      pw_fields;     /* internal: fields filled in */
};

EAPI struct passwd * getpwuid (uid_t uid);


#ifdef __cplusplus
}
#endif


#endif /* __EVIL_PWD_H__ */
