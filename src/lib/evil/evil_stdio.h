#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


/**
 * @file evil_stdio.h
 * @brief The file that provides functions ported from Unix in stdio.h.
 * @defgroup Evil_Stdio_Group Stdio.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in stdio.h.
 *
 * 
 */

#define EVIL_PATH_SEP_SWAP(p, s1, s2) \
   do { \
     char *_evil_path_tmp; \
     _evil_path_tmp = p; \
     while (*_evil_path_tmp) \
       { \
          if (*_evil_path_tmp == s1) \
            *_evil_path_tmp = s2; \
          _evil_path_tmp++; \
       } \
   } while (0)

#define EVIL_PATH_SEP_WIN32_TO_UNIX(p) EVIL_PATH_SEP_SWAP(p, '\\', '/')
#define EVIL_PATH_SEP_UNIX_TO_WIN32(p) EVIL_PATH_SEP_SWAP(p, '/', '\\')


/**
 * @brief Emulate the rename() function on Windows.
 *
 * @param src The old pathname.
 * @param dst The new pathname.
 * @return 0 on success, -1 otherwise.
 *
 * This function emulates the POSIX rename() function on Windows.
 * The difference with the POSIX function is that the rename() function
 * on windows fails if the destination exists.
 *
 * @since 1.8
 */
EAPI int evil_rename(const char *src, const char *dst);

/**
 * @def rename(src, dest)
 *
 * Wrapper around evil_rename().
 */
# define rename(src, dst) evil_rename(src, dst)


#endif /* __EVIL_STDIO_H__ */
