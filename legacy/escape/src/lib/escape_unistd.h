#ifndef __ESCAPE_UNISTD_H__
#define __ESCAPE_UNISTD_H__

/**
 * @file escape_unistd.h
 * @brief The file that provides functions ported from Unix in unistd.h.
 * @defgroup Escape_Unistd_Group Unistd.h functions
 *
 * This header provides functions ported from Unix in unistd.h.
 *
 * @{
 */

#include <sys/syslimits.h>

/* Path function */
/**
 * @brief return the canonicalized absolute pathname
 *
 * @param path A path to canonicalize
 * @param resolved_path A pointer of size PATH_MAX where to store the result
 *
 * realpath()  expands all symbolic links and resolves references to /./, /../
 * and extra '/' characters in the null-terminated string named by path to
 * produce a canonicalized absolute pathname.  The resulting pathname is
 * stored as a null-terminated string, up to a maximum of PATH_MAX bytes,
 * in the buffer pointed to by resolved_path.  The resulting path will have
 * no symbolic link, /./ or /../ components.
 *
 * If resolved_path is specified as NULL, then realpath() uses malloc(3)
 * to allocate a buffer of up to PATH_MAX bytes to hold the resolved pathname,
 * and returns a pointer to this buffer.
 * The caller should deallocate this buffer using free(3).
 *
 */
EAPI char *escape_realpath(const char *path, char *resolved_path);
#ifdef realpath
#undef realpath
#endif
#define realpath escape_realpath

EAPI ssize_t
escape_readlink(const char *path,
                char *buf,
                size_t bufsize);
#ifdef readlink
#undef readlink
#endif
#define readlink escape_readlink

EAPI int
escape_symlink(const char *path1, const char *path2);
#ifdef symlink
#undef symlink
#endif
#define symlink escape_symlink

/**
 * @brief check real user's permissions for a file
 *
 * @param pathname The path to check
 * @param mode the permission to check
 *
 * access()  checks  whether the calling process can access the file pathname.
 * If pathname is a symbolic link, it is dereferenced.
 * The mode specifies the accessibility check(s) to be performed, and is either
 * the value F_OK, or a mask  consisting  of  the bitwise  OR  of  one or more
 * of R_OK, W_OK, and X_OK.  F_OK tests for the existence of the file.
 * R_OK, W_OK, and X_OK test whether the file exists and grants read, write,
 * and execute permissions, respectively.
 *
 */
EAPI int
escape_access(const char *pathname, int mode);
#ifdef access
#undef access
#endif
#define access escape_access

/**
 * @brief Create a pair of sockets.
 *
 * @param fds A pointer that contains two sockets.
 *
 * Create a pair of sockets that can be use with select().
 * Contrary to Unix, that functions does not
 * create a pair of file descriptors.
 *
 * Conformity: Not applicable.
 */
EAPI int
escape_pipe(int *fds);

/**
 * @def pipe(fds)
 *
 * Wrapper around escape_pipe().
 */
#define pipe(fds) escape_pipe(fds)

//#define sync()

/**
 * @}
 */

#endif /* __ESCAPE_UNISTD_H__ */
