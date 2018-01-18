#ifndef __EVIL_PWD_H__
#define __EVIL_PWD_H__


/**
 * @file pwd.h
 * @brief The file that provides functions ported from Unix in pwd.h.
 * @defgroup Evil_Pwd_Group Pwd.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in pwd.h.
 *
 * @{
 */


#include <time.h>

#include <Evil.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @struct passwd
 * @brief A structure that describes a password.
 */
struct passwd {
   char    *pw_name;       /**< user name */
   char    *pw_passwd;     /**< encrypted password (always @c NULL) */
   uid_t    pw_uid;        /**< user uid (always 0) */
   gid_t    pw_gid;        /**< user gid (always 0) */
   time_t   pw_change;     /**< password change time (always 0) */
   char    *pw_class;      /**< user access class (always @c NULL) */
   char    *pw_gecos;      /**< Honeywell login info */
   char    *pw_dir;        /**< home directory */
   char    *pw_shell;      /**< default shell */
   time_t   pw_expire;     /**< account expiration (always 0) */
   int      pw_fields;     /**< internal: fields filled in (always 0) */
};

/**
 * @brief Return a passwd structure.
 *
 * @param n The name of the user.
 * @return A stacally allocated passwd structure.
 *
 * This function fills a static buffer @ref passwd with the user name @p n.
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP.
 */
EAPI struct passwd *getpwnam(const char *n);

/**
 * @brief Return a passwd structure.
 *
 * @param uid The User ID.
 * @return A stacally allocated passwd structure.
 *
 * This function fills a static buffer @ref passwd with @p uid and the
 * user name.
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP.
 */
EAPI struct passwd *getpwuid (uid_t uid);


#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI


/**
 * @}
 */


#endif /* __EVIL_PWD_H__ */
