/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_TRASH_H
#define EFREET_TRASH_H

/**
 * @file efreet_trash.h
 * @brief Contains the methods used to support the FDO trash specification.
 * @addtogroup Efreet_Trash Efreet_Trash: The XDG Trash Specification
 * Some better explanation here...
 * @{
 */

#ifdef EAPI
#undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
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
 * @{
 */

/**
 * Efreet_Uri
 */
typedef struct Efreet_Uri Efreet_Uri;

/**
 * Efreet_Uri
 * @brief Contains a simple rappresentation of an uri. The string don't have 
 * special chars escaped.
 */
struct Efreet_Uri
{
    const char *protocol;   /**< The name of the host if any, or NULL */
    const char *hostname;   /**< The name of the host if any, or NULL */
    const char *path;       /**< The full file path whitout protocol nor host*/
};


EAPI const char *efreet_uri_escape(Efreet_Uri *uri);
EAPI Efreet_Uri *efreet_uri_parse(const char *val);
EAPI void        efreet_uri_free(Efreet_Uri *uri);

EAPI int         efreet_trash_init(void);
EAPI void        efreet_trash_shutdown(void);

EAPI const char *efreet_trash_dir_get(void);
EAPI int         efreet_trash_delete_uri(Efreet_Uri *uri, int force_delete);
EAPI Ecore_List *efreet_trash_ls(void);
EAPI int         efreet_trash_is_empty(void);
EAPI int         efreet_trash_empty_trash(void);
   
   
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
