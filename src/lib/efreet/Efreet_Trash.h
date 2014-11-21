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
 *
 * @brief Contains the methods used to support the FDO trash specification.
 *
 * @internal
 * @defgroup Efreet_Trash_Group Efreet_Trash: The XDG Trash Specification
 * @ingroup Efreet_Group
 *
 * Efreet_Trash.h provides all the necessary headers and includes to
 * work with Efreet_Trash.
 *
 * @{
 */

/**
 * @brief Initializes the efreet trash system.
 *
 * @return @c 1 on success, otherwise @c 0 on failure
 */
EAPI int         efreet_trash_init(void);

/**
 * @brief Cleans up the efreet trash system.
 *
 * @return No value
 */
EAPI int         efreet_trash_shutdown(void);

/**
 * @brief Gets the XDG Trash local directory.
 *
 * @remarks The return value must be freed using eina_stringshare_del.
 *
 * @return The XDG Trash local directory, otherwise @c NULL on errors
 */
EAPI const char *efreet_trash_dir_get(const char *for_file);

/**
 * @brief Tries to move the given URI to the trash.
 *
 * @remarks Files on a different filesystem can't be moved to trash. If force_delete
 *          is @c 0 then non-local files are ignored and @c -1 is returned, if you set
 *          @a force_delete to @c 1 non-local files are deleted without asking.
 *
 * @param[in] uri The local URI to move to the trash
 * @param[in] force_delete If @c 1 then files on a different filesystem are deleted permanently
 * 
 * @return @c 1 on success, otherwise @c 0 on failure or @c -1 if the uri is not on
 *         the same filesystem and force_delete is not set
 */
EAPI int         efreet_trash_delete_uri(Efreet_Uri *uri, int force_delete);

/**
 * @brief Lists all the files and directories currently inside the trash.
 *
 * @return A list of strings with a filename (remember to free the list
 *         when you don't need it anymore)
 */
EAPI Eina_List  *efreet_trash_ls(void);

/**
 * @brief Checks whether the trash is currently empty.
 *
 * @return @c 1 if the trash is empty, otherwise @c 0 if some files are present in it
 */
EAPI int         efreet_trash_is_empty(void);

/**
 * @brief Deletes all the files inside the trash.
 *
 * @return @c 1 on success, otherwise @c 0 on failure
 */
EAPI int         efreet_trash_empty_trash(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
