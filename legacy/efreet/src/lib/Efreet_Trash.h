#ifndef EFREET_TRASH_H
#define EFREET_TRASH_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFREET_TRASH_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFREET_TRASH_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file Efreet_Trash.h
 * @brief Contains the methods used to support the FDO trash specification.
 * @addtogroup Efreet_Trash Efreet_Trash: The XDG Trash Specification
 * Efreet_Trash.h provides all of the necessary headers and includes to
 * work with Efreet_Trash.
 * @{
 */

EAPI int         efreet_trash_init(void);
EAPI int         efreet_trash_shutdown(void);

EAPI const char *efreet_trash_dir_get(const char *for_file);
EAPI int         efreet_trash_delete_uri(Efreet_Uri *uri, int force_delete);
EAPI Eina_List  *efreet_trash_ls(void);
EAPI int         efreet_trash_is_empty(void);
EAPI int         efreet_trash_empty_trash(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
