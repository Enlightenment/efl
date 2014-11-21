#ifndef EFREET_MIME_H
#define EFREET_MIME_H

/**
 * @internal
 * @file Efreet_Mime.h
 *
 * @brief The file that must be included by any project wishing to use Efreet_Mime.
 *
 * @internal
 * @defgroup Efreet_Mime_Group Efreet_Mime: The XDG Shared Mime Info standard
 * @ingroup Efreet_Group
 *
 * Efreet Mime is a library designed to help applications work with the
 * Freedesktop.org Shared Mime Info standard.
 * Efreet_Mime.h provides all the necessary headers and
 * includes to work with Efreet_Mime.
 *
 * @{
 */

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFREET_MIME_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFREET_MIME_BUILD */
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
 * @brief Initializes the efreet mime settings.
 *
 * @return @c 1 on success, otherwise @c 0 on failure
 */
EAPI int         efreet_mime_init(void);

/**
 * @brief Shuts down the Efreet mime settings system if a balanced number of
 *        init/shutdown calls have been made.
 *
 * @return The number of times the init function has been called minus the
 *         corresponding init call
 */
EAPI int         efreet_mime_shutdown(void);

/**
 * @brief Gets the mime type of a file.
 *
 * @param[in] file The file to find the mime type of
 * @return The mime type as a string
 */
EAPI const char *efreet_mime_type_get(const char *file);

/**
 * @brief Gets the mime type of a file using magic.
 *
 * @param[in] file The file to check the mime type of
 * @return The mime type as a string
 */
EAPI const char *efreet_mime_magic_type_get(const char *file);

/**
 * @brief Gets the mime type of a file using globs.
 *
 * @param[in] file The file to check the mime type of
 * @return The mime type as a string
 */
EAPI const char *efreet_mime_globs_type_get(const char *file);

/**
 * @brief Gets the special mime type of a file.
 *
 * @param[in] file The file to check the mime type of
 * @return The mime type as a string
 */
EAPI const char *efreet_mime_special_type_get(const char *file);

/**
 * @brief Gets the fallback mime type of a file.
 *
 * @param[in] file The file to check the mime type of
 * @return The mime type as a string
 */
EAPI const char *efreet_mime_fallback_type_get(const char *file);


/**
 * @brief Gets the mime type icon for a file.
 *
 * @param[in] mime The name of the mime type
 * @param[in] theme The name of the theme to search icons in
 * @param[in] size The required size of the icon
 * @return The mime type icon path as a string
 */
EAPI const char *efreet_mime_type_icon_get(const char *mime, const char *theme,
                                           unsigned int size);

/**
 * @brief Clears the mime icons mapping cache.
 */
EAPI void efreet_mime_type_cache_clear(void);

/**
 * @brief Flushes the mime icons mapping cache.
 *
 * @remarks Flush timeout is defined at compile time by
 *          EFREET_MIME_ICONS_FLUSH_TIMEOUT.
 */
EAPI void efreet_mime_type_cache_flush(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
