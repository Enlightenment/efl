#ifndef __EVIL_DIRENT_H__
#define __EVIL_DIRENT_H__

#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef EFL_EVIL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVIL_BUILD */
#endif /* _WIN32 */


/**
 * @file dirent.h
 * @brief The file that provides functions ported from Unix in dirent.h.
 * @defgroup Evil_Dirent_Group Dirent.h functions
 *
 * This header provides functions ported from Unix in dirent.h.
 *
 * @{
 */


#ifdef UNICODE
# include <wchar.h>
#endif

/**
 * @def DT_UNKNOWN
 * Specifies that the file type is unknown.
 */
#define DT_UNKNOWN 0

/**
 * @def DT_DIR
 * Specifies that the file type is a directory.
 */
#define DT_DIR     4

/**
 * @typedef DIR
 * @brief A structure that describes a directory stream.
 */
typedef struct dirent DIR;

/**
 * @struct dirent
 * @brief A structure that describes a directory stream.
 */
struct dirent
{
   char          d_name[260 + 1]; /**< The filename. */
   int           d_mode;          /**< The mode */
   unsigned char d_type;          /**< The type */
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @brief Open the given directory.
 *
 * @param name The directory to open.
 * @return A pointer to the directory stream.
 *
 * This function opens the directory @p name and return the directory
 * stream. On error or if @p dir is NULL, -1 is returned, and errno is
 * set appropriately (on Windows XP only). On success, 0 is returned.
 *
 * @see closedir()
 * @see readdir()
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI DIR *opendir(char const *name);

/**
 * @brief Close the given directory.
 *
 * @param dir The directory stream to close.
 * @return A pointer to the directory stream.
 *
 * This function closes the stream directory @p dir. On error or is
 * @p path is NULL or an empty string, NULL is returned, and errno is set
 * appropriately (on Windows XP only).
 *
 * @see opendir()
 * @see readdir()
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI int closedir(DIR *dir);

/**
 * @brief Read the given directory.
 *
 * @param dir The directory stream to read.
 * @return A pointer to a dirent structure, @c NULL oterhwise.
 *
 * This function returns a pointer to a dirent structure representing
 * the next directory entry in the directory stream pointed to by
 * @p dir. It returns NULL on reaching the end of the directory stream
 * or if an error occurred and errno is set appropriately (on Windows XP only).
 *
 * @see opendir()
 * @see readdir()
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI struct dirent *readdir(DIR *dir);


#ifdef __cplusplus
}
#endif /* __cplusplus */



/**
 * @}
 */


#endif /* __EVIL_DIRENT_H__ */
