#ifndef __EVIL_PWD_H__
#define __EVIL_PWD_H__


/**
 * @file pwd.h
 * @brief The file that provides functions ported from Unix in pwd.h.
 * @defgroup Evil_Pwd_Group Pwd.h functions
 *
 * This header provides functions ported from Unix in dirent.h.
 *
 * @{
 */


#include <time.h>

#include <Evil.h>


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
   uid_t    pw_uid;        /**< user uid */
   gid_t    pw_gid;        /**< user gid (always O) */
   time_t   pw_change;     /**< password change time (always 0) */
   char    *pw_class;      /**< user access class (always @c NULL) */
   char    *pw_gecos;      /**< Honeywell login info */
   char    *pw_dir;        /**< home directory */
   char    *pw_shell;      /**< default shell */
   time_t   pw_expire;     /**< account expiration (always O) */
   int      pw_fields;     /**< internal: fields filled in (always O) */
};

/**
 * @brief Return a passwd structure.
 *
 * @param uid The User ID
 * @return A stacally allocated passwd structure.
 *
 * This function fills a static buffer @ref passwd with @p uid and the
 * user name.
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI struct passwd *getpwuid (uid_t uid);


#ifdef __cplusplus
}
#endif



/**
 * @}
 */


#endif /* __EVIL_PWD_H__ */
